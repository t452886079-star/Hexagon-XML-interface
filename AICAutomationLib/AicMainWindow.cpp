#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#include "AicMainWindow.h"
#include "AicConnectionManager.h"
#include "AicXmlHelper.h"
#include "AicLanguageManager.h"
#include "AicAutomationSingleton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QMessageBox>
#include <QDateTime>
#include <QHeaderView>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>

// ─── 辅助函数 ───────────────────────────────────────────────────────────────

static QPushButton *makeBtn(const QString &text, const QString &objName)
{
    QPushButton *b = new QPushButton(text);
    b->setObjectName(objName);
    b->setMinimumHeight(32);
    return b;
}

// ─── 构造 / 析构 ────────────────────────────────────────────────────────────

/**
 * @brief 构造函数
 *        【修改】不再 new AicConnectionManager，统一使用 Singleton 的连接
 */
AicMainWindow::AicMainWindow(QWidget *parent)
    : QWidget(parent)
    , m_conn(AicAutomationSingleton::instance()->connection())
{
    setupUi();
    loadConfig();

    // 绑定 Singleton 连接管理器的信号到本窗口的槽
    connect(m_conn, &AicConnectionManager::connectionStateChanged,
            this, &AicMainWindow::onConnectionStateChanged);
    connect(m_conn, &AicConnectionManager::responseReceived,
            this, &AicMainWindow::onResponseReceived);
    connect(m_conn, &AicConnectionManager::logMessage,
            this, &AicMainWindow::onLogMessage);
    connect(m_conn, &AicConnectionManager::heartbeatFailed,
            this, &AicMainWindow::onHeartbeatFailed);
    connect(m_conn, &AicConnectionManager::connectionLost,
            this, &AicMainWindow::onConnectionLost);

    // 绑定请求信号到对应的命令槽（信号驱动，异步执行）
    connect(this, &AicMainWindow::sigRequestPartList,
            this, &AicMainWindow::onBtnPartList);
    connect(this, &AicMainWindow::sigRequestInspectionList,
            this, &AicMainWindow::onBtnInspectionList);
    connect(this, &AicMainWindow::sigRequestInspect,
            this, &AicMainWindow::onBtnInspect);
    connect(this, &AicMainWindow::sigRequestInspectContinue,
            this, &AicMainWindow::onBtnInspectContinue);
    connect(this, &AicMainWindow::sigRequestCorrect,
            this, &AicMainWindow::onBtnCorrect);
    connect(this, &AicMainWindow::sigRequestCalibrate,
            this, &AicMainWindow::onBtnCalibrate);
    connect(this, &AicMainWindow::sigRequestReport,
            this, &AicMainWindow::onBtnReport);
    connect(this, &AicMainWindow::sigRequestExportRBS,
            this, &AicMainWindow::onBtnExportRBS);
}

AicMainWindow::~AicMainWindow() {}

/**
 * @brief 获取最近一次 Correct 命令返回的修正数据
 */
QVector<CorrectionRow> AicMainWindow::correctionRows() const
{
    return m_correctionRows;
}

// ─── 【删除】connectionManager() 方法不再需要
//     外部直接用 AicAutomationSingleton::instance()->connection() ──────────

// ─── UI 初始化 ──────────────────────────────────────────────────────────────

void AicMainWindow::setupUi()
{
    setWindowTitle(tr("AIC Automation"));
    setMinimumSize(1000, 700);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // ── 顶部：连接栏 ─────────────────────────────────────────────────────
    QGroupBox *grpConn = new QGroupBox(tr("Connection"));
    grpConn->setObjectName("grpConn");
    QHBoxLayout *hConn = new QHBoxLayout(grpConn);
    QLabel *lblHost = new QLabel(tr("Host:"));
    lblHost->setObjectName("lblHost");
    hConn->addWidget(lblHost);
    m_editHost = new QLineEdit("localhost");
    m_editHost->setObjectName("editHost");
    m_editHost->setMaximumWidth(200);
    hConn->addWidget(m_editHost);
    QLabel *lblPort = new QLabel(tr("Port:"));
    lblPort->setObjectName("lblPort");
    hConn->addWidget(lblPort);
    m_editPort = new QLineEdit("32459");
    m_editPort->setObjectName("editPort");
    m_editPort->setMaximumWidth(80);
    m_editPort->setValidator(new QIntValidator(1, 65535, this));
    hConn->addWidget(m_editPort);
    m_btnConnect = new QPushButton(tr("Connect"));
    m_btnConnect->setObjectName("btnConnect");
    m_btnConnect->setMinimumHeight(32);
    hConn->addWidget(m_btnConnect);
    hConn->addSpacing(20);
    m_lblState = new QLabel(tr("State: Disconnect"));
    m_lblState->setObjectName("lblState");
    m_lblState->setStyleSheet("font-weight:bold; color:#c0392b;");
    hConn->addWidget(m_lblState);
    hConn->addSpacing(10);
    m_lblHeartbeat = new QLabel;
    m_lblHeartbeat->setObjectName("lblHeartbeat");
    m_lblHeartbeat->setFixedSize(24, 24);
    m_lblHeartbeat->setToolTip(tr("Heartbeat: inactive"));
    updateHeartbeatIndicator("red");
    hConn->addWidget(m_lblHeartbeat);
    hConn->addStretch();
    mainLayout->addWidget(grpConn);

    connect(m_btnConnect, &QPushButton::clicked, this, &AicMainWindow::onConnectClicked);

    // ── 中部：左侧（数据+按钮） | 右侧（检测项列表） ─────────────────────
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setObjectName("splitterMain");

    QWidget *leftPanel = new QWidget;
    leftPanel->setObjectName("leftPanel");
    QVBoxLayout *vLeft = new QVBoxLayout(leftPanel);
    vLeft->setContentsMargins(0, 0, 0, 0);
    vLeft->setSpacing(6);

    QGroupBox *grpData = new QGroupBox(tr("New Data"));
    grpData->setObjectName("grpData");
    QGridLayout *gridData = new QGridLayout(grpData);
    QLabel *lblPart = new QLabel(tr("Part:"));
    lblPart->setObjectName("lblPart");
    gridData->addWidget(lblPart, 0, 0);
    m_editPart = new QLineEdit;
    m_editPart->setObjectName("editPart");
    gridData->addWidget(m_editPart, 0, 1);
    QLabel *lblInspection = new QLabel(tr("Inspection:"));
    lblInspection->setObjectName("lblInspection");
    gridData->addWidget(lblInspection, 1, 0);
    m_editInspection = new QLineEdit;
    m_editInspection->setObjectName("editInspection");
    gridData->addWidget(m_editInspection, 1, 1);
    QLabel *lblResult = new QLabel(tr("Result:"));
    lblResult->setObjectName("lblResult");
    gridData->addWidget(lblResult, 2, 0);
    m_editResult = new QLineEdit;
    m_editResult->setObjectName("editResult");
    gridData->addWidget(m_editResult, 2, 1);
    vLeft->addWidget(grpData);

    QGroupBox *grpBtns = new QGroupBox(tr("Command"));
    grpBtns->setObjectName("grpBtns");
    QGridLayout *gridBtns = new QGridLayout(grpBtns);
    gridBtns->setSpacing(6);

    m_btnPartList        = makeBtn(tr("Query PartList"),        "btnPartList");
    m_btnInspectionList  = makeBtn(tr("Query InspectionList"),  "btnInspectionList");
    m_btnCalibrate       = makeBtn(tr("Command Calibrate"),       "btnCalibrate");
    m_btnInspect         = makeBtn(tr("Command Inspect"),         "btnInspect");
    m_btnInspectContinue = makeBtn(tr("Command Inspect-Continue"), "btnInspectContinue");
    m_btnCorrect         = makeBtn(tr("Command Correct"),         "btnCorrect");
    m_btnReport          = makeBtn(tr("Command Report"),          "btnReport");
    m_btnExportRBS       = makeBtn(tr("Command ExportRBS"),       "btnExportRBS");

    gridBtns->addWidget(m_btnPartList,        0, 0);
    gridBtns->addWidget(m_btnInspectionList,  0, 1);
    gridBtns->addWidget(m_btnCalibrate,       1, 0);
    gridBtns->addWidget(m_btnInspect,         1, 1);
    gridBtns->addWidget(m_btnInspectContinue, 2, 0);
    gridBtns->addWidget(m_btnCorrect,         2, 1);
    gridBtns->addWidget(m_btnReport,          3, 0);
    gridBtns->addWidget(m_btnExportRBS,       3, 1);
    vLeft->addWidget(grpBtns);

    connect(m_btnPartList,        &QPushButton::clicked, this, &AicMainWindow::onBtnPartList);
    connect(m_btnInspectionList,  &QPushButton::clicked, this, &AicMainWindow::onBtnInspectionList);
    connect(m_btnInspect,         &QPushButton::clicked, this, &AicMainWindow::onBtnInspect);
    connect(m_btnInspectContinue, &QPushButton::clicked, this, &AicMainWindow::onBtnInspectContinue);
    connect(m_btnCorrect,         &QPushButton::clicked, this, &AicMainWindow::onBtnCorrect);
    connect(m_btnCalibrate,       &QPushButton::clicked, this, &AicMainWindow::onBtnCalibrate);
    connect(m_btnReport,          &QPushButton::clicked, this, &AicMainWindow::onBtnReport);
    connect(m_btnExportRBS,       &QPushButton::clicked, this, &AicMainWindow::onBtnExportRBS);

    splitter->addWidget(leftPanel);

    QGroupBox *grpTree = new QGroupBox(tr("Inspection List"));
    grpTree->setObjectName("grpTree");
    QVBoxLayout *vTree = new QVBoxLayout(grpTree);
    m_treeInspection = new QTreeWidget;
    m_treeInspection->setObjectName("treeInspection");
    m_treeInspection->setHeaderLabels({tr("Name"), tr("GUID"), tr("Comment")});
    m_treeInspection->header()->setSectionResizeMode(QHeaderView::Stretch);
    m_treeInspection->setRootIsDecorated(false);
    m_treeInspection->setAlternatingRowColors(true);
    vTree->addWidget(m_treeInspection);
    splitter->addWidget(grpTree);

    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    mainLayout->addWidget(splitter, 1);

    // ── 底部：调试选项卡 ─────────────────────────────────────────────────
    m_tabDebug = new QTabWidget;
    m_tabDebug->setObjectName("tabDebug");

    QWidget *reqPage = new QWidget;
    reqPage->setObjectName("reqPage");
    QVBoxLayout *vReq = new QVBoxLayout(reqPage);
    vReq->setContentsMargins(4, 4, 4, 4);
    m_txtRequest = new QTextEdit;
    m_txtRequest->setObjectName("txtRequest");
    m_txtRequest->setReadOnly(true);
    m_txtRequest->setStyleSheet("font-family: Consolas, 'Courier New', monospace; font-size: 12px;");
    m_btnClearRequest = makeBtn(tr("Clear"), "btnClearRequest");
    connect(m_btnClearRequest, &QPushButton::clicked, this, &AicMainWindow::onClearRequest);
    vReq->addWidget(m_txtRequest);
    vReq->addWidget(m_btnClearRequest);
    m_tabDebug->addTab(reqPage, tr("Request"));

    QWidget *respPage = new QWidget;
    respPage->setObjectName("respPage");
    QVBoxLayout *vResp = new QVBoxLayout(respPage);
    vResp->setContentsMargins(4, 4, 4, 4);
    m_txtResponse = new QTextEdit;
    m_txtResponse->setObjectName("txtResponse");
    m_txtResponse->setReadOnly(true);
    m_txtResponse->setStyleSheet("font-family: Consolas, 'Courier New', monospace; font-size: 12px;");
    m_btnClearResponse = makeBtn(tr("Clear"), "btnClearResponse");
    connect(m_btnClearResponse, &QPushButton::clicked, this, &AicMainWindow::onClearResponse);
    vResp->addWidget(m_txtResponse);
    vResp->addWidget(m_btnClearResponse);
    m_tabDebug->addTab(respPage, tr("Response"));

    QWidget *logPage = new QWidget;
    logPage->setObjectName("logPage");
    QVBoxLayout *vLog = new QVBoxLayout(logPage);
    vLog->setContentsMargins(4, 4, 4, 4);
    m_txtLog = new QTextEdit;
    m_txtLog->setObjectName("txtLog");
    m_txtLog->setReadOnly(true);
    m_txtLog->setStyleSheet("font-family: Consolas, 'Courier New', monospace; font-size: 12px;");
    m_btnClearLog = makeBtn(tr("Clear"), "btnClearLog");
    connect(m_btnClearLog, &QPushButton::clicked, this, &AicMainWindow::onClearLog);
    vLog->addWidget(m_txtLog);
    vLog->addWidget(m_btnClearLog);
    m_tabDebug->addTab(logPage, tr("Log"));

    mainLayout->addWidget(m_tabDebug, 1);

    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 1px solid #aaa;
            border-radius: 4px;
            margin-top: 8px;
            padding-top: 14px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QPushButton {
            border: 1px solid #888;
            border-radius: 3px;
            padding: 4px 12px;
            background: #f0f0f0;
        }
        QPushButton:hover { background: #e0e0e0; }
        QPushButton:pressed { background: #ccc; }
        QLineEdit {
            padding: 4px;
            border: 1px solid #aaa;
            border-radius: 3px;
        }
        QTreeWidget {
            border: 1px solid #aaa;
            border-radius: 3px;
        }
    )");
}

// ─── 连接管理 ───────────────────────────────────────────────────────────────

void AicMainWindow::onConnectClicked()
{
    if (m_conn->isConnected()) {
        m_conn->disconnect();
    } else {
        QString host = m_editHost->text().trimmed();
        int port = m_editPort->text().toInt();
        if (host.isEmpty() || port <= 0) {
            QMessageBox::warning(this, tr("Error"), tr("Please enter valid host and port."));
            return;
        }
        m_conn->connectToHost(host, port);
        saveConfig();
    }
}

void AicMainWindow::onConnectionStateChanged(bool connected)
{
    if (connected) {
        m_lblState->setText(tr("State: Connected"));
        m_lblState->setStyleSheet("font-weight:bold; color:#27ae60;");
        m_btnConnect->setText(tr("Disconnect"));
        updateHeartbeatIndicator("green");
    } else {
        m_lblState->setText(tr("State: Disconnect"));
        m_lblState->setStyleSheet("font-weight:bold; color:#c0392b;");
        m_btnConnect->setText(tr("Connect"));
        updateHeartbeatIndicator("red");
    }
}

// ─── 响应处理 ───────────────────────────────────────────────────────────────

void AicMainWindow::onResponseReceived(const QString &tag, const QString &body, int httpCode)
{
    if (body.isEmpty()) {
        onLogMessage(tr("[WARN] %1: empty response (HTTP %2)").arg(tag).arg(httpCode));
        emit commandCompleted(tag, false, tr("HTTP error, empty response (code: %1)").arg(httpCode));
        return;
    }

    m_txtResponse->append(QString("── %1 [%2] ──").arg(tag, QDateTime::currentDateTime().toString("hh:mm:ss")));
    m_txtResponse->append(body);
    m_txtResponse->append("");

    autoExtractFromResponse(body);
}

void AicMainWindow::autoExtractFromResponse(const QString &body)
{
    QString reqId = AicXmlHelper::getRequestID(body);
    bool ok = AicXmlHelper::cmdSuccessful(body);
    QString errMsg = ok ? QString() : AicXmlHelper::getRetMsg(body);
    if (errMsg.isEmpty() && !ok)
	{
        errMsg = tr("Command failed (ErrorCode: %1)").arg(AicXmlHelper::getRetCode(body));
	}

	errMsg = QString::number(AicXmlHelper::getRetCode(body));
	

    if (reqId == "PartList") {
        auto parts = AicXmlHelper::getPartList(body);
        if (ok && !parts.isEmpty()) {
            //if (m_editPart->text().trimmed().isEmpty())
                m_editPart->setText(parts.first().second);
        }
        emit partListReceived(ok, parts, errMsg);
    }
    else if (reqId == "InspectList" || reqId == "InspectionList") {
        auto inspections = AicXmlHelper::getInspectionList(body);
        if (ok) {
            m_treeInspection->clear();
            for (const auto &item : inspections) {
                QTreeWidgetItem *twItem = new QTreeWidgetItem(m_treeInspection);
                twItem->setText(0, item.first);
                twItem->setText(1, item.second);
            }
            m_treeInspection->expandAll();
            //if (!inspections.isEmpty() && m_editInspection->text().trimmed().isEmpty())
                m_editInspection->setText(inspections.first().second);
        }
        emit inspectionListReceived(ok, inspections, errMsg);
    }
    else if (reqId == "Inspect") {
        QString guid = ok ? AicXmlHelper::getResultGUID(body) : QString();
        if (!guid.isEmpty())
            m_editResult->setText(guid);

        // 解析完整测量结果（手册 2.2.3.3.1）
        MeasurementResult mr = AicXmlHelper::parseMeasurementResult(body);

        // 打印管件合格/不合格到日志
        if (ok && mr.resultValue >= 0.0) {
            QString completed = mr.isCompleted
                ? tr("Yes (full tube)")
                : tr("No (partial section)");
            QString level = AicXmlHelper::toleranceLevelString(mr.toleranceLevel);
            // isCompleted == false 表示非整管测量，直接不合格
            QString verdict;
            if (!mr.isCompleted) {
                verdict = tr("NOT OK (incomplete measurement)");
            } else if (mr.toleranceLevel == ToleranceLevel::WithinTolerance) {
                verdict = tr("OK");
            } else if (mr.toleranceLevel == ToleranceLevel::WithinIntervention) {
                verdict = tr("OK (with deviation)");
            } else if (mr.toleranceLevel == ToleranceLevel::AboveConcession) {
                verdict = tr("NOT OK (above concession)");
            } else {
                verdict = tr("NOT OK (out of tolerance)");
            }

            onLogMessage(tr("[Inspect] ResultValue=%1 MaxTolerance=%2% IsCompleted=%3 Level=%4 Verdict=%5")
                            .arg(mr.resultValue, 0, 'f', 2)
                            .arg(mr.maxTolerance, 0, 'f', 2)
                            .arg(completed, level, verdict));

            // 打印分量数据
            for (const auto &comp : mr.components) {
                onLogMessage(tr("  Component: %1 Length=%2mm Dev=%3mm EndToEnd=%4mm Dev=%5mm")
                                .arg(comp.compName)
                                .arg(comp.length, 0, 'f', 2)
                                .arg(comp.lengthDev, 0, 'f', 2)
                                .arg(comp.endToEnd, 0, 'f', 2)
                                .arg(comp.endToEndDev, 0, 'f', 2));
            }
            // 打印护套判定数据
            for (const auto &sc : mr.sheathCriteria) {
                onLogMessage(tr("  SheathCriteria: %1 Comp=%2 Start=%3(%4) End=%5(%6)")
                                .arg(sc.critName, sc.compName)
                                .arg(sc.startType).arg(sc.startValue, 0, 'f', 2)
                                .arg(sc.endType).arg(sc.endValue, 0, 'f', 2));
            }
        }

        // 非整管测量 → 强制判定为不合格，确保下游信号拿到正确的 toleranceLevel
        if (!mr.isCompleted && mr.resultValue >= 0.0) {
            mr.toleranceLevel = ToleranceLevel::OutOfTolerance;
        }

        // 对外输出：旧接口（兼容）+ 新接口（完整结果）
        emit inspectResult(ok, guid, errMsg);
        emit inspectMeasurementResult(ok, mr, errMsg);
    }
    else if (reqId == "InspectContinue") {
        // InspectContinue 同样返回测量结果，解析并输出
        MeasurementResult mr = AicXmlHelper::parseMeasurementResult(body);
        if (ok && mr.resultValue >= 0.0) {
            QString level = AicXmlHelper::toleranceLevelString(mr.toleranceLevel);
            onLogMessage(tr("[InspectContinue] ResultValue=%1 Level=%2")
                            .arg(mr.resultValue, 0, 'f', 2).arg(level));
        }
        // 非整管测量 → 强制判定为不合格
        if (!mr.isCompleted && mr.resultValue >= 0.0) {
            mr.toleranceLevel = ToleranceLevel::OutOfTolerance;
        }
        emit inspectContinueResult(ok, errMsg);
        emit inspectMeasurementResult(ok, mr, errMsg);
    }
    else if (reqId == "Correct") {
        QVector<CorrectionRow> rows;
        if (ok) {
            rows = AicXmlHelper::parseCorrectResponse(body);

            // 打印修正数据到日志
            onLogMessage(tr("[Correct] Received %1 correction rows").arg(rows.size()));
            for (const auto &row : rows) {
                onLogMessage(tr("  BE=%1 Push=%2(%3) Bend=%4(%5) Rotate=%6(%7) Radius=%8(%9)")
                                .arg(row.be)
                                .arg(row.push, 0, 'f', 2).arg(row.pushDev, 0, 'f', 2)
                                .arg(row.bend, 0, 'f', 2).arg(row.bendDev, 0, 'f', 2)
                                .arg(row.rotate, 0, 'f', 2).arg(row.rotateDev, 0, 'f', 2)
                                .arg(row.radius, 0, 'f', 2).arg(row.radiusDev, 0, 'f', 2));
            }
        }
        // 缓存修正数据，供外部通过 correctionRows() 访问
        m_correctionRows = rows;
        emit correctResult(ok, rows, errMsg);
    }
    else if (reqId == "Calibrate") {
        emit calibrateResult(ok, errMsg);
    }
    else if (reqId == "Report") {
        emit reportResult(ok, errMsg);
    }
    else if (reqId == "ExportRBS") {
        emit exportRBSResult(ok, errMsg);
    }

    // 统一发送命令完成信号
    emit commandCompleted(reqId, ok, errMsg);
}

void AicMainWindow::onLogMessage(const QString &msg)
{
    m_txtLog->append(QString("[%1] %2")
                     .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                     .arg(msg));
}

// ─── 命令按钮槽函数 ────────────────────────────────────────────────────────

void AicMainWindow::sendCommand(const QString &xml, const QString &tag)
{
    m_txtRequest->append(QString("── %1 [%2] ──").arg(tag, QDateTime::currentDateTime().toString("hh:mm:ss")));
    m_txtRequest->append(xml);
    m_txtRequest->append("");
    m_conn->sendRequest(xml, tag);
}

void AicMainWindow::onBtnPartList()
{
    sendCommand(AicXmlHelper::formatPartsQuery(), "PartList");
}

void AicMainWindow::onBtnInspectionList()
{
    QString part = m_editPart->text().trimmed();
    if (part.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Please enter a Part first."));
        return;
    }
    sendCommand(AicXmlHelper::formatInspectionsQuery(part), "InspectionList");
}

void AicMainWindow::onBtnInspect()
{
    QString part = m_editPart->text().trimmed();
    QString insp = m_editInspection->text().trimmed();
    if (part.isEmpty() || insp.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Please enter Part and Inspection."));
        return;
    }
    sendCommand(AicXmlHelper::formatInspectCmd(part, insp, ""), "Inspect");
}

void AicMainWindow::onBtnInspectContinue()
{
    QString part = m_editPart->text().trimmed();
    QString insp = m_editInspection->text().trimmed();
    QString result = m_editResult->text().trimmed();
    if (part.isEmpty() || insp.isEmpty() || result.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Please enter Part, Inspection and Result."));
        return;
    }
    sendCommand(AicXmlHelper::formatInspectContinueCmd(part, insp, result, ""), "InspectContinue");
}

void AicMainWindow::onBtnCorrect()
{
    QString part = m_editPart->text().trimmed();
    QString result = m_editResult->text().trimmed();
    if (part.isEmpty() || result.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Please enter Part and Result."));
        return;
    }
    sendCommand(AicXmlHelper::formatCorrectCmd(part, result), "Correct");
}

void AicMainWindow::onBtnCalibrate()
{
    sendCommand(AicXmlHelper::formatCalibrateCmd(), "Calibrate");
}

void AicMainWindow::onBtnReport()
{
    QString part = m_editPart->text().trimmed();
    QString result = m_editResult->text().trimmed();
    if (part.isEmpty() || result.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Please enter Part and Result."));
        return;
    }
    sendCommand(AicXmlHelper::formatReportCmd(part, result), "Report");
}

void AicMainWindow::onBtnExportRBS()
{
    QString part = m_editPart->text().trimmed();
    QString result = m_editResult->text().trimmed();
    if (part.isEmpty() || result.isEmpty()) {
        QMessageBox::information(this, tr("Info"), tr("Please enter Part and Result."));
        return;
    }
    sendCommand(AicXmlHelper::formatExportRBSCmd(part, result), "ExportRBS");
}

// ─── 清空按钮 ───────────────────────────────────────────────────────────────

void AicMainWindow::onClearRequest()  { m_txtRequest->clear(); }
void AicMainWindow::onClearResponse() { m_txtResponse->clear(); }
void AicMainWindow::onClearLog()      { m_txtLog->clear(); }

void AicMainWindow::onHeartbeatFailed()
{
    updateHeartbeatIndicator("yellow");
}

void AicMainWindow::onConnectionLost(bool userDisconnected)
{
    if (userDisconnected) {
        updateHeartbeatIndicator("red");
        m_lblHeartbeat->setToolTip(tr("Heartbeat: inactive"));
    } else {
        updateHeartbeatIndicator("yellow");
        m_lblHeartbeat->setToolTip(tr("Heartbeat: lost, reconnecting..."));
    }
}

void AicMainWindow::updateHeartbeatIndicator(const QString &state)
{
    QString color, tooltip;
    if (state == "green") {
        color = "#27ae60";
        tooltip = tr("Heartbeat: OK");
    } else if (state == "yellow") {
        color = "#f39c12";
        tooltip = tr("Heartbeat: warning");
    } else {
        color = "#c0392b";
        tooltip = tr("Heartbeat: inactive");
    }
    m_lblHeartbeat->setStyleSheet(
        QString("background-color: %1; border-radius: 12px; border: 1px solid #888;")
            .arg(color));
    m_lblHeartbeat->setToolTip(tooltip);
}

// ─── 配置持久化 ─────────────────────────────────────────────────────────────

static QString configFilePath()
{
    return QCoreApplication::applicationDirPath() + QDir::separator() + "HostPort.ini";
}

void AicMainWindow::loadConfig()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    m_editHost->setText(settings.value("Connection/Host", "localhost").toString());
    m_editPort->setText(settings.value("Connection/Port", "32459").toString());
}

void AicMainWindow::saveConfig()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    settings.setValue("Connection/Host", m_editHost->text().trimmed());
    settings.setValue("Connection/Port", m_editPort->text().trimmed());
    settings.sync();
}

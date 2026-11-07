#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#include "AicAutomationSingleton.h"
#include "AicConnectionManager.h"
#include "AicXmlHelper.h"

// 静态单例实例初始化
AicAutomationSingleton *AicAutomationSingleton::s_instance = nullptr;

/**
 * @brief 获取单例实例，首次调用时自动创建
 */
AicAutomationSingleton *AicAutomationSingleton::instance()
{
    if (!s_instance)
        s_instance = new AicAutomationSingleton;
    return s_instance;
}

/**
 * @brief 私有构造函数：创建连接管理器，转发连接管理器的信号
 */
AicAutomationSingleton::AicAutomationSingleton(QObject *parent)
    : QObject(parent)
    , m_conn(new AicConnectionManager(this))
{
    connect(m_conn, &AicConnectionManager::connectionStateChanged,
            this, &AicAutomationSingleton::connectionStateChanged);
    connect(m_conn, &AicConnectionManager::responseReceived,
            this, &AicAutomationSingleton::responseReceived);
    connect(m_conn, &AicConnectionManager::logMessage,
            this, &AicAutomationSingleton::logMessage);
    connect(m_conn, &AicConnectionManager::heartbeatFailed,
            this, &AicAutomationSingleton::heartbeatFailed);
    connect(m_conn, &AicConnectionManager::connectionLost,
            this, &AicAutomationSingleton::connectionLost);

    // 自动解析 Correct 响应，发射 correctResult 信号
    connect(m_conn, &AicConnectionManager::responseReceived,
            this, [this](const QString &tag, const QString &body, int httpCode) {
        if (tag == "Correct") {
            if (body.isEmpty()) {
                emit correctResult(false, QVector<CorrectionRow>(),
                                   tr("HTTP error, empty response (code: %1)").arg(httpCode));
                return;
            }
            bool ok = AicXmlHelper::cmdSuccessful(body);
            QString errMsg = ok ? QString() : AicXmlHelper::getRetMsg(body);
            if (errMsg.isEmpty() && !ok)
                errMsg = QString("Command failed (ErrorCode: %1)").arg(AicXmlHelper::getRetCode(body));
            QVector<CorrectionRow> rows = ok ? AicXmlHelper::parseCorrectResponse(body) : QVector<CorrectionRow>();
            emit correctResult(ok, rows, errMsg);
        }
    });
}

/**
 * @brief 析构函数：将静态实例指针置空
 */
AicAutomationSingleton::~AicAutomationSingleton()
{
    s_instance = nullptr;
}

/**
 * @brief 返回底层连接管理器指针
 */
AicConnectionManager *AicAutomationSingleton::connection() const
{
    return m_conn;
}

/**
 * @brief 连接到指定主机
 */
void AicAutomationSingleton::connectToHost(const QString &host, int port)
{
    m_conn->connectToHost(host, port);
}

/**
 * @brief 断开连接
 */
void AicAutomationSingleton::disconnect()
{
    m_conn->disconnect();
}

/**
 * @brief 查询当前连接状态
 */
bool AicAutomationSingleton::isConnected() const
{
    return m_conn->isConnected();
}

/**
 * @brief 查询心跳是否在运行
 */
bool AicAutomationSingleton::isHeartbeatActive() const
{
    return m_conn->isHeartbeatActive();
}

/**
 * @brief 获取最后一次成功心跳的时间
 */
QDateTime AicAutomationSingleton::lastHeartbeatTime() const
{
    return m_conn->lastHeartbeatTime();
}

/**
 * @brief 查询是否为用户主动断开
 */
bool AicAutomationSingleton::isUserDisconnected() const
{
    return m_conn->isUserDisconnected();
}

/**
 * @brief 直接发送原始 XML 请求
 */
void AicAutomationSingleton::sendRequest(const QString &xmlBody, const QString &tag)
{
    m_conn->sendRequest(xmlBody, tag);
}

// ─── 高层业务命令 ───────────────────────────────────────────────────────────

/**
 * @brief 查询零件列表
 */
void AicAutomationSingleton::queryPartList()
{
    m_conn->sendRequest(AicXmlHelper::formatPartsQuery(), "PartList");
}

/**
 * @brief 查询指定零件的检测项列表
 */
void AicAutomationSingleton::queryInspectionList(const QString &part)
{
    m_conn->sendRequest(AicXmlHelper::formatInspectionsQuery(part), "InspectionList");
}

/**
 * @brief 发送检测命令
 */
void AicAutomationSingleton::commandInspect(const QString &part, const QString &inspection, const QString &comment)
{
    m_conn->sendRequest(AicXmlHelper::formatInspectCmd(part, inspection, comment), "Inspect");
}

/**
 * @brief 发送带结果的检测命令
 */
void AicAutomationSingleton::commandInspectContinue(const QString &part, const QString &inspection,
                                                     const QString &result, const QString &comment)
{
    m_conn->sendRequest(AicXmlHelper::formatInspectContinueCmd(part, inspection, result, comment), "InspectContinue");
}

/**
 * @brief 发送修正命令
 */
void AicAutomationSingleton::commandCorrect(const QString &part, const QString &result)
{
    m_conn->sendRequest(AicXmlHelper::formatCorrectCmd(part, result), "Correct");
}

/**
 * @brief 发送校准命令
 */
void AicAutomationSingleton::commandCalibrate()
{
    m_conn->sendRequest(AicXmlHelper::formatCalibrateCmd(), "Calibrate");
}

/**
 * @brief 发送报告命令
 */
void AicAutomationSingleton::commandReport(const QString &part, const QString &result, const QString &exportPath)
{
    m_conn->sendRequest(AicXmlHelper::formatReportCmd(part, result, exportPath), "Report");
}

/**
 * @brief 发送导出 RBS 命令
 */
void AicAutomationSingleton::commandExportRBS(const QString &part, const QString &result)
{
    m_conn->sendRequest(AicXmlHelper::formatExportRBSCmd(part, result), "ExportRBS");
}

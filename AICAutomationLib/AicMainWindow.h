#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#ifndef AICMAINWINDOW_H
#define AICMAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTabWidget>
#include <QVector>
#include <QPair>
#include <QList>
#include "AICAutomationLib_global.h"
#include "AicXmlHelper.h"

class AicConnectionManager;

/**
 * @brief AIC 自动化主窗口
 *        提供连接管理、零件/检测项查询、检测命令发送、
 *        响应解析和调试日志等完整的 GUI 界面。
 *
 *        内部统一使用 AicAutomationSingleton 的连接管理器，
 *        外部通过 Singleton 或本窗口操作的是同一个连接。
 */
class AICAUTOMATIONLIBSHARED_EXPORT AicMainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AicMainWindow(QWidget *parent = nullptr);
    ~AicMainWindow();

    /**
     * @brief 获取最近一次 Correct 命令返回的修正数据
     *        在 correctResult 信号之后调用可获得最新数据
     * @return 修正数据行列表，未执行过 Correct 则为空列表
     */
    QVector<CorrectionRow> correctionRows() const;

signals:
    // ─── 命令请求信号（外部直接 emit 触发） ──────────────────────────────

    /** @brief 零件列表查询请求 */
    void sigRequestPartList();
    /** @brief 检测项列表查询请求 */
    void sigRequestInspectionList();
    /** @brief 检测命令请求 */
    void sigRequestInspect();
    /** @brief 带结果的检测命令请求 */
    void sigRequestInspectContinue();
    /** @brief 修正命令请求 */
    void sigRequestCorrect();
    /** @brief 校准命令请求 */
    void sigRequestCalibrate();
    /** @brief 报告命令请求 */
    void sigRequestReport();
    /** @brief 导出 RBS 命令请求 */
    void sigRequestExportRBS();

    // ─── 命令结果信号 ─────────────────────────────────────────────────────

    void partListReceived(bool success, const QList<QPair<QString, QString>> &parts, const QString &errorMsg);
    void inspectionListReceived(bool success, const QList<QPair<QString, QString>> &inspections, const QString &errorMsg);

    /**
     * @brief 检测结果信号（旧版，仅返回 GUID，保留兼容）
     * @param success   是否成功
     * @param resultGUID 结果 GUID
     * @param errorMsg  错误信息
     */
    void inspectResult(bool success, const QString &resultGUID, const QString &errorMsg);

    /**
     * @brief 检测结果信号（新版，包含完整测量结果 + 公差水平）
     *        对应手册 2.2.3.3.1 Measurement Result
     *
     *        外部连接此信号可获取：
     *        - result.resultValue   测量值 [0, 4]
     *        - result.maxTolerance  最大公差利用率 [%]
     *        - result.isCompleted   是否整管完成测量（表22 vs 表23）
     *        - result.toleranceLevel 公差水平分类
     *        - result.components    分量数据
     *        - result.sheathCriteria 护套判定数据
     *
     * @param success   是否成功
     * @param result    完整测量结果结构体
     * @param errorMsg  错误信息
     */
    void inspectMeasurementResult(bool success, const MeasurementResult &result, const QString &errorMsg);

    void inspectContinueResult(bool success, const QString &errorMsg);

    /**
     * @brief 修正结果信号（包含修正表数据）
     *        对应手册 2.2.3.3.2 / 2.2.3.4.2 Correction Result
     *
     *        外部连接此信号可获取每个弯曲点的修正值：
     *        - BE, Push, PushDev, Bend, BendDev, Rotate, RotateDev, Radius, RadiusDev
     *
     * @param success   是否成功
     * @param rows      修正数据行列表
     * @param errorMsg  错误信息
     */
    void correctResult(bool success, const QVector<CorrectionRow> &rows, const QString &errorMsg);

    void calibrateResult(bool success, const QString &errorMsg);
    void reportResult(bool success, const QString &errorMsg);
    void exportRBSResult(bool success, const QString &errorMsg);
    void commandCompleted(const QString &tag, bool success, const QString &errorMsg);

private slots:
    void onConnectClicked();
    void onConnectionStateChanged(bool connected);
    void onResponseReceived(const QString &tag, const QString &body, int httpCode);
    void onLogMessage(const QString &msg);

    void onBtnPartList();
    void onBtnInspectionList();
    void onBtnInspect();
    void onBtnInspectContinue();
    void onBtnCorrect();
    void onBtnCalibrate();
    void onBtnReport();
    void onBtnExportRBS();

    void onClearRequest();
    void onClearResponse();
    void onClearLog();

    void onHeartbeatFailed();
    void onConnectionLost(bool userDisconnected);

private:
    void setupUi();
    void sendCommand(const QString &xml, const QString &tag);
    void autoExtractFromResponse(const QString &body);
    void loadConfig();
    void saveConfig();
    void updateHeartbeatIndicator(const QString &state);

    // ─── 连接区域控件 ─────────────────────────────────────────────────────
    QLineEdit   *m_editHost;
    QLineEdit   *m_editPort;
    QPushButton *m_btnConnect;
    QLabel      *m_lblState;
    QLabel      *m_lblHeartbeat;

    // ─── 数据输入区域控件 ─────────────────────────────────────────────────
    QLineEdit   *m_editPart;
    QLineEdit   *m_editInspection;
    QLineEdit   *m_editResult;

    // ─── 检测项列表 ───────────────────────────────────────────────────────
    QTreeWidget *m_treeInspection;

    // ─── 调试面板 ─────────────────────────────────────────────────────────
    QTabWidget  *m_tabDebug;
    QTextEdit   *m_txtRequest;
    QTextEdit   *m_txtResponse;
    QTextEdit   *m_txtLog;

    QPushButton *m_btnClearRequest;
    QPushButton *m_btnClearResponse;
    QPushButton *m_btnClearLog;

    // ─── 命令按钮 ─────────────────────────────────────────────────────────
    QPushButton *m_btnPartList;
    QPushButton *m_btnInspectionList;
    QPushButton *m_btnInspect;
    QPushButton *m_btnInspectContinue;
    QPushButton *m_btnCorrect;
    QPushButton *m_btnCalibrate;
    QPushButton *m_btnReport;
    QPushButton *m_btnExportRBS;

    // 【修改】不再持有自己的连接管理器，统一使用 Singleton
    AicConnectionManager *m_conn;  // 缓存指针，来自 Singleton

    // ─── 修正数据缓存 ─────────────────────────────────────────────────────
    QVector<CorrectionRow> m_correctionRows;  ///< 最近一次 Correct 命令返回的修正数据
};

#endif // AICMAINWINDOW_H

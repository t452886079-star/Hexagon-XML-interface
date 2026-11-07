#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#ifndef AICAUTOMATIONSINGLETON_H
#define AICAUTOMATIONSINGLETON_H

#include <QObject>
#include <QDateTime>
#include <QVector>
#include "AICAutomationLib_global.h"
#include "AicTypes.h"

class AicConnectionManager;

/**
 * @brief AIC 自动化单例类
 *        提供全局唯一的访问入口，封装连接管理和高层业务命令，
 *        外部通过 instance() 获取实例即可使用全部功能。
 */
class AICAUTOMATIONLIBSHARED_EXPORT AicAutomationSingleton : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 获取单例实例（首次调用时创建）
     * @return AicAutomationSingleton 的唯一实例指针
     */
    static AicAutomationSingleton *instance();

    // ─── 连接管理 ─────────────────────────────────────────────────────────

    /**
     * @brief 获取底层连接管理器，用于高级自定义操作
     * @return AicConnectionManager 指针
     */
    AicConnectionManager *connection() const;

    /**
     * @brief 连接到指定的 AIC 设备主机
     * @param host 主机地址（IP 或域名）
     * @param port 端口号
     */
    void connectToHost(const QString &host, int port);

    /**
     * @brief 主动断开与设备的连接
     */
    void disconnect();

    /**
     * @brief 查询当前是否已连接到设备
     * @return true 已连接，false 未连接
     */
    bool isConnected() const;

    /**
     * @brief 查询心跳检测是否正在运行
     * @return true 心跳定时器已启动
     */
    bool isHeartbeatActive() const;

    /**
     * @brief 获取最后一次成功心跳的时间
     * @return QDateTime 时间戳
     */
    QDateTime lastHeartbeatTime() const;

    /**
     * @brief 查询是否为用户主动断开
     * @return true 用户主动断开
     */
    bool isUserDisconnected() const;

    /**
     * @brief 直接发送原始 XML 请求体
     * @param xmlBody 要发送的 XML 字符串
     * @param tag     请求标签，用于标识响应来源
     */
    void sendRequest(const QString &xmlBody, const QString &tag = QString());

    // ─── 高层业务命令（自动构建 XML 并发送） ──────────────────────────────

    /**
     * @brief 查询零件列表（PartList）
     */
    void queryPartList();

    /**
     * @brief 查询指定零件的检测项列表（InspectionList）
     * @param part 零件 GUID
     */
    void queryInspectionList(const QString &part);

    /**
     * @brief 发送检测命令（Inspect）
     * @param part       零件 GUID
     * @param inspection 检测项 GUID
     * @param comment    备注信息（可选）
     */
    void commandInspect(const QString &part, const QString &inspection, const QString &comment = QString());

    /**
     * @brief 发送带结果的检测命令（Inspect-Continue）
     * @param part       零件 GUID
     * @param inspection 检测项 GUID
     * @param result     检测结果 GUID
     * @param comment    备注信息（可选）
     */
    void commandInspectContinue(const QString &part, const QString &inspection, const QString &result, const QString &comment = QString());

    /**
     * @brief 发送修正命令（Correct）
     * @param part   零件 GUID
     * @param result 结果 GUID
     */
    void commandCorrect(const QString &part, const QString &result);

    /**
     * @brief 发送校准命令（Calibrate）
     */
    void commandCalibrate();

    /**
     * @brief 发送报告命令（Report），可选导出路径
     * @param part       零件 GUID
     * @param result     结果 GUID
     * @param exportPath 导出文件路径（可选）
     */
    void commandReport(const QString &part, const QString &result, const QString &exportPath = QString());

    /**
     * @brief 发送导出 RBS 命令（ExportRBS）
     * @param part   零件 GUID
     * @param result 结果 GUID
     */
    void commandExportRBS(const QString &part, const QString &result);

signals:
    /**
     * @brief 连接状态变化信号
     * @param connected true 已连接，false 已断开
     */
    void connectionStateChanged(bool connected);

    /**
     * @brief 收到设备响应信号
     * @param tag     对应请求的标签
     * @param body    响应 XML 内容
     * @param httpCode HTTP 状态码
     */
    void responseReceived(const QString &tag, const QString &body, int httpCode);

    /**
     * @brief 日志消息信号
     * @param msg 日志内容
     */
    void logMessage(const QString &msg);

    /**
     * @brief 心跳检测失败信号（连接丢失前触发）
     */
    void heartbeatFailed();

    /**
     * @brief 连接丢失信号
     * @param userDisconnected true 用户主动断开，false 心跳超时/网络异常
     */
    void connectionLost(bool userDisconnected);

    // ─── 高层业务结果信号（自动解析响应） ──────────────────────────────────

    /**
     * @brief 修正结果信号（自动解析 Correct 响应）
     *        内部自动调用 AicXmlHelper::parseCorrectResponse 解析
     * @param success 是否成功
     * @param rows    修正数据行列表
     * @param errorMsg 错误信息
     */
    void correctResult(bool success, const QVector<CorrectionRow> &rows, const QString &errorMsg);

private:
    /**
     * @brief 私有构造函数（单例模式），创建连接管理器并连接信号
     * @param parent 父对象
     */
    explicit AicAutomationSingleton(QObject *parent = nullptr);

    /**
     * @brief 析构函数，清理单例实例指针
     */
    ~AicAutomationSingleton();

    Q_DISABLE_COPY(AicAutomationSingleton)

    static AicAutomationSingleton *s_instance;  ///< 单例静态实例指针
    AicConnectionManager *m_conn;               ///< 连接管理器实例
};

#endif // AICAUTOMATIONSINGLETON_H

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#ifndef AICCONNECTIONMANAGER_H
#define AICCONNECTIONMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QDateTime>
#include "AICAutomationLib_global.h"

/**
 * @brief AIC 连接管理器
 *        负责通过 HTTP POST 与 AIC 设备建立连接、发送 XML 请求、
 *        接收响应，并实现断线自动重连与心跳检测。
 */
class AICAUTOMATIONLIBSHARED_EXPORT AicConnectionManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数，初始化网络管理器、重连定时器和心跳定时器
     * @param parent 父对象
     */
    explicit AicConnectionManager(QObject *parent = nullptr);

    /**
     * @brief 析构函数，断开连接并释放资源
     */
    ~AicConnectionManager();

    /**
     * @brief 连接到指定的 AIC 设备主机
     * @param host 主机地址（IP 或域名）
     * @param port 端口号
     */
    void connectToHost(const QString &host, int port);

    /**
     * @brief 主动断开与 AIC 设备的连接，停止重连和心跳
     */
    void disconnect();

    /**
     * @brief 查询当前是否已连接
     * @return true 已连接，false 未连接
     */
    bool isConnected() const;

    /**
     * @brief 查询心跳检测是否正在运行
     * @return true 心跳定时器已启动，false 未启动
     */
    bool isHeartbeatActive() const;

    /**
     * @brief 获取最后一次成功心跳的时间
     * @return QDateTime 时间戳，未收到过心跳则返回无效时间
     */
    QDateTime lastHeartbeatTime() const;

    /**
     * @brief 查询是否为用户主动断开（而非心跳超时/网络异常）
     * @return true 用户主动断开，false 非主动断开
     */
    bool isUserDisconnected() const;

    /**
     * @brief 获取当前连接的完整 URL 地址
     * @return 格式为 "http://host:port/" 的 URL 字符串
     */
    QString url() const;

    /**
     * @brief 向 AIC 设备发送 XML 请求体
     * @param xmlBody 要发送的 XML 字符串
     * @param tag     请求标签，用于在响应中标识是哪次请求
     */
    void sendRequest(const QString &xmlBody, const QString &tag = QString());

signals:
    /**
     * @brief 连接状态变化信号
     * @param connected true 表示已连接，false 表示已断开
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
     * @brief 日志消息信号，用于输出调试/运行日志
     * @param msg 日志内容
     */
    void logMessage(const QString &msg);

    /**
     * @brief 心跳检测失败信号（连接丢失前触发）
     *        外部可连接此信号，在连接真正断开前做预处理
     */
    void heartbeatFailed();

    /**
     * @brief 连接丢失信号（区分用户主动断开和异常断开）
     * @param userDisconnected true 为用户主动断开，false 为心跳超时/网络异常
     */
    void connectionLost(bool userDisconnected);

private slots:
    /**
     * @brief 网络请求完成的槽函数，解析响应并处理错误/重连逻辑
     * @param reply 完成的网络回复对象
     */
    void onReplyFinished(QNetworkReply *reply);

    /**
     * @brief 重连定时器超时的槽函数，触发重新连接尝试
     */
    void onReconnectTimer();

    /**
     * @brief 执行连接探测（发送 probe 请求），用于初始连接和心跳检测
     */
    void checkConnection();

private:
    QNetworkAccessManager *m_nam;           ///< 网络访问管理器
    QTimer *m_reconnectTimer;               ///< 断线重连定时器
    QTimer *m_heartbeatTimer;               ///< 心跳检测定时器
    QString m_host;                         ///< 目标主机地址
    int m_port;                             ///< 目标端口
    bool m_connected;                       ///< 当前连接状态
    bool m_userDisconnected;                ///< 用户是否主动断开
    int m_reconnectInterval;                ///< 当前重连间隔（毫秒），指数退避
    QDateTime m_lastHeartbeatTime;          ///< 最后一次成功心跳的时间

    /**
     * @brief 设置连接状态，如果状态发生变化则发射 connectionStateChanged 信号
     * @param c 新的连接状态
     */
    void setConnected(bool c);

    /**
     * @brief 安排一次重连，使用指数退避策略（最大 30 秒）
     */
    void scheduleReconnect();

    /**
     * @brief 构建标准的 HTTP POST 请求对象（Content-Type: application/xml）
     * @return 配置好的 QNetworkRequest
     */
    QNetworkRequest buildRequest() const;
};

#endif // AICCONNECTIONMANAGER_H

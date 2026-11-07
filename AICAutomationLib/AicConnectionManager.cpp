#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif
#include "AicConnectionManager.h"
#include <QNetworkRequest>
#include <QUrl>

/**
 * @brief 构造函数，初始化网络管理器和定时器，连接信号槽
 */
AicConnectionManager::AicConnectionManager(QObject *parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_reconnectTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_port(32459)
    , m_connected(false)
    , m_userDisconnected(false)
    , m_reconnectInterval(3000)
    , m_lastHeartbeatTime()
{
    connect(m_nam, &QNetworkAccessManager::finished, this, &AicConnectionManager::onReplyFinished);
    connect(m_reconnectTimer, &QTimer::timeout, this, &AicConnectionManager::onReconnectTimer);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &AicConnectionManager::checkConnection);
}

/**
 * @brief 析构函数，断开连接
 */
AicConnectionManager::~AicConnectionManager()
{
    disconnect();
}

/**
 * @brief 连接到指定主机，重置重连间隔，启动首次探测
 */
void AicConnectionManager::connectToHost(const QString &host, int port)
{
    m_host = host;
    m_port = port;
    m_userDisconnected = false;
    m_reconnectInterval = 3000;

    emit logMessage(tr("Connecting to %1:%2 ...").arg(host).arg(port));

    m_reconnectTimer->stop();
    checkConnection();
}

/**
 * @brief 主动断开连接，停止所有定时器
 */
void AicConnectionManager::disconnect()
{
    m_userDisconnected = true;
    m_reconnectTimer->stop();
    m_heartbeatTimer->stop();
    setConnected(false);
    emit connectionLost(true);
    emit logMessage(tr("Disconnected by user"));
}

/**
 * @brief 返回当前连接状态
 */
bool AicConnectionManager::isConnected() const
{
    return m_connected;
}

/**
 * @brief 查询心跳定时器是否正在运行
 */
bool AicConnectionManager::isHeartbeatActive() const
{
    return m_heartbeatTimer->isActive();
}

/**
 * @brief 返回最后一次成功心跳的时间
 */
QDateTime AicConnectionManager::lastHeartbeatTime() const
{
    return m_lastHeartbeatTime;
}

/**
 * @brief 返回是否为用户主动断开
 */
bool AicConnectionManager::isUserDisconnected() const
{
    return m_userDisconnected;
}

/**
 * @brief 拼接并返回完整的 HTTP URL
 */
QString AicConnectionManager::url() const
{
    return QString("http://%1:%2/").arg(m_host).arg(m_port);
}

/**
 * @brief 构建标准 HTTP 请求，设置 Content-Type 和 Accept 为 application/xml
 */
QNetworkRequest AicConnectionManager::buildRequest() const
{
    QUrl u(url());
    QNetworkRequest req(u);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
    req.setRawHeader("Accept", "application/xml");
    return req;
}

/**
 * @brief 发送 XML 请求到设备，未连接时输出错误日志
 */
void AicConnectionManager::sendRequest(const QString &xmlBody, const QString &tag)
{
    if (!m_connected) {
        emit logMessage(tr("Error: Not connected"));
        return;
    }

    emit logMessage(tr("[TX %1] Sending %2 bytes").arg(tag).arg(xmlBody.size()));
    QNetworkReply *reply = m_nam->post(buildRequest(), xmlBody.toUtf8());
    reply->setProperty("tag", tag);
    reply->setProperty("requestBody", xmlBody);
}

/**
 * @brief 网络请求完成回调：处理错误、连接探测响应、正常数据响应
 */
void AicConnectionManager::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    QString tag = reply->property("tag").toString();
    int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // 请求出错：记录日志，触发断线重连
    if (reply->error() != QNetworkReply::NoError) {
        QString err = reply->errorString();
        emit logMessage(tr("[ERROR] %1 - %2").arg(tag).arg(err));

        if (m_connected) {
            emit heartbeatFailed();
            setConnected(false);
            emit connectionLost(false);
            scheduleReconnect();
        }
        emit responseReceived(tag, QString(), httpCode);
        return;
    }

    QString body = QString::fromUtf8(reply->readAll());

    // 探测请求成功：标记为已连接，记录心跳时间，启动心跳
    if (tag == "__probe__") {
        m_lastHeartbeatTime = QDateTime::currentDateTime();
        if (!m_connected) {
            setConnected(true);
            m_reconnectTimer->stop();
            m_heartbeatTimer->start(10000);
            m_reconnectInterval = 3000;  // 重置重连间隔
            emit logMessage(tr("Connection established"));
        }
        return;  // 心跳探测响应不向上层发射，避免循环打印
    }

    // 正常响应：发射信号通知上层
    emit logMessage(tr("[RX %1] HTTP %2, %3 bytes").arg(tag).arg(httpCode).arg(body.size()));
    emit responseReceived(tag, body, httpCode);
}

/**
 * @brief 重连定时器触发时，再次尝试连接
 */
void AicConnectionManager::onReconnectTimer()
{
    if (m_userDisconnected) return;
    emit logMessage(tr("Reconnecting..."));
    checkConnection();
}

/**
 * @brief 发送心跳探测请求（State 查询），用于判断设备是否在线
 */
void AicConnectionManager::checkConnection()
{
    QString probe = "<?xml version='1.0' encoding='utf-8'?>\n<BSExecQuery>\n <QueryID>State</QueryID>\n</BSExecQuery>";
    QNetworkReply *reply = m_nam->post(buildRequest(), probe.toUtf8());
    reply->setProperty("tag", "__probe__");
}

/**
 * @brief 内部设置连接状态，仅在状态变化时发射信号
 */
void AicConnectionManager::setConnected(bool c)
{
    if (m_connected != c) {
        m_connected = c;
        emit connectionStateChanged(c);
    }
}

/**
 * @brief 安排重连：停止心跳，启动重连定时器，间隔指数退避（最大 30 秒）
 */
void AicConnectionManager::scheduleReconnect()
{
    if (m_userDisconnected) return;
    m_heartbeatTimer->stop();
    emit logMessage(tr("Connection lost. Retrying in %1s ...").arg(m_reconnectInterval / 1000));
    m_reconnectTimer->start(m_reconnectInterval);
    m_reconnectInterval = qMin(m_reconnectInterval * 2, 30000);
}

#pragma once
#include "Transport.h"
#include <QElapsedTimer>
#include <QUdpSocket>

class HudpTransport final : public Transport
{
    Q_OBJECT
public:
    explicit HudpTransport(QObject *parent = nullptr);
    QString name() const override { return "HUDP"; }
    bool connected() const override { return m_connected; }
    void setConnected(bool connected) { if (!connected) disconnectFromTarget(); }
    // Connect to a remote HUDP endpoint by host and port
    // 通过主机和端口连接到远程 HUDP 端点
    void connectToTarget(const QString &host, quint16 port);
    // Disconnect from the current HUDP target
    // 断开与当前 HUDP 目标的连接
    void disconnectFromTarget();
    // Send a legacy binary frame via HUDP datagram
    // 通过 HUDP 数据报发送传统二进制帧
    void write(const QByteArray &frame) override;
private:
    // Build and send a HUDP packet with the given type, flags, and payload
    // 构建并发送带有指定类型、标志和负载的 HUDP 数据包
    void send(quint8 type, quint16 flags, const QByteArray &payload);
    QUdpSocket m_socket; QElapsedTimer m_clock; QHostAddress m_host; quint16 m_port = 0; quint32 m_sessionId = 0; quint32 m_sequence = 0; bool m_connected = false;
};

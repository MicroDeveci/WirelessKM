#include "HudpTransport.h"
#include "HudpProtocol.h"
#include <QRandomGenerator>

// Constructor: initialise the elapsed timer for HUDP timestamps
// 构造函数: 初始化 HUDP 时间戳用的计时器
HudpTransport::HudpTransport(QObject *parent) : Transport(parent) { m_clock.start(); }
// Connect to a remote HUDP endpoint, generate a session ID, and send a StateReset
// 连接到远程 HUDP 端点，生成会话 ID 并发送 StateReset
void HudpTransport::connectToTarget(const QString &host, quint16 port) {
    m_host = QHostAddress(host); if (m_host.isNull() || !port) { emit errorOccurred("Invalid HUDP endpoint"); return; }
    m_port = port; m_sessionId = QRandomGenerator::global()->generate(); m_sequence = 0; m_connected = true; emit connectedChanged();
    send(HudpProtocol::StateReset, HudpProtocol::ResetState, {});
}
// Disconnect from the HUDP target and send a StateReset
// 断开与 HUDP 目标的连接并发送 StateReset
void HudpTransport::disconnectFromTarget() { if (!m_connected) return; send(HudpProtocol::StateReset, HudpProtocol::ResetState, {}); m_connected = false; emit connectedChanged(); }
// Send a legacy binary frame as a HUDP datagram
// 将传统二进制帧作为 HUDP 数据报发送
void HudpTransport::write(const QByteArray &frame) { if (!m_connected) { emit errorOccurred("HUDP transport is not connected"); return; } send(HudpProtocol::LegacyBinaryFrame, 0, frame); }
// Build and send a HUDP packet as a UDP datagram
// 构建 HUDP 数据包并作为 UDP 数据报发送
void HudpTransport::send(quint8 type, quint16 flags, const QByteArray &payload) {
    const QByteArray packet = HudpProtocol::build(type, flags, m_sessionId, ++m_sequence, quint32(m_clock.elapsed()), payload);
    if (packet.isEmpty() || m_socket.writeDatagram(packet, m_host, m_port) != packet.size()) { emit errorOccurred("HUDP datagram write failed"); return; }
}

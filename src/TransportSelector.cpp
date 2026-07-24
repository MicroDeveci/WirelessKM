#include "TransportSelector.h"
#include "Transport.h"

// Constructor / 构造函数
TransportSelector::TransportSelector(QObject *parent)
    : QObject(parent)
{
}

// Register BLE, UART, and HUDP transports and wire their connection signals
// 注册 BLE、UART 和 HUDP 传输并连接其连接状态信号
void TransportSelector::setTransports(Transport *ble, Transport *uart, Transport *hudp)
{
    m_ble = ble;
    m_uart = uart;
    m_hudp = hudp;

    const auto wire = [this](Transport *transport) {
        if (transport)
            connect(transport, &Transport::connectedChanged, this, &TransportSelector::updateActiveTransport);
    };
    wire(m_ble);
    wire(m_uart);
    wire(m_hudp);

    updateActiveTransport();
}

// Return the name of the active transport, or "None" if none connected
// 返回活跃传输的名称，无活跃传输时返回 "None"
QString TransportSelector::activeTransportName() const
{
    return m_activeTransport ? m_activeTransport->name() : QString("None");
}

// Re-evaluate which transport should be active based on connection state
// 根据连接状态重新评估应使用哪个传输
void TransportSelector::updateActiveTransport()
{
    Transport *next = chooseActiveTransport();
    if (m_activeTransport == next)
        return;
    m_activeTransport = next;
    emit activeTransportChanged();
}

// Select the first connected transport in priority order: BLE, UART, HUDP
// 按优先级顺序选择第一个已连接的传输: BLE、UART、HUDP
Transport *TransportSelector::chooseActiveTransport() const
{
    if (m_ble && m_ble->connected())
        return m_ble;
    if (m_uart && m_uart->connected())
        return m_uart;
    if (m_hudp && m_hudp->connected())
        return m_hudp;
    return nullptr;
}

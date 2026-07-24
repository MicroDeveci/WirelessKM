#include "InterfaceLayer.h"
#include "SerialManager.h"
#include "ConnectionStateManager.h"

// Register UART transport command routes (refresh ports, connect with port/baud, disconnect)
// 注册UART传输相关的命令路由（刷新端口、指定端口和波特率连接、断开连接）
void InterfaceLayer::registerUartRoutes()
{
    m_routes["uart.refreshPorts"] = [this](const QVariantMap &) {
        traceDataFlow("route uart.refreshPorts", "port refresh requested");
        m_serialManager->refreshPorts();
        appendLog(3, "UART", "Port scan: " + m_serialManager->availablePorts().join(", "));
    };
    m_routes["uart.connect"] = [this](const QVariantMap &p) {
        traceDataFlow("route uart.connect", "connect requested", p);
        m_connectionStateManager->connectUart(p["port"].toString(), p["baudrate"].toInt());
        appendLog(0, "UART", "Connecting to " + p["port"].toString());
    };
    m_routes["uart.disconnect"] = [this](const QVariantMap &) {
        traceDataFlow("route uart.disconnect", "disconnect requested");
        m_connectionStateManager->disconnectUart();
        appendLog(0, "UART", "Disconnected");
    };
}

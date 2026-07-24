#include "InterfaceLayer.h"
#include "BleManager.h"
#include "ConnectionStateManager.h"

// Register BLE transport command routes (start/stop scan, connect, disconnect)
// 注册蓝牙传输相关的命令路由（开始/停止扫描、连接、断开连接）
void InterfaceLayer::registerBleRoutes()
{
    m_routes["ble.startScan"] = [this](const QVariantMap &) {
        traceDataFlow("route ble.startScan", "scan start requested");
        m_connectionStateManager->startBleScan();
    };
    m_routes["ble.stopScan"] = [this](const QVariantMap &) {
        traceDataFlow("route ble.stopScan", "scan stop requested");
        m_connectionStateManager->stopBleScan();
    };
    m_routes["ble.connect"] = [this](const QVariantMap &p) {
        traceDataFlow("route ble.connect", "connect requested", p);
        m_connectionStateManager->connectBle(p["address"].toString());
        appendLog(0, "BLE", "Connecting to " + p["address"].toString());
    };
    m_routes["ble.disconnect"] = [this](const QVariantMap &) {
        traceDataFlow("route ble.disconnect", "disconnect requested");
        m_connectionStateManager->disconnectBle();
        appendLog(0, "BLE", "Disconnected");
    };
}

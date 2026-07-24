#include "InterfaceLayer.h"
#include "ConnectionStateManager.h"

// Register HUDP transport command routes (connect with target/port, disconnect)
// 注册HUDP传输相关的命令路由（指定目标和端口连接、断开连接）
void InterfaceLayer::registerHudpRoutes()
{
    m_routes["hudp.connect"] = [this](const QVariantMap &p) {
        m_connectionStateManager->connectHudp(p.value("target").toString(), p.value("port", 45820).toInt());
        appendLog(0, "HUDP", "Endpoint " + hudpTarget() + ":" + QString::number(hudpPort()));
    };
    m_routes["hudp.disconnect"] = [this](const QVariantMap &) {
        m_connectionStateManager->disconnectHudp();
    };
}

#include "InterfaceLayer.h"

#include "MouseInputManager.h"
#include "MouseProtocol.h"

// Register mouse input command routes (move, button press/release, wheel, release all)
// 注册鼠标输入相关的命令路由（移动、按键按下/释放、滚轮、全部释放）
void InterfaceLayer::registerMouseRoutes()
{
    m_routes["mouse.move"] = [this](const QVariantMap &params) {
        const int dx = params.value("dx").toInt();
        const int dy = params.value("dy").toInt();
        traceDataFlow("route mouse.move", "relative mouse movement requested",
                      {{"dx", dx}, {"dy", dy}});
        m_mouseInputManager->moveBy(dx, dy);
    };
    m_routes["mouse.button"] = [this](const QVariantMap &params) {
        const int button = params.value("button").toInt();
        const bool pressed = params.value("pressed").toBool();
        if (button != MouseProtocol::LeftButton
            && button != MouseProtocol::RightButton
            && button != MouseProtocol::MiddleButton) {
            appendLog(1, "Mouse", "Ignored invalid mouse button: " + QString::number(button));
            return;
        }
        traceDataFlow("route mouse.button", "mouse button transition requested",
                      {{"button", button}, {"pressed", pressed}});
        m_mouseInputManager->setButton(static_cast<quint8>(button), pressed);
    };
    m_routes["mouse.wheel"] = [this](const QVariantMap &params) {
        const int delta = params.value("delta").toInt();
        traceDataFlow("route mouse.wheel", "mouse wheel movement requested", {{"delta", delta}});
        m_mouseInputManager->wheelBy(delta);
    };
    m_routes["mouse.releaseAll"] = [this](const QVariantMap &) {
        traceDataFlow("route mouse.releaseAll", "mouse release requested");
        m_mouseInputManager->releaseAll();
    };
}

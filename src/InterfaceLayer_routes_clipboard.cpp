#include "InterfaceLayer.h"
#include "ClipboardTextSource.h"

// Register clipboard-related command routes (send clipboard text, toggle watch mode)
// 注册剪贴板相关的命令路由（发送剪贴板文本、切换监视模式）
void InterfaceLayer::registerClipboardRoutes()
{
    m_routes["clipboard.send"] = [this](const QVariantMap &) {
        traceDataFlow("route clipboard.send", "clipboard send requested");
        m_clipboardTextSource->requestText();
    };
    m_routes["clipboard.watchToggled"] = [this](const QVariantMap &p) {
        const bool enabled = p["enabled"].toBool();
        traceDataFlow("route clipboard.watchToggled", "clipboard watch toggled", {{"enabled", enabled}});
        m_clipboardTextSource->setWatching(enabled);
        appendLog(0, "Clipboard", enabled ? "Clipboard watch ON" : "Clipboard watch OFF");
    };
}

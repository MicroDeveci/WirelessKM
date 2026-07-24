#include "InterfaceLayer.h"
#include "CaptureManager.h"
#include "KeySettingManager.h"
#include "MouseInputManager.h"
#include "TransportSelector.h"

// Register key-related command routes (change capture hotkey, toggle capture)
// 注册按键相关的命令路由（更改捕获热键、切换捕获状态）
void InterfaceLayer::registerKeyRoutes()
{
    m_routes["key.captureHotkeyChanged"] = [this](const QVariantMap &p) {
        traceDataFlow("route key.captureHotkeyChanged", "hotkey update requested", p);
        m_keySettingManager->setCaptureHotkey(p["newHotkey"].toString());
        appendLog(0, "KeySetting", "Capture hotkey: " + p["newHotkey"].toString());
        saveSettings();
    };
    m_routes["key.capturePasteHotkeyChanged"] = [this](const QVariantMap &p) {
        traceDataFlow("route key.capturePasteHotkeyChanged", "hotkey update requested", p);
        m_keySettingManager->setCapturePasteHotkey(p["newHotkey"].toString());
        appendLog(0, "KeySetting", "Paste hotkey: " + p["newHotkey"].toString());
        saveSettings();
    };
    m_routes["key.captureToggled"] = [this](const QVariantMap &p) {
        traceDataFlow("route key.captureToggled", "capture toggle requested", p);
        m_keySettingManager->setCaptureEnabled(p["enabled"].toBool());
        appendLog(0, "KeySetting", m_keySettingManager->captureEnabled() ? "Capture ON" : "Capture OFF");
        saveSettings();
    };
}

// Register passthrough (app foreground capture) command routes (listen, exclusive, mouse)
// 注册透传（应用前台捕获）相关的命令路由（监听、独占模式、鼠标）
void InterfaceLayer::registerPassthroughRoutes()
{
    m_routes["passthrough.listenToggled"] = [this](const QVariantMap &p) {
        const bool enabled = p["enabled"].toBool();
        traceDataFlow("route passthrough.listenToggled", "listen mode requested", {{"enabled", enabled}});
        m_captureManager->setAppCaptureListening(enabled);
        appendLog(0, "Passthrough", enabled ? "Listen mode ON" : "Listen mode OFF");
    };
    m_routes["passthrough.exclusiveToggled"] = [this](const QVariantMap &p) {
        const bool enabled = p["enabled"].toBool();
        traceDataFlow("route passthrough.exclusiveToggled", "exclusive app mode requested", {{"enabled", enabled}});
        m_captureManager->setAppCaptureExclusive(enabled);
        appendLog(0, "Passthrough", enabled ? "App exclusive mode ON" : "App exclusive mode OFF");
    };
    m_routes["passthrough.mouseListenToggled"] = [this](const QVariantMap &p) {
        const bool enabled = p["enabled"].toBool();
        traceDataFlow("route passthrough.mouseListenToggled", "mouse listen mode requested",
                      {{"enabled", enabled}});
        if (enabled && !m_transportSelector->activeTransport()) {
            appendLog(1, "Passthrough", "Mouse listen rejected: no active transport");
            return;
        }
        m_captureManager->setMouseCaptureListening(enabled);
        if (!enabled)
            m_mouseInputManager->releaseAll();
        appendLog(0, "Passthrough", enabled ? "Mouse listen mode ON" : "Mouse listen mode OFF");
    };
}

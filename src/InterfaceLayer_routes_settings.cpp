#include "InterfaceLayer.h"
#include "KeySettingManager.h"
#include "SettingsManager.h"
#include "ConnectionStateManager.h"

// Register settings management command routes (language, auto-start, remember, import, reset, theme)
// 注册设置管理相关的命令路由（语言、开机自启、记住连接、导入、重置、主题）
void InterfaceLayer::registerSettingsRoutes()
{
    m_routes["settings.uiLanguageChanged"] = [this](const QVariantMap &p) {
        const QString language = p.value("language").toString() == "en-US" ? "en-US" : "zh-CN";
        if (m_settingsManager->uiLanguage() == language)
            return;
        m_settingsManager->setUiLanguage(language);
        emit uiLanguageChanged();
        saveSettings();
    };
    m_routes["settings.autoStartToggled"] = [this](const QVariantMap &p) {
        const bool enabled = p["enabled"].toBool();
        traceDataFlow("route settings.autoStartToggled", "setting changed", {{"enabled", enabled}});
        if (!applyAutoStart(enabled)) {
            setSettingsStatus("Auto start update failed");
            appendLog(2, "Settings", settingsStatus());
            emit autoStartChanged();
            emit settingsStatusChanged();
            return;
        }
        m_settingsManager->setAutoStart(enabled);
        emit autoStartChanged();
        saveSettings();
    };

    m_routes["settings.rememberLastConnectionToggled"] = [this](const QVariantMap &p) {
        m_connectionStateManager->setRememberLastConnection(p["enabled"].toBool());
        traceDataFlow("route settings.rememberLastConnectionToggled", "setting changed",
                      {{"enabled", m_settingsManager->rememberLastConnection()}});
        emit rememberLastConnectionChanged();
        saveSettings();
    };

    m_routes["settings.importConfig"] = [this](const QVariantMap &p) {
        const QString path = p["path"].toString();
        traceDataFlow("route settings.importConfig", "import requested", {{"path", path}});
        importSettingsFromFile(path);
    };

    m_routes["settings.resetToDefaults"] = [this](const QVariantMap &) {
        traceDataFlow("route settings.resetToDefaults", "reset requested");
        m_settingsManager->resetToDefaults();
        emit autoStartChanged();
        emit rememberLastConnectionChanged();
        emit configFilePathChanged();
        emit darkModeChanged();
        appendLog(1, "Settings", "Default config restored");
    };

    m_routes["settings.themeChanged"] = [this](const QVariantMap &p) {
        const int mode = p["mode"].toInt();
        traceDataFlow("route settings.themeChanged", "theme changed", {{"mode", mode}});
        m_settingsManager->setDarkMode(mode);
        emit darkModeChanged();
        saveSettings();
    };
}

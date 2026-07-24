#include "KeySettingManager.h"
#include "DebugTrace.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>

// Construct and load key settings from persistent storage.
// 构造并从持久存储加载按键设置。
KeySettingManager::KeySettingManager(QObject *parent)
    : QObject(parent)
{
    loadFromSettings();
}

// Enable or disable the global capture hotkey.
// 启用或禁用全局捕获热键。
void KeySettingManager::setCaptureEnabled(bool v)
{
    if (m_captureEnabled != v) {
        m_captureEnabled = v;
        emit captureEnabledChanged();
    }
}

// Set the capture hotkey string (e.g. "Ctrl+`").
// 设置捕获热键字符串（如 "Ctrl+`"）。
void KeySettingManager::setCaptureHotkey(const QString &v)
{
    if (m_captureHotkey != v) {
        m_captureHotkey = v;
        emit captureHotkeyChanged();
    }
}

// Set the capture-paste hotkey string (e.g. "Ctrl+V").
// 设置捕获粘贴热键字符串（如 "Ctrl+V"）。
void KeySettingManager::setCapturePasteHotkey(const QString &v)
{
    if (m_capturePasteHotkey != v) {
        m_capturePasteHotkey = v;
        emit capturePasteHotkeyChanged();
    }
}

// Reset all key settings to factory defaults.
// 将所有按键设置重置为出厂默认值。
void KeySettingManager::resetToDefaults()
{
    setCaptureEnabled(true);
    setCaptureHotkey("Ctrl+`");
    setCapturePasteHotkey("Ctrl+V");
    if (DebugTrace::enabled()) qDebug() << "[KeySettingManager] Reset to defaults";
}

// Load key settings from QSettings persistent storage.
// 从 QSettings 持久存储加载按键设置。
void KeySettingManager::loadFromSettings()
{
    QSettings s("ESP32KB", "Bridge");
    m_captureEnabled = s.value("captureEnabled", true).toBool();
    m_captureHotkey = s.value("captureHotkey", "Ctrl+`").toString();
    m_capturePasteHotkey = s.value("capturePasteHotkey", "Ctrl+V").toString();
    if (DebugTrace::enabled()) qDebug() << "[KeySettingManager] Loaded settings"
             << "\n  capture:" << m_captureHotkey << m_captureEnabled
             << "\n  paste:" << m_capturePasteHotkey;
}

// Save key settings to QSettings persistent storage.
// 将按键设置保存到 QSettings 持久存储。
void KeySettingManager::saveToSettings()
{
    QSettings s("ESP32KB", "Bridge");
    s.setValue("captureEnabled", m_captureEnabled);
    s.setValue("captureHotkey", m_captureHotkey);
    s.setValue("capturePasteHotkey", m_capturePasteHotkey);
    s.sync();
    if (DebugTrace::enabled()) qDebug() << "[KeySettingManager] Saved settings";
}

// Export key settings as a formatted JSON string.
// 将按键设置导出为格式化的 JSON 字符串。
QString KeySettingManager::exportToJson() const
{
    QJsonObject obj;
    obj["captureEnabled"] = m_captureEnabled;
    obj["captureHotkey"] = m_captureHotkey;
    obj["capturePasteHotkey"] = m_capturePasteHotkey;
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

// Import key settings from a JSON string; returns true on success.
// 从 JSON 字符串导入按键设置；成功返回 true。
bool KeySettingManager::importFromJson(const QString &json)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "[KeySettingManager] JSON parse error:" << err.errorString();
        return false;
    }
    QJsonObject obj = doc.object();
    if (obj.contains("captureEnabled")) setCaptureEnabled(obj["captureEnabled"].toBool());
    if (obj.contains("captureHotkey")) setCaptureHotkey(obj["captureHotkey"].toString());
    if (obj.contains("capturePasteHotkey")) setCapturePasteHotkey(obj["capturePasteHotkey"].toString());
    saveToSettings();
    if (DebugTrace::enabled()) qDebug() << "[KeySettingManager] Imported from JSON";
    return true;
}

// Export key settings to a JSON file; returns true on success.
// 将按键设置导出到 JSON 文件；成功返回 true。
bool KeySettingManager::exportToFile(const QString &filePath) const
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[KeySettingManager] Cannot write to" << filePath;
        return false;
    }
    f.write(exportToJson().toUtf8());
    f.close();
    if (DebugTrace::enabled()) qDebug() << "[KeySettingManager] Exported to" << filePath;
    return true;
}

// Import key settings from a JSON file; returns true on success.
// 从 JSON 文件导入按键设置；成功返回 true。
bool KeySettingManager::importFromFile(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[KeySettingManager] Cannot read" << filePath;
        return false;
    }
    QString json = QString::fromUtf8(f.readAll());
    f.close();
    return importFromJson(json);
}

#pragma once

#include <QObject>
#include <QSettings>

class KeySettingManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool captureEnabled READ captureEnabled WRITE setCaptureEnabled NOTIFY captureEnabledChanged)
    Q_PROPERTY(QString captureHotkey READ captureHotkey WRITE setCaptureHotkey NOTIFY captureHotkeyChanged)
    Q_PROPERTY(QString capturePasteHotkey READ capturePasteHotkey WRITE setCapturePasteHotkey NOTIFY capturePasteHotkeyChanged)

public:
    // Construct and load settings from persistent storage.
    // 构造并从持久存储加载设置。
    explicit KeySettingManager(QObject *parent = nullptr);

    // Return whether the global capture hotkey is enabled.
    // 返回全局捕获热键是否启用。
    bool captureEnabled() const { return m_captureEnabled; }
    // Return the capture hotkey string (e.g. "Ctrl+`").
    // 返回捕获热键字符串（如 "Ctrl+`"）。
    QString captureHotkey() const { return m_captureHotkey; }
    // Return the capture-paste hotkey string (e.g. "Ctrl+V").
    // 返回捕获粘贴热键字符串（如 "Ctrl+V"）。
    QString capturePasteHotkey() const { return m_capturePasteHotkey; }

    // Enable or disable the global capture hotkey.
    // 启用或禁用全局捕获热键。
    void setCaptureEnabled(bool v);
    // Set the capture hotkey string.
    // 设置捕获热键字符串。
    void setCaptureHotkey(const QString &v);
    // Set the capture-paste hotkey string.
    // 设置捕获粘贴热键字符串。
    void setCapturePasteHotkey(const QString &v);

    // Reset all key settings to factory defaults.
    // 将所有按键设置重置为出厂默认值。
    Q_INVOKABLE void resetToDefaults();
    // Load key settings from QSettings persistent storage.
    // 从 QSettings 持久存储加载按键设置。
    Q_INVOKABLE void loadFromSettings();
    // Save key settings to QSettings persistent storage.
    // 将按键设置保存到 QSettings 持久存储。
    Q_INVOKABLE void saveToSettings();

    // Export key settings as a JSON string.
    // 将按键设置导出为 JSON 字符串。
    Q_INVOKABLE QString exportToJson() const;
    // Import key settings from a JSON string; returns true on success.
    // 从 JSON 字符串导入按键设置；成功返回 true。
    Q_INVOKABLE bool importFromJson(const QString &json);
    // Export key settings to a JSON file; returns true on success.
    // 将按键设置导出到 JSON 文件；成功返回 true。
    Q_INVOKABLE bool exportToFile(const QString &filePath) const;
    // Import key settings from a JSON file; returns true on success.
    // 从 JSON 文件导入按键设置；成功返回 true。
    Q_INVOKABLE bool importFromFile(const QString &filePath);

signals:
    void captureEnabledChanged();
    void captureHotkeyChanged();
    void capturePasteHotkeyChanged();
    void configImported();
    void configExported();

private:
    bool    m_captureEnabled = true;
    QString m_captureHotkey = "Ctrl+`";
    QString m_capturePasteHotkey = "Ctrl+V";
};

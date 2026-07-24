#pragma once

#include <QObject>
#include <QString>

class KeySettingManager;

// Owns persistent desktop settings.  InterfaceLayer deliberately does not
// perform file or OS-startup work; it only exposes these values to QML.
class SettingsManager : public QObject
{
    Q_OBJECT
public:
    // Construct with a reference to the key settings manager.
    // 使用按键设置管理器的引用进行构造。
    explicit SettingsManager(KeySettingManager *keySettings, QObject *parent = nullptr);

    // Return whether auto-start on login is enabled.
    // 返回登录时自启动是否启用。
    bool autoStart() const { return m_autoStart; }
    // Return whether last connection should be remembered for auto-restore.
    // 返回是否记住上次连接以自动恢复。
    bool rememberLastConnection() const { return m_rememberLastConnection; }
    // Return the full path to the JSON config file.
    // 返回 JSON 配置文件的完整路径。
    QString configFilePath() const { return m_configFilePath; }
    // Return the full path to the application log file.
    // 返回应用程序日志文件的完整路径。
    QString logFilePath() const;
    // Return the current status message string.
    // 返回当前状态消息字符串。
    QString status() const { return m_status; }
    // Return the UI language code ("zh-CN" or "en-US").
    // 返回 UI 语言代码（"zh-CN" 或 "en-US"）。
    QString uiLanguage() const { return m_uiLanguage; }
    // Return the dark mode setting (0=auto, 1=light, 2=dark).
    // 返回深色模式设置（0=自动，1=浅色，2=深色）。
    int darkMode() const { return m_darkMode; }

    // Return the transport type of the last connection.
    // 返回上次连接的传输类型。
    QString lastConnectionTransport() const { return m_lastConnectionTransport; }
    // Return whether the last connection was active.
    // 返回上次连接是否处于活动状态。
    bool lastConnectionConnected() const { return m_lastConnectionConnected; }
    // Return the last UART port name.
    // 返回上次 UART 端口名称。
    QString lastUartPort() const { return m_lastUartPort; }
    // Return the last UART baud rate.
    // 返回上次 UART 波特率。
    int lastUartBaudrate() const { return m_lastUartBaudrate; }
    // Return the last BLE device address.
    // 返回上次 BLE 设备地址。
    QString lastBleAddress() const { return m_lastBleAddress; }
    // Return the last HUDP target host.
    // 返回上次 HUDP 目标主机。
    QString lastHudpTarget() const { return m_lastHudpTarget; }
    // Return the last HUDP target port.
    // 返回上次 HUDP 目标端口。
    int lastHudpPort() const { return m_lastHudpPort; }

    // Load settings from the default config file; creates it if missing.
    // 从默认配置文件加载设置；若不存在则创建。
    void load();
    // Save current settings to the config file.
    // 将当前设置保存到配置文件。
    void save();
    // Import settings from a JSON file; merges into current state.
    // 从 JSON 文件导入设置；合并到当前状态。
    bool importFromFile(const QString &filePath);
    // Apply or remove the OS auto-start registration.
    // 应用或移除操作系统自启动注册。
    bool applyAutoStart(bool enabled);
    // Reset all settings to factory defaults and save.
    // 将所有设置重置为出厂默认值并保存。
    void resetToDefaults();

    // Set the auto-start on login flag.
    // 设置登录时自启动标志。
    void setAutoStart(bool enabled) { m_autoStart = enabled; }
    // Set the remember-last-connection flag.
    // 设置记住上次连接标志。
    void setRememberLastConnection(bool enabled) { m_rememberLastConnection = enabled; }
    // Set the UI language ("zh-CN" or "en-US").
    // 设置 UI 语言（"zh-CN" 或 "en-US"）。
    void setUiLanguage(const QString &language) { m_uiLanguage = language == "en-US" ? "en-US" : "zh-CN"; }
    // Set the dark mode (0=auto, 1=light, 2=dark).
    // 设置深色模式（0=自动，1=浅色，2=深色）。
    void setDarkMode(int mode) { m_darkMode = mode; }
    // Store the last connection transport and connected state.
    // 存储上次连接的传输类型和连接状态。
    void setLastConnection(const QString &transport, bool connected);
    // Store the last UART port and baud rate.
    // 存储上次 UART 端口和波特率。
    void setLastUart(const QString &port, int baudrate);
    // Store the last BLE device address.
    // 存储上次 BLE 设备地址。
    void setLastBleAddress(const QString &address) { m_lastBleAddress = address; }
    // Store the last HUDP target and port.
    // 存储上次 HUDP 目标和端口。
    void setLastHudp(const QString &target, int port);
    // Set and emit the status message string.
    // 设置并发出状态消息字符串。
    void setStatus(const QString &status);
    // Remember a successful connection for future auto-restore.
    // 记住成功的连接以便将来自动恢复。
    void rememberConnection(const QString &transport, const QString &endpoint = QString(), int port = 0);
    // Mark a connection as disconnected in the saved settings.
    // 在保存的设置中将连接标记为断开。
    void markConnectionDisconnected(const QString &transport);

public slots:
    // Attempt to restore the last remembered connection on startup.
    // 在启动时尝试恢复上次记住的连接。
    void restoreLastConnection();

signals:
    void configFilePathChanged();
    void statusChanged();
    void restoreUartRequested(const QString &port, int baudrate);
    void restoreBleRequested(const QString &address);
    void restoreHudpRequested(const QString &target, int port);

private:
    // Parse a JSON byte array and apply values to member variables.
    // 解析 JSON 字节数组并将值应用到成员变量。
    void readJson(const QByteArray &json, bool useCurrentValues);
    // Return the default config file path based on platform conventions.
    // 根据平台惯例返回默认配置文件路径。
    QString defaultConfigFilePath() const;

    KeySettingManager *m_keySettings = nullptr;
    bool m_autoStart = false;
    bool m_rememberLastConnection = false;
    QString m_uiLanguage = "zh-CN";
    int m_darkMode = 0;
    QString m_configFilePath;
    QString m_status;
    QString m_lastConnectionTransport;
    bool m_lastConnectionConnected = false;
    QString m_lastUartPort;
    int m_lastUartBaudrate = 115200;
    QString m_lastBleAddress;
    QString m_lastHudpTarget;
    int m_lastHudpPort = 45820;
};

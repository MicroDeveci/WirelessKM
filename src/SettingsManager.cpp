#include "SettingsManager.h"

#include "KeySettingManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

// Construct with a reference to the key settings manager and resolve the config path.
// 使用按键设置管理器的引用进行构造并解析配置路径。
SettingsManager::SettingsManager(KeySettingManager *keySettings, QObject *parent)
    : QObject(parent), m_keySettings(keySettings), m_configFilePath(defaultConfigFilePath())
{
}

// Return the full path to the application log file (app.log alongside config).
// 返回应用程序日志文件的完整路径（与配置文件同目录的 app.log）。
QString SettingsManager::logFilePath() const
{
    return QDir(QFileInfo(m_configFilePath).absolutePath()).filePath("app.log");
}

// Load settings from the default config file; creates it if missing.
// 从默认配置文件加载设置；若不存在则创建。
void SettingsManager::load()
{
    m_configFilePath = defaultConfigFilePath();
    emit configFilePathChanged();
    QFile file(m_configFilePath);
    if (!file.exists()) {
        save();
        setStatus("Config created: " + m_configFilePath);
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        setStatus("Config load failed: " + file.errorString());
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        setStatus("Config load failed: invalid json");
        return;
    }
    readJson(doc.toJson(QJsonDocument::Compact), false);
    applyAutoStart(m_autoStart);
    setStatus("Config loaded: " + m_configFilePath);
}

// Parse a JSON byte array and apply values to member variables.
// 解析 JSON 字节数组并将值应用到成员变量。
void SettingsManager::readJson(const QByteArray &json, bool useCurrentValues)
{
    const QJsonObject root = QJsonDocument::fromJson(json).object();
    const QJsonObject settings = root.value("settings").toObject();
    const QJsonObject key = root.value("keySettings").toObject();
    const QJsonObject last = root.value("lastConnection").toObject();
    const QJsonObject uart = last.value("uart").toObject();
    const QJsonObject ble = last.value("ble").toObject();
    // Read the former KMUDP setting once so existing desktop settings migrate
    // without losing their target endpoint.
    const QJsonObject hudp = last.contains("hudp")
        ? last.value("hudp").toObject()
        : last.value("kmudp").toObject();
    m_autoStart = settings.value("autoStart").toBool(useCurrentValues ? m_autoStart : false);
    m_rememberLastConnection = settings.value("rememberLastConnection").toBool(useCurrentValues ? m_rememberLastConnection : false);
    m_uiLanguage = settings.value("uiLanguage").toString(useCurrentValues ? m_uiLanguage : "zh-CN") == "en-US" ? "en-US" : "zh-CN";
    m_darkMode = settings.value("darkMode").toInt(useCurrentValues ? m_darkMode : 0);
    if (m_keySettings) {
        if (!useCurrentValues || key.contains("captureEnabled")) m_keySettings->setCaptureEnabled(key.value("captureEnabled").toBool(m_keySettings->captureEnabled()));
        if (!useCurrentValues || key.contains("captureHotkey")) m_keySettings->setCaptureHotkey(key.value("captureHotkey").toString(m_keySettings->captureHotkey()));
        if (!useCurrentValues || key.contains("capturePasteHotkey")) m_keySettings->setCapturePasteHotkey(key.value("capturePasteHotkey").toString(m_keySettings->capturePasteHotkey()));
    }
    m_lastConnectionTransport = last.value("transport").toString(useCurrentValues ? m_lastConnectionTransport : QString());
    if (m_lastConnectionTransport == "KMUDP")
        m_lastConnectionTransport = "HUDP";
    m_lastConnectionConnected = last.value("connected").toBool(useCurrentValues ? m_lastConnectionConnected : false);
    m_lastUartPort = uart.value("port").toString(useCurrentValues ? m_lastUartPort : QString());
    m_lastUartBaudrate = uart.value("baudrate").toInt(useCurrentValues ? m_lastUartBaudrate : 115200);
    m_lastBleAddress = ble.value("address").toString(useCurrentValues ? m_lastBleAddress : QString());
    m_lastHudpTarget = hudp.value("target").toString(useCurrentValues ? m_lastHudpTarget : QString());
    m_lastHudpPort = hudp.value("port").toInt(useCurrentValues ? m_lastHudpPort : 45820);
}

// Save all current settings to the JSON config file atomically.
// 将所有当前设置原子性地保存到 JSON 配置文件。
void SettingsManager::save()
{
    QDir().mkpath(QFileInfo(m_configFilePath).absolutePath());
    QJsonObject settings{{"autoStart", m_autoStart}, {"rememberLastConnection", m_rememberLastConnection}, {"uiLanguage", m_uiLanguage}, {"darkMode", m_darkMode}};
    QJsonObject key;
    if (m_keySettings) {
        key = {{"captureEnabled", m_keySettings->captureEnabled()}, {"captureHotkey", m_keySettings->captureHotkey()}, {"capturePasteHotkey", m_keySettings->capturePasteHotkey()}};
    }
    QJsonObject last{{"transport", m_lastConnectionTransport}, {"connected", m_lastConnectionConnected},
                     {"uart", QJsonObject{{"port", m_lastUartPort}, {"baudrate", m_lastUartBaudrate}}},
                     {"ble", QJsonObject{{"address", m_lastBleAddress}}},
                     {"hudp", QJsonObject{{"target", m_lastHudpTarget}, {"port", m_lastHudpPort}}}};
    QSaveFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) || file.write(QJsonDocument(QJsonObject{{"version", 1}, {"settings", settings}, {"keySettings", key}, {"lastConnection", last}}).toJson(QJsonDocument::Indented)) < 0 || !file.commit()) {
        setStatus("Config save failed: " + file.errorString());
        return;
    }
    if (m_keySettings) m_keySettings->saveToSettings();
    setStatus("Config saved: " + m_configFilePath);
}

// Import settings from a JSON file; merges into current state and saves.
// 从 JSON 文件导入设置；合并到当前状态并保存。
bool SettingsManager::importFromFile(const QString &filePath)
{
    const QString path = filePath.trimmed();
    if (path.isEmpty()) { setStatus("Import failed: empty path"); return false; }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { setStatus("Import failed: " + file.errorString()); return false; }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) { setStatus("Import failed: invalid json"); return false; }
    readJson(doc.toJson(QJsonDocument::Compact), true);
    applyAutoStart(m_autoStart);
    save();
    setStatus("Imported config: " + path);
    return true;
}

// Reset all settings to factory defaults and save to disk.
// 将所有设置重置为出厂默认值并保存到磁盘。
void SettingsManager::resetToDefaults()
{
    applyAutoStart(false);
    m_autoStart = false; m_rememberLastConnection = false; m_uiLanguage = "zh-CN"; m_darkMode = 0;
    m_lastConnectionTransport.clear(); m_lastConnectionConnected = false;
    m_lastUartPort.clear(); m_lastUartBaudrate = 115200;
    m_lastBleAddress.clear(); m_lastHudpTarget.clear(); m_lastHudpPort = 45820;
    if (m_keySettings) { m_keySettings->resetToDefaults(); m_keySettings->saveToSettings(); }
    save();
    setStatus("Default config restored");
}

// Apply or remove the OS auto-start registration (Windows/macOS/Linux).
// 应用或移除操作系统自启动注册（Windows/macOS/Linux）。
bool SettingsManager::applyAutoStart(bool enabled)
{
    const QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
#ifdef Q_OS_WIN
    QSettings runKey("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (enabled) runKey.setValue("ESP32S3KeyboardBridge", "\"" + appPath + "\""); else runKey.remove("ESP32S3KeyboardBridge");
    runKey.sync(); return runKey.status() == QSettings::NoError;
#elif defined(Q_OS_MACOS)
    const QString path = QDir::homePath() + "/Library/LaunchAgents/com.esp32s3.keyboardbridge.plist";
    QDir().mkpath(QFileInfo(path).absolutePath());
    if (!enabled) return QFile::remove(path) || !QFile::exists(path);
    QFile file(path); if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file); out << "<?xml version=\"1.0\"?><plist version=\"1.0\"><dict><key>Label</key><string>com.esp32s3.keyboardbridge</string><key>ProgramArguments</key><array><string>" << appPath << "</string></array><key>RunAtLoad</key><true/></dict></plist>\n"; return true;
#elif defined(Q_OS_UNIX)
    const QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart/esp32s3-keyboard-bridge.desktop";
    QDir().mkpath(QFileInfo(path).absolutePath());
    if (!enabled) return QFile::remove(path) || !QFile::exists(path);
    QFile file(path); if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file); out << "[Desktop Entry]\nType=Application\nName=Wireless KM\nExec=" << appPath << "\nTerminal=false\nX-GNOME-Autostart-enabled=true\n"; return true;
#else
    Q_UNUSED(enabled); return false;
#endif
}

// Store the last connection transport type and connected state.
// 存储上次连接的传输类型和连接状态。
void SettingsManager::setLastConnection(const QString &transport, bool connected) { m_lastConnectionTransport = transport; m_lastConnectionConnected = connected; }
// Store the last UART port name and baud rate.
// 存储上次 UART 端口名称和波特率。
void SettingsManager::setLastUart(const QString &port, int baudrate) { m_lastUartPort = port; m_lastUartBaudrate = baudrate; }
// Store the last HUDP target host and port.
// 存储上次 HUDP 目标主机和端口。
void SettingsManager::setLastHudp(const QString &target, int port) { m_lastHudpTarget = target; m_lastHudpPort = port; }
// Set the status message string and emit if changed.
// 设置状态消息字符串，若变更则发出。
void SettingsManager::setStatus(const QString &status) { if (m_status != status) { m_status = status; emit statusChanged(); } }
// Return the default config file path based on platform conventions.
// 根据平台惯例返回默认配置文件路径。
QString SettingsManager::defaultConfigFilePath() const { QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation); if (dir.isEmpty()) dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); if (dir.isEmpty()) dir = QDir::homePath() + "/.config/ESP32S3KeyboardBridge"; return QDir(dir).filePath("settings.json"); }

// Remember a successful connection for future auto-restore.
// 记住成功的连接以便将来自动恢复。
void SettingsManager::rememberConnection(const QString &transport, const QString &endpoint, int port)
{
    if (!m_rememberLastConnection)
        return;
    setLastConnection(transport, true);
    if (transport == "UART")
        setLastUart(endpoint, port);
    else if (transport == "BLE")
        setLastBleAddress(endpoint);
    else if (transport == "HUDP")
        setLastHudp(endpoint, port);
    save();
}

// Mark a connection as disconnected in the saved settings.
// 在保存的设置中将连接标记为断开。
void SettingsManager::markConnectionDisconnected(const QString &transport)
{
    if (!m_rememberLastConnection || (!transport.isEmpty() && m_lastConnectionTransport != transport))
        return;
    setLastConnection(m_lastConnectionTransport, false);
    save();
}

// Attempt to restore the last remembered connection on startup.
// 在启动时尝试恢复上次记住的连接。
void SettingsManager::restoreLastConnection()
{
    if (!m_rememberLastConnection || !m_lastConnectionConnected)
        return;
    if (m_lastConnectionTransport == "UART" && !m_lastUartPort.isEmpty())
        emit restoreUartRequested(m_lastUartPort, m_lastUartBaudrate);
    else if (m_lastConnectionTransport == "BLE" && !m_lastBleAddress.isEmpty())
        emit restoreBleRequested(m_lastBleAddress);
    else if (m_lastConnectionTransport == "HUDP" && !m_lastHudpTarget.isEmpty())
        emit restoreHudpRequested(m_lastHudpTarget, m_lastHudpPort);
}

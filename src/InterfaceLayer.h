#pragma once

#include <QObject>
#include <QHash>
#include <QVariantMap>
#include <QVariantList>
#include <QDateTime>
#include <QByteArray>
#include <functional>
#include <QTextStream>

class SerialManager;
class CaptureManager;
class KeySettingManager;
class BleManager;
class LogManager;
class InputQueue;
class TextInputSource;
class ClipboardTextSource;
class TransportSelector;
class SerialTransport;
class BleTransport;
class HudpTransport;
class SettingsManager;
class AutoClickerManager;
class MouseInputManager;
class InputOutputManager;
class ConnectionStateManager;

class InterfaceLayer : public QObject
{
    Q_OBJECT

    // ========== Home ==========
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)

    // ========== Key settings ==========
    Q_PROPERTY(bool captureEnabled READ captureEnabled NOTIFY captureEnabledChanged)
    Q_PROPERTY(QString captureHotkey READ captureHotkey NOTIFY captureHotkeyChanged)
    Q_PROPERTY(QString capturePasteHotkey READ capturePasteHotkey NOTIFY capturePasteHotkeyChanged)

    // ========== Device links ==========
    Q_PROPERTY(QString hudpTarget READ hudpTarget NOTIFY hudpTargetChanged)
    Q_PROPERTY(int hudpPort READ hudpPort NOTIFY hudpPortChanged)
    Q_PROPERTY(bool hudpConnecting READ hudpConnecting NOTIFY hudpConnectingChanged)
    Q_PROPERTY(QString hudpStatus READ hudpStatus NOTIFY hudpStatusChanged)

    Q_PROPERTY(bool bleScanning READ bleScanning NOTIFY bleScanningChanged)
    Q_PROPERTY(bool bleConnecting READ bleConnecting NOTIFY bleConnectingChanged)
    Q_PROPERTY(bool bleConnected READ bleConnected NOTIFY bleConnectedChanged)
    Q_PROPERTY(QString bleStatus READ bleStatus NOTIFY bleStatusChanged)

    Q_PROPERTY(QString uartPort READ uartPort NOTIFY uartPortChanged)
    Q_PROPERTY(int uartBaudrate READ uartBaudrate NOTIFY uartBaudrateChanged)
    Q_PROPERTY(bool uartConnected READ uartConnected NOTIFY uartConnectedChanged)
    Q_PROPERTY(bool uartConnecting READ uartConnecting NOTIFY uartConnectingChanged)
    Q_PROPERTY(QString uartStatus READ uartStatus NOTIFY uartStatusChanged)

    Q_PROPERTY(QString connectionError READ connectionError NOTIFY connectionErrorChanged)
    Q_PROPERTY(QString pairedDeviceName READ pairedDeviceName NOTIFY pairedDeviceNameChanged)
    Q_PROPERTY(QString pairedDeviceAddress READ pairedDeviceAddress NOTIFY pairedDeviceAddressChanged)

    // ========== Connection status ==========
    Q_PROPERTY(int connectedTime READ connectedTime NOTIFY connectedTimeChanged)
    Q_PROPERTY(QString connectedTimeStr READ connectedTimeStr NOTIFY connectedTimeChanged)

    // ========== Text input ==========
    Q_PROPERTY(QString lastSentText READ lastSentText NOTIFY lastSentTextChanged)
    Q_PROPERTY(bool sending READ sending NOTIFY sendingChanged)
    Q_PROPERTY(bool clipboardWatching READ clipboardWatching NOTIFY clipboardWatchingChanged)
    Q_PROPERTY(bool passthroughListening READ passthroughListening NOTIFY passthroughModeChanged)
    Q_PROPERTY(bool passthroughExclusive READ passthroughExclusive NOTIFY passthroughModeChanged)
    Q_PROPERTY(bool passthroughMouseListening READ passthroughMouseListening NOTIFY passthroughModeChanged)

    // ========== Auto clicker ==========
    Q_PROPERTY(QVariantList autoClickerKeys READ autoClickerKeys NOTIFY autoClickerKeysChanged)
    Q_PROPERTY(int autoClickerInterval READ autoClickerInterval NOTIFY autoClickerIntervalChanged)
    Q_PROPERTY(int autoClickerRepeatCount READ autoClickerRepeatCount NOTIFY autoClickerRepeatCountChanged)
    Q_PROPERTY(int autoClickerCompletedCycles READ autoClickerCompletedCycles NOTIFY autoClickerCompletedCyclesChanged)
    Q_PROPERTY(bool autoClickerRunning READ autoClickerRunning NOTIFY autoClickerRunningChanged)
    Q_PROPERTY(QString autoClickerStatus READ autoClickerStatus NOTIFY autoClickerStatusChanged)

    // ========== Logs ==========
    Q_PROPERTY(int currentLogLevel READ currentLogLevel NOTIFY currentLogLevelChanged)
    Q_PROPERTY(int infoCount READ infoCount NOTIFY infoCountChanged)
    Q_PROPERTY(int warningCount READ warningCount NOTIFY warningCountChanged)
    Q_PROPERTY(int errorLogCount READ errorLogCount NOTIFY errorLogCountChanged)
    Q_PROPERTY(int debugCount READ debugCount NOTIFY debugCountChanged)
    Q_PROPERTY(QVariantList logEntries READ logEntries NOTIFY logEntriesChanged)
    Q_PROPERTY(QString logText READ logText NOTIFY logEntriesChanged)
    Q_PROPERTY(QString selectedLogDetail READ selectedLogDetail NOTIFY selectedLogDetailChanged)
    Q_PROPERTY(int logViewRevision READ logViewRevision NOTIFY logViewRevisionChanged)

    // ========== Settings ==========
    Q_PROPERTY(bool autoStart READ autoStart NOTIFY autoStartChanged)
    Q_PROPERTY(bool rememberLastConnection READ rememberLastConnection NOTIFY rememberLastConnectionChanged)
    Q_PROPERTY(QString configFilePath READ configFilePath NOTIFY configFilePathChanged)
    Q_PROPERTY(QString settingsStatus READ settingsStatus NOTIFY settingsStatusChanged)
    Q_PROPERTY(QString uiLanguage READ uiLanguage NOTIFY uiLanguageChanged)
    Q_PROPERTY(int darkMode READ darkMode NOTIFY darkModeChanged)

    // ========== About ==========
    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString buildDate READ buildDate CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString copyright READ copyright CONSTANT)

public:
    // Constructor: initialize the interface layer with all manager objects and route table
    // 构造函数：初始化接口层，创建所有管理器对象并建立路由表
    explicit InterfaceLayer(bool debugTrace = true, QObject *parent = nullptr);

    // Dispatch a command string to the registered route handler
    // 将命令字符串分发到对应的路由处理函数
    Q_INVOKABLE void dispatch(const QString &cmd, const QVariantMap &params);

    // Get the serial manager as a QObject pointer for QML
    // 获取串口管理器的QObject指针，供QML使用
    Q_INVOKABLE QObject *serialManagerObj();
    // Get the capture manager as a QObject pointer for QML
    // 获取按键捕获管理器的QObject指针，供QML使用
    Q_INVOKABLE QObject *captureManagerObj();
    // Get the key setting manager as a QObject pointer for QML
    // 获取按键设置管理器的QObject指针，供QML使用
    Q_INVOKABLE QObject *keySettingManagerObj();
    // Get the BLE manager as a QObject pointer for QML
    // 获取蓝牙管理器的QObject指针，供QML使用
    Q_INVOKABLE QObject *bleManagerObj();

    // -- Home --
    // Check whether a device connection is currently active
    // 检查当前是否有设备处于连接状态
    bool connected() const;
    // Get the name of the currently connected device
    // 获取当前已连接设备的名称
    QString deviceName() const { return m_deviceName; }

    // -- Key settings --
    // Check whether key capture is currently enabled
    // 检查按键捕获功能是否已启用
    bool captureEnabled() const;
    // Get the current capture hotkey combination string
    // 获取当前捕获热键的组合字符串
    QString captureHotkey() const;
    // Get the current paste hotkey combination string
    // 获取当前粘贴热键的组合字符串
    QString capturePasteHotkey() const;

    // -- Device links --
    // Get the HUDP target address string
    // 获取HUDP目标地址字符串
    QString hudpTarget() const;
    // Get the HUDP target port number
    // 获取HUDP目标端口号
    int hudpPort() const;
    // Check whether a HUDP connection attempt is in progress
    // 检查是否正在进行HUDP连接
    bool hudpConnecting() const;
    // Get the current HUDP connection status description
    // 获取当前HUDP连接状态描述
    QString hudpStatus() const;

    // Check whether a BLE scan is in progress
    // 检查是否正在进行蓝牙扫描
    bool bleScanning() const;
    // Check whether a BLE connection attempt is in progress
    // 检查是否正在进行蓝牙连接
    bool bleConnecting() const;
    // Check whether a BLE device is currently connected
    // 检查是否有蓝牙设备已连接
    bool bleConnected() const;
    // Get the current BLE connection status description
    // 获取当前蓝牙连接状态描述
    QString bleStatus() const;

    // Get the current UART serial port name
    // 获取当前UART串口名称
    QString uartPort() const;
    // Get the current UART baud rate setting
    // 获取当前UART波特率设置
    int uartBaudrate() const;
    // Check whether a UART device is currently connected
    // 检查是否有UART设备已连接
    bool uartConnected() const;
    // Check whether a UART connection attempt is in progress
    // 检查是否正在进行UART连接
    bool uartConnecting() const;
    // Get the current UART connection status description
    // 获取当前UART连接状态描述
    QString uartStatus() const;

    // Get the last connection error message
    // 获取最近一次连接错误信息
    QString connectionError() const;
    // Get the name of the paired/remembered device
    // 获取已配对/已记住设备的名称
    QString pairedDeviceName() const;
    // Get the address of the paired/remembered device
    // 获取已配对/已记住设备的地址
    QString pairedDeviceAddress() const;

    // -- Connection status --
    // Get the elapsed time in seconds since connection was established
    // 获取自连接建立以来的经过时间（秒）
    int connectedTime() const;
    // Get the formatted connection duration string
    // 获取格式化的连接持续时间字符串
    QString connectedTimeStr() const;

    // -- Text input --
    // Get a preview of the last sent text (up to 40 chars)
    // 获取最近发送文本的预览（最多40个字符）
    QString lastSentText() const;
    // Check whether a text send operation is currently in progress
    // 检查是否正在进行文本发送操作
    bool sending() const;
    // Check whether clipboard watching is currently active
    // 检查剪贴板监视功能是否已启用
    bool clipboardWatching() const;
    // Check whether app foreground passthrough key listening is active
    // 检查应用前台按键透传监听是否已启用
    bool passthroughListening() const;
    // Check whether exclusive app passthrough mode is active
    // 检查应用独占透传模式是否已启用
    bool passthroughExclusive() const;
    // Check whether app foreground mouse passthrough listening is active
    // 检查应用前台鼠标透传监听是否已启用
    bool passthroughMouseListening() const;

    // Get the list of auto-clicker keys with usage codes and labels
    // 获取自动点击器的按键列表（包含usage码和标签）
    QVariantList autoClickerKeys() const;
    // Get the auto-clicker interval in milliseconds
    // 获取自动点击器的间隔时间（毫秒）
    int autoClickerInterval() const;
    // Get the auto-clicker repeat count setting
    // 获取自动点击器的重复次数设置
    int autoClickerRepeatCount() const;
    // Get the number of completed auto-clicker cycles
    // 获取已完成的自动点击器循环次数
    int autoClickerCompletedCycles() const;
    // Check whether the auto-clicker is currently running
    // 检查自动点击器是否正在运行
    bool autoClickerRunning() const;
    // Get the auto-clicker status description string
    // 获取自动点击器的状态描述字符串
    QString autoClickerStatus() const;
    // Get the list of all available keys for auto-clicker selection
    // 获取自动点击器可用的所有按键列表
    Q_INVOKABLE QVariantList autoClickerAvailableKeys() const;

    // -- Logs --
    // Get the current active log level filter (0=info, 3=debug)
    // 获取当前激活的日志级别过滤器（0=info, 3=debug）
    int currentLogLevel() const;
    // Get the count of info-level log entries
    // 获取info级别日志条目数量
    int infoCount() const;
    // Get the count of warning-level log entries
    // 获取warning级别日志条目数量
    int warningCount() const;
    // Get the count of error-level log entries
    // 获取error级别日志条目数量
    int errorLogCount() const;
    // Get the count of debug-level log entries
    // 获取debug级别日志条目数量
    int debugCount() const;
    // Get the current log entries list for QML display
    // 获取当前日志条目列表，供QML显示
    QVariantList logEntries() const;
    // Get the full log text as a single string
    // 获取完整日志文本（单个字符串）
    QString logText() const;
    // Get the detail text of the currently selected log entry
    // 获取当前选中日志条目的详细文本
    QString selectedLogDetail() const;
    // Get the log view revision counter (increments on each view rebuild)
    // 获取日志视图版本计数器（每次重建视图时递增）
    int logViewRevision() const;

    // -- Settings --
    // Check whether reconnecting to the most recent device is enabled.
    // 检查是否启用了记住上次连接。
    bool rememberLastConnection() const;
    // Check whether auto-start on system boot is enabled
    // 检查是否启用了系统开机自启动
    bool autoStart() const;
    // Check whether remembering the last connection is enabled
    // 检查是否启用了记住上次连接
    QString configFilePath() const;
    // Get the current settings operation status string
    // 获取当前设置操作状态字符串
    QString settingsStatus() const;
    // Get the current UI language code (e.g. "zh-CN", "en-US")
    // 获取当前界面语言代码（如"zh-CN"、"en-US"）
    QString uiLanguage() const;
    // Get the current dark mode setting (0=light, 1=dark, 2=system)
    // 获取当前深色模式设置（0=浅色, 1=深色, 2=跟随系统）
    int darkMode() const;

    // -- About --
    // Get the application display name
    // 获取应用程序显示名称
    QString appName() const { return "Wireless KM"; }
    // Get the application version string
    // 获取应用程序版本字符串
    QString appVersion() const { return "v1.0.0"; }
    // Get the build date of the current binary
    // 获取当前二进制文件的构建日期
    QString buildDate() const { return __DATE__; }
    // Get the application description text
    // 获取应用程序描述文本
    QString description() const { return "Desktop C++ / Qt Quick client for ESP32-S3 wireless keyboard & mouse input. Supports BLE / UART / UDP transport, real-time HID keyboard & mouse forwarding, remote touchpad, and auto clicker."; }
    // Get the copyright notice string
    // 获取版权声明字符串
    QString copyright() const { return "(C) 2025 MicroDeveci"; }

signals:
    void connectedChanged();
    void deviceNameChanged();

    void captureEnabledChanged();
    void captureHotkeyChanged();
    void capturePasteHotkeyChanged();

    void hudpTargetChanged();
    void hudpPortChanged();
    void hudpConnectingChanged();
    void hudpStatusChanged();

    void bleScanningChanged();
    void bleConnectingChanged();
    void bleConnectedChanged();
    void bleStatusChanged();

    void uartPortChanged();
    void uartBaudrateChanged();
    void uartConnectedChanged();
    void uartConnectingChanged();
    void uartStatusChanged();

    void connectionErrorChanged();
    void pairedDeviceNameChanged();
    void pairedDeviceAddressChanged();

    void connectedTimeChanged();

    void lastSentTextChanged();
    void sendingChanged();
    void clipboardWatchingChanged();
    void passthroughModeChanged();
    void autoClickerKeysChanged();
    void autoClickerIntervalChanged();
    void autoClickerRepeatCountChanged();
    void autoClickerCompletedCyclesChanged();
    void autoClickerRunningChanged();
    void autoClickerStatusChanged();

    void currentLogLevelChanged();
    void infoCountChanged();
    void warningCountChanged();
    void errorLogCountChanged();
    void debugCountChanged();
    void logEntriesChanged();
    void selectedLogDetailChanged();
    void logViewRevisionChanged();

    void autoStartChanged();
    void rememberLastConnectionChanged();
    void configFilePathChanged();
    void settingsStatusChanged();
    void uiLanguageChanged();
    void darkModeChanged();

private:
    // Set up the command routing table and register all route handlers
    // 设置命令路由表并注册所有路由处理函数
    void setupRoutes();
    // Wire up signal/slot connections between all manager objects
    // 建立所有管理器对象之间的信号槽连接
    void setupManagerConnections();
    // Append a log entry with level, module, message, and optional detail
    // 追加一条日志条目，包含级别、模块、消息和可选详情
    void appendLog(int level, const QString &module, const QString &message, const QString &detail = "");
    // Log a debug data-flow trace entry when debug tracing is enabled
    // 当调试跟踪启用时，记录一条数据流跟踪日志
    void traceDataFlow(const QString &source, const QString &message, const QVariantMap &data = {});
    // Rebuild the log view model after data changes
    // 在数据变化后重建日志视图模型
    void rebuildLogView();
    // Clear the current connection error state
    // 清除当前连接错误状态
    void clearConnectionError();
    // Import settings from an external JSON file
    // 从外部JSON文件导入设置
    bool importSettingsFromFile(const QString &filePath);
    // Apply the auto-start on boot setting to the OS
    // 将开机自启动设置应用到操作系统
    bool applyAutoStart(bool enabled);
    // Set the settings operation status message
    // 设置设置操作状态消息
    void setSettingsStatus(const QString &status);
    // Register all key-related command routes
    // 注册所有按键相关的命令路由
    void registerKeyRoutes();
    // Register all HUDP transport command routes
    // 注册所有HUDP传输相关的命令路由
    void registerHudpRoutes();
    // Register all BLE transport command routes
    // 注册所有蓝牙传输相关的命令路由
    void registerBleRoutes();
    // Register all UART transport command routes
    // 注册所有UART传输相关的命令路由
    void registerUartRoutes();
    // Register all text input command routes
    // 注册所有文本输入相关的命令路由
    void registerTextRoutes();
    // Register all clipboard-related command routes
    // 注册所有剪贴板相关的命令路由
    void registerClipboardRoutes();
    // Register all auto-clicker command routes
    // 注册所有自动点击器相关的命令路由
    void registerAutoClickerRoutes();
    // Register all mouse input command routes
    // 注册所有鼠标输入相关的命令路由
    void registerMouseRoutes();
    // Register all passthrough (app foreground capture) command routes
    // 注册所有透传（应用前台捕获）相关的命令路由
    void registerPassthroughRoutes();
    // Register all log management command routes
    // 注册所有日志管理相关的命令路由
    void registerLogRoutes();
    // Register all application-level command routes
    // 注册所有应用层面的命令路由
    void registerAppRoutes();
    // Register all settings management command routes
    // 注册所有设置管理相关的命令路由
    void registerSettingsRoutes();
    // Load settings from persistent storage
    // 从持久化存储加载设置
    void loadSettings();
    // Save current settings to persistent storage
    // 将当前设置保存到持久化存储
    void saveSettings();

    using RouteFn = std::function<void(const QVariantMap &)>;
    QHash<QString, RouteFn> m_routes;

    // Managers
    SerialManager       *m_serialManager = nullptr;
    CaptureManager      *m_captureManager = nullptr;
    KeySettingManager   *m_keySettingManager = nullptr;
    BleManager          *m_bleManager = nullptr;
    LogManager          *m_logManager = nullptr;
    InputQueue          *m_inputQueue = nullptr;
    TextInputSource     *m_textInputSource = nullptr;
    ClipboardTextSource *m_clipboardTextSource = nullptr;
    TransportSelector   *m_transportSelector = nullptr;
    SerialTransport     *m_serialTransport = nullptr;
    BleTransport        *m_bleTransport = nullptr;
    HudpTransport      *m_hudpTransport = nullptr;
    SettingsManager     *m_settingsManager = nullptr;
    AutoClickerManager  *m_autoClickerManager = nullptr;
    MouseInputManager   *m_mouseInputManager = nullptr;
    InputOutputManager  *m_inputOutputManager = nullptr;
    ConnectionStateManager *m_connectionStateManager = nullptr;

    QString m_deviceName;

    // Logs
    QString m_logFilePath;
    QString m_lastSentText;
    bool    m_debugTrace = true;

};

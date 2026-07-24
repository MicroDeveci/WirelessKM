#include "InterfaceLayer.h"
#include "SerialManager.h"
#include "CaptureManager.h"
#include "KeySettingManager.h"
#include "BleManager.h"
#include "LogManager.h"
#include "KeyboardProtocol.h"
#include "InputQueue.h"
#include "InputOutputManager.h"
#include "AutoClickerManager.h"
#include "MouseInputManager.h"
#include "MouseProtocol.h"
#include "TextInputSource.h"
#include "ClipboardTextSource.h"
#include "TransportSelector.h"
#include "SerialTransport.h"
#include "BleTransport.h"
#include "HudpTransport.h"
#include "DebugTrace.h"
#include "SettingsManager.h"
#include "ConnectionStateManager.h"
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>
#include <QStringList>
#include <QKeySequence>

namespace {
constexpr quint16 AutoClickMouseLeft = 0xff01;
constexpr quint16 AutoClickMouseRight = 0xff02;
constexpr quint16 AutoClickMouseMiddle = 0xff04;

// Convert an auto-clicker key usage code to a human-readable label
// 将自动点击器的按键usage码转换为可读标签
QString autoClickerKeyLabel(quint16 usage)
{
    switch (usage) {
    case AutoClickMouseLeft: return QStringLiteral("Mouse Left");
    case AutoClickMouseRight: return QStringLiteral("Mouse Right");
    case AutoClickMouseMiddle: return QStringLiteral("Mouse Middle");
    }
    for (const KeyDefinition &definition : KeyboardProtocol::keyDefinitions()) {
        if (definition.usage == usage)
            return definition.normal.isNull() ? definition.id : QString(definition.normal).toUpper();
    }
    return QString("0x%1").arg(usage, 4, 16, QChar('0'));
}
}

#ifdef Q_OS_UNIX
#include <QTextStream>
#endif

// Construct the InterfaceLayer, creating all managers, transports, and initializing the route table
// 构造InterfaceLayer，创建所有管理器、传输层，并初始化路由表
InterfaceLayer::InterfaceLayer(bool debugTrace, QObject *parent)
    : QObject(parent)
    , m_serialManager(new SerialManager(this))
    , m_debugTrace(debugTrace)
{
    DebugTrace::setEnabled(debugTrace);
    m_bleManager = new BleManager(this);
    m_captureManager = new CaptureManager(this);
    m_keySettingManager = new KeySettingManager(this);
    m_settingsManager = new SettingsManager(m_keySettingManager, this);
    connect(m_settingsManager, &SettingsManager::statusChanged, this, [this]() {
        emit settingsStatusChanged();
        traceDataFlow("InterfaceLayer::settings", "status changed", {{"status", m_settingsManager->status()}});
    });
    connect(m_settingsManager, &SettingsManager::configFilePathChanged, this, &InterfaceLayer::configFilePathChanged);
    m_logManager = new LogManager(this);
    m_logManager->setDebugTrace(debugTrace);
    m_inputQueue = new InputQueue(this);
    m_mouseInputManager = new MouseInputManager(this);
    m_textInputSource = new TextInputSource(m_inputQueue, m_transportSelector, this);
    m_clipboardTextSource = new ClipboardTextSource(this);
    m_transportSelector = new TransportSelector(this);
    m_inputOutputManager = new InputOutputManager(m_inputQueue, m_mouseInputManager, m_transportSelector, this);
    m_autoClickerManager = new AutoClickerManager(m_inputQueue, m_mouseInputManager, m_transportSelector, this);
    m_serialTransport = new SerialTransport(m_serialManager, this);
    m_bleTransport = new BleTransport(m_bleManager, this);
    m_hudpTransport = new HudpTransport(this);
    m_transportSelector->setTransports(m_bleTransport, m_serialTransport, m_hudpTransport);
    m_inputQueue->setActiveTransport(m_transportSelector->activeTransport());
    m_connectionStateManager = new ConnectionStateManager(
        m_serialManager, m_bleManager, m_hudpTransport, m_settingsManager,
        m_captureManager, m_mouseInputManager, m_autoClickerManager, m_transportSelector, this);

    if (m_debugTrace) qDebug() << "[InterfaceLayer] Initializing, debugTrace=" << m_debugTrace;

    const QString logPath = m_settingsManager->logFilePath();
    QDir().mkpath(QFileInfo(logPath).absolutePath());
    m_logFilePath = logPath;
    m_logManager->setLogFilePath(m_logFilePath);
    loadSettings();
    setupManagerConnections();

    setupRoutes();
    QTimer::singleShot(300, m_settingsManager, &SettingsManager::restoreLastConnection);
    traceDataFlow("InterfaceLayer::InterfaceLayer", "initialized", {{"debugTrace", m_debugTrace}});
    if (m_debugTrace) qDebug() << "[InterfaceLayer] Initialization complete.";
}

// Get the serial manager as a QObject pointer for QML access
// 获取串口管理器的QObject指针，供QML访问
QObject *InterfaceLayer::serialManagerObj() { return m_serialManager; }
// Get the capture manager as a QObject pointer for QML access
// 获取按键捕获管理器的QObject指针，供QML访问
QObject *InterfaceLayer::captureManagerObj() { return m_captureManager; }
// Get the key setting manager as a QObject pointer for QML access
// 获取按键设置管理器的QObject指针，供QML访问
QObject *InterfaceLayer::keySettingManagerObj() { return m_keySettingManager; }
// Get the BLE manager as a QObject pointer for QML access
// 获取蓝牙管理器的QObject指针，供QML访问
QObject *InterfaceLayer::bleManagerObj() { return m_bleManager; }

// Check whether a device connection is currently active
// 检查当前是否有设备处于连接状态
bool InterfaceLayer::connected() const { return m_connectionStateManager->connected(); }
// Get the HUDP target address
// 获取HUDP目标地址
QString InterfaceLayer::hudpTarget() const { return m_connectionStateManager->hudpTarget(); }
// Get the HUDP target port number
// 获取HUDP目标端口号
int InterfaceLayer::hudpPort() const { return m_connectionStateManager->hudpPort(); }
// Check whether a HUDP connection attempt is in progress
// 检查是否正在进行HUDP连接
bool InterfaceLayer::hudpConnecting() const { return m_connectionStateManager->hudpConnecting(); }
// Get the current HUDP connection status description
// 获取当前HUDP连接状态描述
QString InterfaceLayer::hudpStatus() const { return m_connectionStateManager->hudpStatus(); }
// Check whether a BLE scan is in progress
// 检查是否正在进行蓝牙扫描
bool InterfaceLayer::bleScanning() const { return m_connectionStateManager->bleScanning(); }
// Check whether a BLE connection attempt is in progress
// 检查是否正在进行蓝牙连接
bool InterfaceLayer::bleConnecting() const { return m_connectionStateManager->bleConnecting(); }
// Check whether a BLE device is currently connected
// 检查是否有蓝牙设备已连接
bool InterfaceLayer::bleConnected() const { return m_connectionStateManager->bleConnected(); }
// Get the current BLE connection status description
// 获取当前蓝牙连接状态描述
QString InterfaceLayer::bleStatus() const { return m_connectionStateManager->bleStatus(); }
// Get the current UART serial port name
// 获取当前UART串口名称
QString InterfaceLayer::uartPort() const { return m_connectionStateManager->uartPort(); }
// Get the current UART baud rate setting
// 获取当前UART波特率设置
int InterfaceLayer::uartBaudrate() const { return m_connectionStateManager->uartBaudrate(); }
// Check whether a UART device is currently connected
// 检查是否有UART设备已连接
bool InterfaceLayer::uartConnected() const { return m_connectionStateManager->uartConnected(); }
// Check whether a UART connection attempt is in progress
// 检查是否正在进行UART连接
bool InterfaceLayer::uartConnecting() const { return m_connectionStateManager->uartConnected(); }
// Get the current UART connection status description
// 获取当前UART连接状态描述
QString InterfaceLayer::uartStatus() const { return m_connectionStateManager->uartStatus(); }
// Get the last connection error message
// 获取最近一次连接错误信息
QString InterfaceLayer::connectionError() const { return m_connectionStateManager->connectionError(); }
// Get the name of the paired/remembered device
// 获取已配对/已记住设备的名称
QString InterfaceLayer::pairedDeviceName() const { return m_connectionStateManager->pairedDeviceName(); }
// Get the address of the paired/remembered device
// 获取已配对/已记住设备的地址
QString InterfaceLayer::pairedDeviceAddress() const { return m_connectionStateManager->pairedDeviceAddress(); }

// Check whether auto-start on system boot is enabled
// 检查是否启用了系统开机自启动
bool InterfaceLayer::autoStart() const { return m_settingsManager && m_settingsManager->autoStart(); }
// Check whether remembering the last connection is enabled
// 检查是否启用了记住上次连接
bool InterfaceLayer::rememberLastConnection() const { return m_settingsManager && m_settingsManager->rememberLastConnection(); }
// Get the config file path, or empty if settings manager is unavailable
// 获取配置文件路径，若设置管理器不可用则返回空字符串
QString InterfaceLayer::configFilePath() const { return m_settingsManager ? m_settingsManager->configFilePath() : QString(); }
// Get the current settings operation status string
// 获取当前设置操作状态字符串
QString InterfaceLayer::settingsStatus() const { return m_settingsManager ? m_settingsManager->status() : QString(); }
// Get the current UI language code, defaulting to zh-CN
// 获取当前界面语言代码，默认为zh-CN
QString InterfaceLayer::uiLanguage() const { return m_settingsManager ? m_settingsManager->uiLanguage() : QStringLiteral("zh-CN"); }
// Get the current dark mode setting (0=light, 1=dark, 2=system)
// 获取当前深色模式设置（0=浅色, 1=深色, 2=跟随系统）
int InterfaceLayer::darkMode() const { return m_settingsManager ? m_settingsManager->darkMode() : 0; }

// Dispatch a command string to its registered route handler
// 将命令字符串分发到对应的路由处理函数
void InterfaceLayer::dispatch(const QString &cmd, const QVariantMap &params)
{
    auto it = m_routes.find(cmd);
    if (it != m_routes.end()) {
        traceDataFlow("InterfaceLayer::dispatch", "route received", {{"cmd", cmd}, {"params", params}});
        it.value()(params);
    } else {
        qWarning() << "[InterfaceLayer] Unknown command:" << cmd;
    }
}

// ============================================================
// Manager signal wiring
// ============================================================
// Wire up signal/slot connections between all managers, transports, and the interface layer
// 建立所有管理器、传输层与接口层之间的信号槽连接
void InterfaceLayer::setupManagerConnections()
{
    connect(m_logManager, &LogManager::currentLogLevelChanged, this, &InterfaceLayer::currentLogLevelChanged);
    connect(m_logManager, &LogManager::infoCountChanged, this, &InterfaceLayer::infoCountChanged);
    connect(m_logManager, &LogManager::warningCountChanged, this, &InterfaceLayer::warningCountChanged);
    connect(m_logManager, &LogManager::errorLogCountChanged, this, &InterfaceLayer::errorLogCountChanged);
    connect(m_logManager, &LogManager::debugCountChanged, this, &InterfaceLayer::debugCountChanged);
    connect(m_logManager, &LogManager::logEntriesChanged, this, &InterfaceLayer::logEntriesChanged);
    connect(m_logManager, &LogManager::selectedLogDetailChanged, this, &InterfaceLayer::selectedLogDetailChanged);
    connect(m_logManager, &LogManager::logViewRevisionChanged, this, &InterfaceLayer::logViewRevisionChanged);
    connect(m_transportSelector, &TransportSelector::activeTransportChanged, this, [this]() {
        m_inputQueue->setActiveTransport(m_transportSelector->activeTransport());
        traceDataFlow("TransportSelector::activeTransportChanged", "active transport selected",
                      {{"transport", m_transportSelector->activeTransportName()}});
    });
    connect(m_inputQueue, &InputQueue::sendingChanged, this, [this]() {
        traceDataFlow("InputQueue::sendingChanged", "queue sending state mirrored", {{"sending", sending()}});
        emit sendingChanged();
    });
    connect(m_inputQueue, &InputQueue::packetWritten, this, [this](const QString &transport, const QString &hex, const QString &label) {
        traceDataFlow("InputQueue::packetWritten", "packet written to active transport",
                      {{"transport", transport}, {"hex", hex}, {"label", label}});
        appendLog(3, transport + " TX", hex + " | " + label);
    });
    connect(m_inputQueue, &InputQueue::errorOccurred, this, [this](const QString &err) {
        m_connectionStateManager->handleOutputError(err);
        traceDataFlow("InputQueue::errorOccurred", "queue error received", {{"error", err}});
        appendLog(2, "InputQueue", err);
    });
    connect(m_autoClickerManager, &AutoClickerManager::keysChanged, this, &InterfaceLayer::autoClickerKeysChanged);
    connect(m_autoClickerManager, &AutoClickerManager::intervalChanged, this, &InterfaceLayer::autoClickerIntervalChanged);
    connect(m_autoClickerManager, &AutoClickerManager::repeatCountChanged, this, &InterfaceLayer::autoClickerRepeatCountChanged);
    connect(m_autoClickerManager, &AutoClickerManager::completedCyclesChanged, this, &InterfaceLayer::autoClickerCompletedCyclesChanged);
    connect(m_autoClickerManager, &AutoClickerManager::runningChanged, this, &InterfaceLayer::autoClickerRunningChanged);
    connect(m_autoClickerManager, &AutoClickerManager::statusChanged, this, &InterfaceLayer::autoClickerStatusChanged);
    connect(m_autoClickerManager, &AutoClickerManager::sendRejected, this, [this]() {
        m_connectionStateManager->setConnectionError("No active transport");
        appendLog(2, "AutoClicker", "Stopped: no active transport");
    });
    connect(m_autoClickerManager, &AutoClickerManager::cycleEnqueued, this, [this](int keys, int cycle) {
        traceDataFlow("AutoClicker::cycle", "input cycle enqueued",
                      {{"keys", keys}, {"cycles", cycle}});
    });
    connect(m_inputOutputManager, &InputOutputManager::outputRejected, this,
            [this](const QString &source, const QString &detail) {
        m_connectionStateManager->setConnectionError(detail);
        appendLog(2, source, "Send failed: " + detail);
    });
    connect(m_inputOutputManager, &InputOutputManager::mouseQueued, this,
            [this](quint8 buttons, qint8 dx, qint8 dy, qint8 wheel, const QString &transport) {
        clearConnectionError();
        traceDataFlow("InputOutputManager::mouseQueued", "mouse report enqueued",
                      {{"buttons", buttons}, {"dx", static_cast<int>(dx)}, {"dy", static_cast<int>(dy)},
                       {"wheel", static_cast<int>(wheel)}, {"transport", transport}});
    });
    connect(m_inputOutputManager, &InputOutputManager::keyUnmapped, this, [this](const QString &key, const QString &text) {
        appendLog(1, "Passthrough", "Unmapped key: " + key, "text=" + text);
    });
    connect(m_inputOutputManager, &InputOutputManager::capturedKeyQueued, this,
            [this](const QString &key, quint16 usage, bool pressed, const QString &transport) {
        clearConnectionError();
        traceDataFlow("InputOutputManager::capturedKeyQueued", "captured key enqueued",
                      {{"key", key}, {"usage", usage}, {"pressed", pressed}, {"transport", transport}});
    });
    connect(m_textInputSource, &TextInputSource::queueSizeChanged, this, [this]() {
        traceDataFlow("TextInputSource::queueSizeChanged", "text source queue changed",
                      {{"queueSize", m_textInputSource->queueSize()}});
    });
    connect(m_textInputSource, &TextInputSource::mappingChecked, this,
            [this](const TextInputRequest &request, int mappedChars, int totalChars, const QStringList &unmapped) {
        traceDataFlow("TextInputSource::mappingChecked", "text mapping checked",
                      {{"source", request.source}, {"totalChars", totalChars}, {"mappedChars", mappedChars}, {"unmapped", unmapped}});
        if (!unmapped.isEmpty())
            appendLog(1, "KeyboardProtocol", "Text contains unmapped characters: " + unmapped.join(", "),
                      QString("source=%1 mapped=%2 total=%3").arg(request.source).arg(mappedChars).arg(totalChars));
    });
    connect(m_textInputSource, &TextInputSource::sendRejected, this, [this](const QString &source, int length) {
        m_connectionStateManager->setConnectionError("No active transport");
        traceDataFlow("TextInputSource::sendRejected", "send rejected: no active transport",
                      {{"source", source}, {"length", length}});
        appendLog(2, "TextInput", "Send failed: no active transport");
    });
    connect(m_textInputSource, &TextInputSource::textQueued, this,
            [this](const QString &text, const QString &source, const QString &transport) {
        clearConnectionError();
        m_lastSentText = text.left(40);
        emit lastSentTextChanged();
        appendLog(0, "TextInput", "Queued send via " + transport + ": " + QString::number(text.length()) + " chars");
        traceDataFlow("TextInputSource::textQueued", "text encoded and enqueued",
                      {{"source", source}, {"length", text.length()}, {"transport", transport}});
    });
    connect(m_clipboardTextSource, &ClipboardTextSource::textProduced, this, [this](const QString &text, const QString &source) {
        traceDataFlow("ClipboardTextSource::textProduced", "clipboard text produced",
                      {{"source", source}, {"length", text.length()}, {"preview", text.left(40)}});
        m_textInputSource->enqueueText(text, source);
        appendLog(0, "Clipboard", "Clipboard text queued: " + QString::number(text.length()) + " chars");
    });
    connect(m_clipboardTextSource, &ClipboardTextSource::emptyText, this, [this]() {
        traceDataFlow("ClipboardTextSource::emptyText", "clipboard has no text");
        appendLog(1, "Clipboard", "Clipboard has no text");
    });
    connect(m_clipboardTextSource, &ClipboardTextSource::errorOccurred, this, [this](const QString &err) {
        traceDataFlow("ClipboardTextSource::errorOccurred", "clipboard read failed", {{"error", err}});
        appendLog(2, "Clipboard", err);
    });
    connect(m_clipboardTextSource, &ClipboardTextSource::watchingChanged, this, [this]() {
        traceDataFlow("ClipboardTextSource::watchingChanged", "watching state mirrored",
                      {{"watching", clipboardWatching()}});
        emit clipboardWatchingChanged();
    });
    // Connection state is owned by ConnectionStateManager; the facade forwards its notifications to QML.
    connect(m_connectionStateManager, &ConnectionStateManager::connectedChanged, this, &InterfaceLayer::connectedChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::hudpTargetChanged, this, &InterfaceLayer::hudpTargetChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::hudpPortChanged, this, &InterfaceLayer::hudpPortChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::hudpConnectingChanged, this, &InterfaceLayer::hudpConnectingChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::hudpStatusChanged, this, &InterfaceLayer::hudpStatusChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::bleScanningChanged, this, &InterfaceLayer::bleScanningChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::bleConnectingChanged, this, &InterfaceLayer::bleConnectingChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::bleConnectedChanged, this, &InterfaceLayer::bleConnectedChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::bleStatusChanged, this, &InterfaceLayer::bleStatusChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::uartPortChanged, this, &InterfaceLayer::uartPortChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::uartBaudrateChanged, this, &InterfaceLayer::uartBaudrateChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::uartConnectedChanged, this, &InterfaceLayer::uartConnectedChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::uartConnectingChanged, this, &InterfaceLayer::uartConnectingChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::uartStatusChanged, this, &InterfaceLayer::uartStatusChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::connectionErrorChanged, this, &InterfaceLayer::connectionErrorChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::pairedDeviceNameChanged, this, &InterfaceLayer::pairedDeviceNameChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::pairedDeviceAddressChanged, this, &InterfaceLayer::pairedDeviceAddressChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::connectedTimeChanged, this, &InterfaceLayer::connectedTimeChanged);
    connect(m_connectionStateManager, &ConnectionStateManager::diagnostic, this,
            [this](int level, const QString &module, const QString &message) { appendLog(level, module, message); });    // KeySettingManager -> InterfaceLayer mirror
    connect(m_keySettingManager, &KeySettingManager::captureEnabledChanged, this, [this]() {
        traceDataFlow("KeySettingManager::captureEnabledChanged", "value mirrored", {{"enabled", captureEnabled()}});
        emit captureEnabledChanged();
    });
    connect(m_keySettingManager, &KeySettingManager::captureHotkeyChanged, this, [this]() {
        traceDataFlow("KeySettingManager::captureHotkeyChanged", "value mirrored", {{"hotkey", captureHotkey()}});
        emit captureHotkeyChanged();
    });
    connect(m_keySettingManager, &KeySettingManager::capturePasteHotkeyChanged, this, [this]() {
        traceDataFlow("KeySettingManager::capturePasteHotkeyChanged", "value mirrored", {{"hotkey", capturePasteHotkey()}});
        emit capturePasteHotkeyChanged();
    });

    // App foreground passthrough capture.
    connect(m_captureManager, &CaptureManager::appCaptureModeChanged, this, [this]() {
        traceDataFlow("CaptureManager::appCaptureModeChanged", "passthrough mode mirrored",
                      {{"listening", passthroughListening()},
                       {"exclusive", passthroughExclusive()},
                       {"mouseListening", passthroughMouseListening()}});
        emit passthroughModeChanged();
    });
    connect(m_captureManager, &CaptureManager::appMouseCaptured, this,
            [this](int buttons, int dx, int dy) {
        traceDataFlow("CaptureManager::appMouseCaptured", "app foreground mouse captured",
                      {{"buttons", buttons}, {"dx", dx}, {"dy", dy}});
        m_mouseInputManager->setButtons(static_cast<quint8>(buttons));
        m_mouseInputManager->moveBy(dx, dy);
    });
    connect(m_captureManager, &CaptureManager::appMouseWheelCaptured, this,
            [this](int delta) {
        traceDataFlow("CaptureManager::appMouseWheelCaptured", "app foreground mouse wheel captured",
                      {{"delta", delta}});
        m_mouseInputManager->wheelBy(delta);
    });
    connect(m_captureManager, &CaptureManager::appKeyCaptured, this,
            [this](const QString &key, const QString &text, bool pressed, bool autoRepeat, int modifiers) {
        traceDataFlow("CaptureManager::appKeyCaptured", "app foreground key captured",
                      {{"key", key},
                       {"text", text},
                       {"pressed", pressed},
                       {"autoRepeat", autoRepeat},
                        {"modifiers", modifiers},
                        {"exclusive", passthroughExclusive()}});
        m_inputOutputManager->handleCapturedKey(key, text, pressed, autoRepeat, modifiers);
    });
    connect(m_captureManager, &CaptureManager::hotkeyTriggered, this, [this]() {
        traceDataFlow("CaptureManager::hotkeyTriggered", "capture placeholder triggered");
        appendLog(0, "Capture", "Capture placeholder triggered");
    });
}

// Append a log entry to the log manager with level, module, message, and optional detail
// 向日志管理器追加一条日志条目，包含级别、模块、消息和可选详情
void InterfaceLayer::appendLog(int level, const QString &module, const QString &message, const QString &detail)
{
    if (m_logManager)
        m_logManager->append(level, module, message, detail);
}

// Log a debug data-flow trace entry with optional JSON-serialized data
// 记录一条调试数据流跟踪日志，可附加JSON序列化数据
void InterfaceLayer::traceDataFlow(const QString &source, const QString &message, const QVariantMap &data)
{
    if (!m_debugTrace)
        return;

    const QString detail = data.isEmpty()
        ? QString()
        : QString::fromUtf8(QJsonDocument::fromVariant(data).toJson(QJsonDocument::Compact));
    appendLog(3, source, message, detail);
    if (m_debugTrace) qDebug() << "[DATAFLOW]" << source << message << data;
}

// Rebuild the log view model to reflect current log data
// 重建日志视图模型以反映当前日志数据
void InterfaceLayer::rebuildLogView()
{
    if (m_logManager)
        m_logManager->rebuildView();
}

// Load settings from persistent storage and emit change signals
// 从持久化存储加载设置并发出变更信号
void InterfaceLayer::loadSettings()
{
    m_settingsManager->load();
    emit autoStartChanged();
    emit rememberLastConnectionChanged();
    emit darkModeChanged();
}

// Save current settings to persistent storage
// 将当前设置保存到持久化存储
void InterfaceLayer::saveSettings()
{
    m_settingsManager->save();
    if (m_settingsManager->status().startsWith("Config save failed"))
        appendLog(2, "Settings", m_settingsManager->status());
    else
        appendLog(0, "Settings", "Settings saved", m_settingsManager->configFilePath());
}

// Import settings from an external JSON config file
// 从外部JSON配置文件导入设置
bool InterfaceLayer::importSettingsFromFile(const QString &filePath)
{
    const bool imported = m_settingsManager->importFromFile(filePath);
    if (imported) {
        emit autoStartChanged();
        emit rememberLastConnectionChanged();
        appendLog(0, "Settings", "Config imported", filePath.trimmed());
    } else {
        appendLog(2, "Settings", m_settingsManager->status(), filePath.trimmed());
    }
    return imported;
}

// Apply the auto-start on boot setting to the operating system
// 将开机自启动设置应用到操作系统
bool InterfaceLayer::applyAutoStart(bool enabled) { return m_settingsManager->applyAutoStart(enabled); }
// Set the settings operation status message string
// 设置设置操作状态消息字符串
void InterfaceLayer::setSettingsStatus(const QString &status) { m_settingsManager->setStatus(status); }


// ============================================================
// Mirror getters from managers
// ============================================================
// Check whether key capture is currently enabled
// 检查按键捕获功能是否已启用
bool InterfaceLayer::captureEnabled() const { return m_keySettingManager->captureEnabled(); }
// Get the current capture hotkey combination string
// 获取当前捕获热键的组合字符串
QString InterfaceLayer::captureHotkey() const { return m_keySettingManager->captureHotkey(); }
// Get the current paste hotkey combination string
// 获取当前粘贴热键的组合字符串
QString InterfaceLayer::capturePasteHotkey() const { return m_keySettingManager->capturePasteHotkey(); }

// Get the elapsed time in seconds since connection was established
// 获取自连接建立以来的经过时间（秒）
int InterfaceLayer::connectedTime() const { return m_connectionStateManager->connectedTime(); }
// Get the formatted connection duration string
// 获取格式化的连接持续时间字符串
QString InterfaceLayer::connectedTimeStr() const { return m_connectionStateManager->connectedTimeStr(); }

// Get a preview of the last sent text (up to 40 characters)
// 获取最近发送文本的预览（最多40个字符）
QString InterfaceLayer::lastSentText() const { return m_lastSentText; }
// Check whether a text send operation is currently in progress
// 检查是否正在进行文本发送操作
bool InterfaceLayer::sending() const
{
    return (m_bleManager && m_bleManager->isSending())
        || (m_inputQueue && m_inputQueue->sending());
}

// Check whether clipboard watching is currently active
// 检查剪贴板监视功能是否已启用
bool InterfaceLayer::clipboardWatching() const
{
    return m_clipboardTextSource && m_clipboardTextSource->watching();
}

// Check whether app foreground passthrough key listening is active
// 检查应用前台按键透传监听是否已启用
bool InterfaceLayer::passthroughListening() const
{
    return m_captureManager && m_captureManager->appCaptureListening();
}

// Check whether exclusive app passthrough mode is active
// 检查应用独占透传模式是否已启用
bool InterfaceLayer::passthroughExclusive() const
{
    return m_captureManager && m_captureManager->appCaptureExclusive();
}

// Check whether app foreground mouse passthrough listening is active
// 检查应用前台鼠标透传监听是否已启用
bool InterfaceLayer::passthroughMouseListening() const
{
    return m_captureManager && m_captureManager->mouseCaptureListening();
}

// Get the list of auto-clicker keys with usage codes and human-readable labels
// 获取自动点击器的按键列表（包含usage码和可读标签）
QVariantList InterfaceLayer::autoClickerKeys() const
{
    QVariantList result;
    if (!m_autoClickerManager)
        return result;
    for (quint16 usage : m_autoClickerManager->keys())
        result.append(QVariantMap{{"usage", usage}, {"label", autoClickerKeyLabel(usage)}});
    return result;
}

// Get the auto-clicker interval in milliseconds
// 获取自动点击器的间隔时间（毫秒）
int InterfaceLayer::autoClickerInterval() const
{
    return m_autoClickerManager ? m_autoClickerManager->intervalMs() : 100;
}

// Get the auto-clicker repeat count setting
// 获取自动点击器的重复次数设置
int InterfaceLayer::autoClickerRepeatCount() const
{
    return m_autoClickerManager ? m_autoClickerManager->repeatCount() : 0;
}

// Get the number of completed auto-clicker cycles
// 获取已完成的自动点击器循环次数
int InterfaceLayer::autoClickerCompletedCycles() const
{
    return m_autoClickerManager ? m_autoClickerManager->completedCycles() : 0;
}

// Check whether the auto-clicker is currently running
// 检查自动点击器是否正在运行
bool InterfaceLayer::autoClickerRunning() const
{
    return m_autoClickerManager && m_autoClickerManager->running();
}

// Get the auto-clicker status description string
// 获取自动点击器的状态描述字符串
QString InterfaceLayer::autoClickerStatus() const
{
    return m_autoClickerManager ? m_autoClickerManager->status() : QString();
}

// Get the list of all available keys for auto-clicker selection
// 获取自动点击器可用的所有按键列表
QVariantList InterfaceLayer::autoClickerAvailableKeys() const
{
    QVariantList result;
    for (const KeyDefinition &definition : KeyboardProtocol::keyDefinitions()) {
        result.append(QVariantMap{{"usage", definition.usage},
                                  {"label", autoClickerKeyLabel(definition.usage)}});
    }
    return result;
}

// Get the current active log level filter
// 获取当前激活的日志级别过滤器
int InterfaceLayer::currentLogLevel() const { return m_logManager ? m_logManager->currentLogLevel() : 0; }
// Get the count of info-level log entries
// 获取info级别日志条目数量
int InterfaceLayer::infoCount() const { return m_logManager ? m_logManager->infoCount() : 0; }
// Get the count of warning-level log entries
// 获取warning级别日志条目数量
int InterfaceLayer::warningCount() const { return m_logManager ? m_logManager->warningCount() : 0; }
// Get the count of error-level log entries
// 获取error级别日志条目数量
int InterfaceLayer::errorLogCount() const { return m_logManager ? m_logManager->errorLogCount() : 0; }
// Get the count of debug-level log entries
// 获取debug级别日志条目数量
int InterfaceLayer::debugCount() const { return m_logManager ? m_logManager->debugCount() : 0; }
// Get the current log entries list for QML display
// 获取当前日志条目列表，供QML显示
QVariantList InterfaceLayer::logEntries() const { return m_logManager ? m_logManager->logEntries() : QVariantList(); }
// Get the detail text of the currently selected log entry
// 获取当前选中日志条目的详细文本
QString InterfaceLayer::selectedLogDetail() const { return m_logManager ? m_logManager->selectedLogDetail() : QString(); }
// Get the log view revision counter (increments on each rebuild)
// 获取日志视图版本计数器（每次重建时递增）
int InterfaceLayer::logViewRevision() const { return m_logManager ? m_logManager->logViewRevision() : 0; }

// Get the full log text as a single concatenated string
// 获取完整日志文本（单个拼接字符串）
QString InterfaceLayer::logText() const
{
    return m_logManager ? m_logManager->logText() : QString();
}

// Clear the current connection error state
// 清除当前连接错误状态
void InterfaceLayer::clearConnectionError()
{
    m_connectionStateManager->clearConnectionError();
}


// ============================================================
// Command routing table
// ============================================================
// Set up the command routing table, register all route handlers, and log startup info
// 设置命令路由表，注册所有路由处理函数，并记录启动信息
void InterfaceLayer::setupRoutes()
{
    m_routes["home.navigateTo"] = [this](const QVariantMap &p) {
        traceDataFlow("route home.navigateTo", "navigation requested", p);
        appendLog(0, "UI", "Navigate to " + p["route"].toString());
    };

    registerKeyRoutes();
    registerHudpRoutes();
    registerBleRoutes();
    registerUartRoutes();
    registerTextRoutes();
    registerClipboardRoutes();
    registerAutoClickerRoutes();
    registerMouseRoutes();
    registerPassthroughRoutes();
    registerLogRoutes();
    registerAppRoutes();
    registerSettingsRoutes();

    if (m_debugTrace) qDebug() << "[InterfaceLayer] Route table initialized with" << m_routes.size() << "commands";

    // 鍚姩鏃ュ織 + 娉ㄥ唽鐑敭
    appendLog(0, "System", "Application started");
    auto ports = m_serialManager->availablePorts();
    if (!ports.isEmpty()) appendLog(3, "System", "Available COM ports: " + ports.join(", "));
}

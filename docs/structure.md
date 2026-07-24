Desktop Application Structure / 桌面应用架构

Qt C++ desktop application for ESP32 HID bridge control. Manages keyboard/mouse
input capture, text transmission, and auto-clicker functionality over BLE, UART,
or HUDP transports.

Qt C++ 桌面应用，用于 ESP32 HID 桥接控制。管理键盘/鼠标输入捕获、文本传输和自动点击功能，支持 BLE、UART、HUDP 三种传输方式。

---

Architecture / 架构

```
QML (App.qml -> main.qml -> page panes / 页面面板)
  |
  | dispatch("cmd", params)
  v
InterfaceLayer  <----->  ConnectionStateManager
  |                        |   |   |
  | creates & owns:        |   |   +-- SerialManager
  |                        |   +------ BleManager
  |                        |          +-- BleSystemRouter
  |                        |               +-- WinRtBleTransport (Windows)
  |                        +---------- HudpTransport
  |                        +---------- SettingsManager
  |
  +-- CaptureManager ------> InputOutputManager --> InputQueue --> TransportSelector.activeTransport()
  |       (keyboard/mouse)        |                                      |    |    |
  |                               +<-- MouseInputManager --+            BLE  UART HUDP
  |
  +-- TextInputSource -----> InputQueue
  +-- ClipboardTextSource -> TextInputSource
  +-- AutoClickerManager --> InputQueue + MouseInputManager
  +-- LogManager
  +-- KeySettingManager
  +-- DebugTrace (static)
```

Startup Sequence / 启动流程 (main.cpp)

1. Parse CLI args (--debug / --release) to set DebugTrace
   解析命令行参数以设置 DebugTrace
2. Install global message handler routing qDebug to stderr
   安装全局消息处理器，将 qDebug 路由到 stderr
3. Create QGuiApplication and install QTranslator for i18n
   创建 QGuiApplication 并安装 QTranslator 用于国际化
4. Create InterfaceLayer iface(g_debugTrace) -- owns all managers
   创建 InterfaceLayer，拥有所有管理器
5. Install CaptureManager as QAbstractNativeEventFilter on the app
   将 CaptureManager 安装为应用的原生事件过滤器
6. Create QQmlApplicationEngine, expose context properties:
   创建 QQmlApplicationEngine，暴露上下文属性：
   InterfaceLayer, SerialManager, CaptureManager, KeySettingManager, BleManager
7. Load qrc:/App.qml and enter event loop
   加载 qrc:/App.qml 并进入事件循环

---

All Modules / 所有模块

InterfaceLayer (central coordinator / QML facade)
InterfaceLayer (中心协调器 / QML 门面)

Files / 文件: InterfaceLayer.h/.cpp, InterfaceLayer_routes_*.cpp (11 files / 11个文件)

Owns all 17 manager objects. Wires all signal/slot connections in
setupManagerConnections(). Provides dispatch(cmd, params) command router
with a QHash<QString, RouteFn> route table populated by 12 registration methods.

拥有全部 17 个管理器对象。在 setupManagerConnections() 中连接所有信号/槽。
提供 dispatch(cmd, params) 命令路由器，使用 QHash<QString, RouteFn> 路由表，
由 12 个注册方法填充。

Q_PROPERTY (exposed to QML / 暴露给 QML)

| Property / 属性 | Type / 类型 | Category / 分类 |
|----------|------|----------|
| connected | bool | Home / 主页 |
| deviceName | QString | Home / 主页 |
| captureEnabled | bool | Key settings / 按键设置 |
| captureHotkey | QString | Key settings / 按键设置 |
| capturePasteHotkey | QString | Key settings / 按键设置 |
| hudpTarget | QString | HUDP |
| hudpPort | int | HUDP |
| hudpConnecting | bool | HUDP |
| hudpStatus | QString | HUDP |
| bleScanning | bool | BLE |
| bleConnecting | bool | BLE |
| bleConnected | bool | BLE |
| bleStatus | QString | BLE |
| uartPort | QString | UART |
| uartBaudrate | int | UART |
| uartConnected | bool | UART |
| uartConnecting | bool | UART |
| uartStatus | QString | UART |
| connectionError | QString | General / 通用 |
| pairedDeviceName | QString | General / 通用 |
| pairedDeviceAddress | QString | General / 通用 |
| connectedTime | int | Connection timer / 连接计时 |
| connectedTimeStr | QString | Connection timer / 连接计时 |
| lastSentText | QString | Text input / 文本输入 |
| sending | bool | Text input / 文本输入 |
| clipboardWatching | bool | Text input / 文本输入 |
| passthroughListening | bool | Passthrough / 直通模式 |
| passthroughExclusive | bool | Passthrough / 直通模式 |
| passthroughMouseListening | bool | Passthrough / 直通模式 |
| autoClickerKeys | QVariantList | Auto clicker / 自动点击 |
| autoClickerInterval | int | Auto clicker / 自动点击 |
| autoClickerRepeatCount | int | Auto clicker / 自动点击 |
| autoClickerCompletedCycles | int | Auto clicker / 自动点击 |
| autoClickerRunning | bool | Auto clicker / 自动点击 |
| autoClickerStatus | QString | Auto clicker / 自动点击 |
| currentLogLevel | int | Logs / 日志 |
| infoCount | int | Logs / 日志 |
| warningCount | int | Logs / 日志 |
| errorLogCount | int | Logs / 日志 |
| debugCount | int | Logs / 日志 |
| logEntries | QVariantList | Logs / 日志 |
| logText | QString | Logs / 日志 |
| selectedLogDetail | QString | Logs / 日志 |
| logViewRevision | int | Logs / 日志 |
| autoStart | bool | Settings / 设置 |
| rememberLastConnection | bool | Settings / 设置 |
| configFilePath | QString | Settings / 设置 |
| settingsStatus | QString | Settings / 设置 |
| uiLanguage | QString | Settings / 设置 |
| darkMode | int | Settings / 设置 |
| appName | QString | About / 关于 |
| appVersion | QString | About / 关于 |
| buildDate | QString | About / 关于 |
| description | QString | About / 关于 |
| copyright | QString | About / 关于 |

Q_INVOKABLE

| Method / 方法 | Description / 说明 |
|--------|-------------|
| dispatch(cmd, params) | Route command to registered handler / 路由命令到已注册的处理器 |
| serialManagerObj() | Get SerialManager as QObject for QML / 获取 SerialManager 供 QML 使用 |
| captureManagerObj() | Get CaptureManager as QObject for QML / 获取 CaptureManager 供 QML 使用 |
| keySettingManagerObj() | Get KeySettingManager as QObject for QML / 获取 KeySettingManager 供 QML 使用 |
| bleManagerObj() | Get BleManager as QObject for QML / 获取 BleManager 供 QML 使用 |
| autoClickerAvailableKeys() | Get list of all available auto-clicker keys / 获取所有可用的自动点击按键列表 |

Route Files / 路由文件

| File / 文件 | Registration Method / 注册方法 | Commands / 命令 |
|------|-------------------|----------|
| InterfaceLayer_routes_key.cpp | registerKeyRoutes() | Key-related commands / 按键相关命令 |
| InterfaceLayer_routes_key.cpp | registerPassthroughRoutes() | Passthrough mode commands / 直通模式命令 |
| InterfaceLayer_routes_hudp.cpp | registerHudpRoutes() | hudp.connect, hudp.disconnect |
| InterfaceLayer_routes_ble.cpp | registerBleRoutes() | ble.scan, ble.connect, ble.disconnect |
| InterfaceLayer_routes_uart.cpp | registerUartRoutes() | uart.connect, uart.disconnect, uart.refresh |
| InterfaceLayer_routes_text.cpp | registerTextRoutes() | Text input commands / 文本输入命令 |
| InterfaceLayer_routes_clipboard.cpp | registerClipboardRoutes() | Clipboard commands / 剪贴板命令 |
| InterfaceLayer_routes_clicker.cpp | registerAutoClickerRoutes() | Auto-clicker commands / 自动点击命令 |
| InterfaceLayer_routes_mouse.cpp | registerMouseRoutes() | Mouse commands / 鼠标命令 |
| InterfaceLayer_routes_log.cpp | registerLogRoutes() | Log commands / 日志命令 |
| InterfaceLayer_routes_app.cpp | registerAppRoutes() | App-level commands / 应用级命令 |
| InterfaceLayer_routes_settings.cpp | registerSettingsRoutes() | Settings commands / 设置命令 |

---

Transport Hierarchy / 传输层层次

Base class / 基类: Transport (pure virtual / 纯虚类)

```cpp
class Transport : public QObject {
    Q_OBJECT
public:
    virtual QString name() const = 0;
    virtual bool connected() const = 0;
    virtual void write(const QByteArray &frame) = 0;
signals:
    void connectedChanged();
    void readyRead(const QByteArray &data);
    void errorOccurred(const QString &error);
};
```

| Class / 类 | name() | Implementation / 实现 |
|-------|--------|---------------|
| BleTransport | "BLE" | Wraps BleManager* / 封装 BleManager |
| SerialTransport | "UART" | Wraps SerialManager* / 封装 SerialManager |
| HudpTransport | "HUDP" | Own QUdpSocket, HUDP framing / 自有 QUdpSocket，HUDP 帧封装 |

TransportSelector / 传输选择器

Selects active transport. Priority: BLE > UART > HUDP.
选择活动传输。优先级：BLE > UART > HUDP。

chooseActiveTransport() checks each transport's connected() state in
priority order. When the active transport changes, InputQueue receives
the new pointer via setActiveTransport().

按优先级顺序检查每个传输的 connected() 状态。当活动传输变更时，
InputQueue 通过 setActiveTransport() 接收新的传输指针。

---

SerialManager (UART) / 串口管理器

File / 文件: SerialManager.h/.cpp

Qt SerialPort wrapper for ESP32 HID bridge over UART.
Qt SerialPort 封装，用于通过 UART 连接 ESP32 HID 桥接器。

Q_PROPERTY

| Property / 属性 | Type / 类型 | NOTIFY |
|----------|------|--------|
| currentPort | QString | currentPortChanged |
| connected | bool | connectedChanged |
| availablePorts | QStringList | availablePortsChanged |
| statusText | QString | statusTextChanged |
| monitorText | QString | monitorTextChanged |
| monitorRunning | bool | monitorRunningChanged |

Q_INVOKABLE

| Method / 方法 | Description / 说明 |
|--------|-------------|
| refreshPorts() | Re-enumerate available serial ports / 重新枚举可用串口 |
| startMonitor() | Start serial monitor output / 启动串口监视器输出 |
| stopMonitor() | Stop serial monitor output / 停止串口监视器输出 |
| clearMonitor() | Clear serial monitor buffer / 清空串口监视器缓冲区 |

Public Slots / 公共槽

| Method / 方法 | Description / 说明 |
|--------|-------------|
| connectToPort(port, baudrate=115200) | Open and connect to serial port / 打开并连接串口 |
| disconnectPort() | Close serial port / 关闭串口 |
| writeRaw(data) | Write raw bytes to serial port / 向串口写入原始字节 |
| changePort(newPort, baudrate=115200) | Switch to different serial port / 切换到其他串口 |

Signals / 信号

- currentPortChanged(), connectedChanged(), availablePortsChanged()
- statusTextChanged(), monitorTextChanged(), monitorRunningChanged()
- errorOccurred(error)
- rawDataReceived(data), rawBytesReceived(data)
- frameReceived(command, frame) -- complete HID Bridge v3 frames / 完整的 HID Bridge v3 帧

SerialTransport wraps SerialManager as a Transport.
SerialTransport 将 SerialManager 封装为 Transport。
- name() returns "UART"
- connected() delegates to m_serial port open state / 委托给 m_serial 端口打开状态
- write() delegates to m_serial->writeRaw(frame) / 委托给 m_serial->writeRaw(frame)

---

BleManager (BLE) / 蓝牙低功耗管理器

File / 文件: BleManager.h/.cpp

Bluetooth Low Energy scan, connect, and data transfer using Qt Bluetooth Low Energy.
蓝牙低功耗扫描、连接和数据传输，使用 Qt Bluetooth Low Energy。

Q_PROPERTY

| Property / 属性 | Type / 类型 | NOTIFY |
|----------|------|--------|
| scanning | bool | scanningChanged |
| connecting | bool | connectingChanged |
| connected | bool | connectedChanged |
| sending | bool | sendingChanged |
| statusText | QString | statusTextChanged |
| currentDeviceName | QString | currentDeviceChanged |
| currentDeviceAddress | QString | currentDeviceChanged |
| devices | QVariantList | devicesChanged |

Q_INVOKABLE

| Method / 方法 | Description / 说明 |
|--------|-------------|
| startScan() | Start scanning for nearby BLE devices / 开始扫描附近的 BLE 设备 |
| stopScan() | Stop current BLE scan / 停止当前 BLE 扫描 |
| connectToDevice(address) | Connect to BLE device by address / 通过地址连接 BLE 设备 |
| disconnectDevice() | Disconnect from current device / 断开当前设备连接 |

Signals / 信号

- scanningChanged(), connectingChanged(), connectedChanged(), sendingChanged()
- statusTextChanged(), currentDeviceChanged(), devicesChanged()
- deviceDiscovered(name, address, rssi)
- txPacket(hex, desc), rxPacket(hex, desc)
- rawBytesReceived(data), errorOccurred(error), diagnostic(message)

Internal Architecture / 内部架构

Uses QLowEnergyController. Write queue (QQueue<PendingWrite>) for chunked
BLE writes. Phase state machine: Idle -> Connecting -> DiscoveringServices ->
DiscoveringDetails -> Ready -> Disconnecting. Targets Nordic UART Service (NUS)
RX characteristic for writes.

使用 QLowEnergyController。写入队列 (QQueue<PendingWrite>) 用于分块 BLE 写入。
阶段状态机：Idle -> Connecting -> DiscoveringServices -> DiscoveringDetails ->
Ready -> Disconnecting。目标是 Nordic UART Service (NUS) RX 特征值。

WinRtBleTransport (Windows-only) / Windows 专用 BLE 传输

File / 文件: WinRtBleTransport.h/.cpp

Windows GATT transport bypassing QtBluetooth WinRT backend for direct ATT access.
Windows GATT 传输，绕过 QtBluetooth WinRT 后端以直接访问 ATT 层。

| Method / 方法 | Description / 说明 |
|--------|-------------|
| isConnecting() | Connection in progress / 正在连接中 |
| isConnected() | Currently connected / 当前已连接 |
| isSending() | Write in progress / 正在写入 |
| statusText() | Status string / 状态字符串 |
| connectToAddress(address, name) | Connect via Windows GATT API / 通过 Windows GATT API 连接 |
| disconnectDevice() | Disconnect and release resources / 断开并释放资源 |
| writeFrame(frame) | Write to NUS RX via GATT / 通过 GATT 写入 NUS RX |

Uses void *m_nativeState for opaque native WinRT COM GATT objects.
使用 void *m_nativeState 保存不透明的原生 WinRT COM GATT 对象。

BleSystemRouter / BLE 系统路由器

Platform router: on Windows returns usesNativeTransport()=true and provides
WinRtBleTransport*; on other platforms returns nullptr.

平台路由器：Windows 上返回 usesNativeTransport()=true 并提供 WinRtBleTransport*；
其他平台返回 nullptr。

BleTransport / BLE 传输

Wraps BleManager* as a Transport.
将 BleManager* 封装为 Transport。
- name() returns "BLE"
- connected() delegates to m_ble->isConnected() / 委托给 m_ble->isConnected()
- write() delegates to m_ble->writeFrame(frame) / 委托给 m_ble->writeFrame(frame)

---

HudpProtocol / HUDP 协议

File / 文件: HudpProtocol.h/.cpp

UDP-based protocol for HID bridge communication.
基于 UDP 的 HID 桥接通信协议。

Constants / 常量

| Constant / 常量 | Value / 值 |
|----------|-------|
| Default port / 默认端口 | 45820 |
| Version / 版本 | 1 |
| Header size / 头部大小 | 22 bytes / 字节 |
| Max payload / 最大载荷 | 1024 bytes / 字节 |

Packet Types / 包类型 (Type enum)

| Value / 值 | Name / 名称 |
|-------|------|
| 0x01 | KeyboardEvent / 键盘事件 |
| 0x02 | KeyboardState / 键盘状态 |
| 0x03 | MouseEvent / 鼠标事件 |
| 0x04 | MouseState / 鼠标状态 |
| 0x05 | KeepAlive / 保活 |
| 0x06 | StateRequest / 状态请求 |
| 0x07 | StateReset / 状态重置 |
| 0x08 | Ack / 确认 |
| 0x20 | LegacyBinaryFrame / 传统二进制帧 |

Flags / 标志位

| Bit / 位 | Name / 名称 |
|-----|------|
| 1 | AckRequired / 需要确认 |
| 2 | Retransmission / 重传 |
| 4 | FullState / 完整状态 |
| 8 | Encrypted / 加密 |
| 16 | Compressed / 压缩 |
| 32 | ResetState / 重置状态 |

Packet Layout / 包布局 (big-endian / 大端序)

```
Offset  Size  Field / 字段
0       4     Magic "HUDP" (ASCII)
4       1     Version / 版本 (0x01)
5       1     Type / 类型
6       2     Flags / 标志
8       4     SessionId / 会话ID
12      4     Sequence / 序列号
16      4     TimestampMs / 时间戳(毫秒)
20      2     PayloadLength / 载荷长度
22      N     Payload / 载荷 (up to 1024 bytes / 最大 1024 字节)
22+N    4     CRC32
```

CRC32: polynomial 0xEDB88320, init 0xFFFFFFFF, final XOR 0xFFFFFFFF.
Computed over header + payload before appending.
CRC32：多项式 0xEDB88320，初始值 0xFFFFFFFF，最终异或 0xFFFFFFFF。
在追加之前对头部和载荷进行计算。

HudpTransport / HUDP 传输

File / 文件: HudpTransport.h/.cpp

UDP transport using QUdpSocket.
使用 QUdpSocket 的 UDP 传输。

| Method / 方法 | Description / 说明 |
|--------|-------------|
| name() | Returns "HUDP" / 返回 "HUDP" |
| connected() | Returns internal connected state / 返回内部连接状态 |
| connectToTarget(host, port) | Open UDP connection / 打开 UDP 连接 |
| disconnectFromTarget() | Close UDP connection / 关闭 UDP 连接 |
| write(frame) | Wrap in LegacyBinaryFrame (0x20) and send / 封装为 LegacyBinaryFrame 并发送 |
| send(type, flags, payload) | Send raw HUDP packet / 发送原始 HUDP 包 |

Maintains m_sessionId, m_sequence, m_clock (elapsed timer for timestamps).
HUDP is write-only in current implementation (no incoming data path).
维护 m_sessionId、m_sequence、m_clock（用于时间戳的经过时间计时器）。
当前实现中 HUDP 仅支持写入（无接收数据路径）。

---

KeyboardProtocol / 键盘协议

File / 文件: KeyboardProtocol.h/.cpp

Static utility class for keyboard/text HID bridge frames.
键盘/文本 HID 桥接帧的静态工具类。

Frame Format / 帧格式

```
[0xAA] [cmd] [len_lo] [len_hi] [payload...]
```

- Prefix / 前缀: 0xAA (magic byte / 魔数字节)
- Command / 命令: 1 byte / 1 字节
- Length / 长度: 2 bytes, little-endian / 2 字节，小端序
- Payload / 载荷: variable / 可变长度

Commands / 命令

| Command / 命令 | Description / 说明 | Payload / 载荷 |
|---------|-------------|---------|
| 0x02 | Raw keycode event / 原始键码事件 | [scanCode_lo, scanCode_hi, pressed] |
| text typing / 文本输入 | "type <text>\n" | UTF-8 encoded string / UTF-8 编码字符串 |

Firmware Response Codes / 固件响应码

| Code / 码 | Meaning / 含义 |
|------|---------|
| 0x00 | OK |
| 0xFE | NACK / 否定确认 |
| 0xFF | BUSY / 忙 |

Key Definitions / 按键定义

Full table covers: Esc, F1-F12, PrintScreen, ScrollLock, Pause, alphanumeric
keys, modifiers (Shift/Ctrl/Alt/Meta left+right), arrow keys, navigation
(Insert/Home/PgUp/Delete/End/PgDn), numpad (0-9, operators, Enter, decimal),
and international keys. Each printable key has normal and shifted character.

完整表涵盖：Esc、F1-F12、PrintScreen、ScrollLock、Pause、字母数字键、
修饰键（Shift/Ctrl/Alt/Meta 左右）、方向键、导航键（Insert/Home/PgUp/
Delete/End/PgDn）、小键盘（0-9、运算符、Enter、小数点）和国际按键。
每个可打印键都有常规字符和 Shift 字符。

Static Methods / 静态方法

| Method / 方法 | Description / 说明 |
|--------|-------------|
| encodeTextCommand(text) | Build text typing command bytes / 构建文本输入命令字节 |
| encodeTextPacket(text, transportLabel) | Build text Packet with label / 构建带标签的文本 Packet |
| encodeRawKeycode(scanCode, pressed) | Build raw keycode frame / 构建原始键码帧 |
| buildFrame(cmd, payload) | Build generic frame with 0xAA prefix / 构建带 0xAA 前缀的通用帧 |
| validateTextMapping(text) | Check text-to-keyboard mapping coverage / 检查文本到键盘映射覆盖率 |
| keyUsageForCapturedInput(key, text, *usage) | Map captured key to HID usage / 将捕获的按键映射到 HID 用法码 |
| keyDefinitions() | Get all key definitions / 获取所有按键定义 |
| dualCharacterKeys() | Get keys with dual characters / 获取具有双重字符的按键 |
| describeTextCommand(transport, textBytes) | Human-readable text command / 人类可读的文本命令描述 |
| describeRawKeycode(scanCode, pressed) | Human-readable keycode / 人类可读的键码描述 |
| describeResponse(response) | Human-readable firmware response / 人类可读的固件响应描述 |
| hexDump(data) | Hex string of data / 数据的十六进制字符串 |

---

MouseProtocol / 鼠标协议

File / 文件: MouseProtocol.h/.cpp

Report Format / 报告格式

```
[0xAA] [0x06] [len_lo] [len_hi] [buttons] [dx] [dy] [wheel]
```

Command 0x06, 4-byte payload. Uses KeyboardProtocol::buildFrame().
命令 0x06，4 字节载荷。使用 KeyboardProtocol::buildFrame()。

| Field / 字段 | Size / 大小 | Range / 范围 |
|-------|------|-------|
| buttons / 按钮 | 1 byte / 1 字节 | Bitmask / 位掩码: Left=0x01, Right=0x02, Middle=0x04 |
| dx | 1 byte signed / 1 字节有符号 | -127 to +127 |
| dy | 1 byte signed / 1 字节有符号 | -127 to +127 |
| wheel / 滚轮 | 1 byte signed / 1 字节有符号 | -127 to +127 |

Static Methods / 静态方法

| Method / 方法 | Description / 说明 |
|--------|-------------|
| encodeReport(buttons, dx, dy, wheel) | Build mouse report frame / 构建鼠标报告帧 |
| describeReport(buttons, dx, dy, wheel) | Human-readable report / 人类可读的报告描述 |

---

InputQueue / 输入队列

File / 文件: InputQueue.h/.cpp

Packet buffer with 10ms flush timer. Writes to active transport.
数据包缓冲区，10ms 刷新定时器。写入活动传输。

Q_PROPERTY

| Property / 属性 | Type / 类型 | NOTIFY |
|----------|------|--------|
| queueSize | int | queueSizeChanged |
| sending | bool | sendingChanged |

Methods / 方法

| Method / 方法 | Description / 说明 |
|--------|-------------|
| setActiveTransport(transport) | Set the transport to write through / 设置要写入的传输 |
| enqueue(packet) | Add packet to queue and try flush / 将数据包加入队列并尝试刷新 |
| tryFlush() | Drain queue through active transport / 通过活动传输清空队列 |

Signals / 信号

- queueSizeChanged(), sendingChanged()
- packetWritten(transport, hex, label)
- errorOccurred(error)

Packet struct / 数据包结构:
```cpp
struct Packet { QByteArray bytes; QString label; int priority = 0; };
```

---

InputOutputManager / 输入输出管理器

File / 文件: InputOutputManager.h/.cpp

Bridges input producers to InputQueue. Owns protocol encoding.
桥接输入生产者到 InputQueue。拥有协议编码逻辑。

Methods / 方法

| Method / 方法 | Description / 说明 |
|--------|-------------|
| handleCapturedKey(key, text, pressed, autoRepeat, modifiers) | Process captured keyboard event / 处理捕获的键盘事件 |

Drops auto-repeat events. Maps keys via KeyboardProtocol::keyUsageForCapturedInput().
Emits keyUnmapped if no HID mapping found.

丢弃自动重复事件。通过 KeyboardProtocol::keyUsageForCapturedInput() 映射按键。
如果没有 HID 映射则发射 keyUnmapped 信号。

Signals / 信号

- outputRejected(source, detail)
- keyUnmapped(key, text)
- mouseQueued(buttons, dx, dy, wheel, transport)
- capturedKeyQueued(key, usage, pressed, transport)

---

CaptureManager / 捕获管理器

File / 文件: CaptureManager.h/.cpp

Inherits QObject and QAbstractNativeEventFilter. Global hotkey and
app-foreground keyboard/mouse capture.

继承 QObject 和 QAbstractNativeEventFilter。全局热键和应用前台键盘/鼠标捕获。

Q_PROPERTY

| Property / 属性 | Type / 类型 | NOTIFY |
|----------|------|--------|
| registered | bool | registeredChanged |
| hotkey | QString | hotkeyChanged |
| appCaptureListening | bool | appCaptureModeChanged |
| appCaptureExclusive | bool | appCaptureModeChanged |
| mouseCaptureListening | bool | appCaptureModeChanged |

Q_INVOKABLE

| Method / 方法 | Description / 说明 |
|--------|-------------|
| registerHotkey(keys) | Register global hotkey from string like "Ctrl+`" / 从字符串注册全局热键 |
| unregisterHotkey() | Unregister global hotkey / 取消注册全局热键 |
| setAppCaptureListening(enabled) | Enable/disable listen-only keyboard capture / 启用/禁用监听模式键盘捕获 |
| setAppCaptureExclusive(enabled) | Enable/disable exclusive keyboard capture / 启用/禁用独占模式键盘捕获 |
| setMouseCaptureListening(enabled) | Enable/disable mouse capture passthrough / 启用/禁用鼠标捕获直通 |

Signals / 信号

- registeredChanged(), hotkeyChanged(), hotkeyTriggered(), appCaptureModeChanged()
- appKeyCaptured(key, text, pressed, autoRepeat, modifiers)
- appMouseCaptured(buttons, dx, dy)
- appMouseWheelCaptured(delta)

Listening and exclusive modes are mutually exclusive. Mouse capture never
consumes local events (always passthrough). Windows-only hotkey registration
via RegisterHotKey/UnregisterHotKey.

监听模式和独占模式互斥。鼠标捕获从不消耗本地事件（始终直通）。
热键注册仅限 Windows，通过 RegisterHotKey/UnregisterHotKey 实现。

---

MouseInputManager / 鼠标输入管理器

File / 文件: MouseInputManager.h/.cpp

Aggregates relative mouse deltas with 4ms flush timer (250 Hz).
聚合相对鼠标增量，4ms 刷新定时器（250 Hz）。

Methods / 方法

| Method / 方法 | Description / 说明 |
|--------|-------------|
| moveBy(dx, dy) | Accumulate movement / 累加移动量 |
| wheelBy(delta) | Accumulate wheel / 累加滚轮量 |
| setButton(button, pressed) | Set/clear single button bit / 设置/清除单个按钮位 |
| setButtons(buttons) | Flush pending, set full button state / 刷新待处理项，设置完整按钮状态 |
| releaseAll() | Flush, clear all buttons, emit report / 刷新，清除所有按钮，发射报告 |
| resetState() | Clear everything without sending / 清除所有状态但不发送 |
| flushPending() | Emit reports for accumulated deltas (clamped to [-127,127]) / 为累积增量发射报告（限制在 [-127,127]） |

Signals / 信号

- buttonsChanged()
- reportRequested(buttons, dx, dy, wheel)

Flushes pending motion before button transitions to preserve event ordering.
Large deltas are split, not clipped.

在按钮状态变更前刷新待处理的移动，以保持事件顺序。
大增量会被分割，而不是截断。

---

TextInputSource / 文本输入源

File / 文件: TextInputSource.h/.cpp

Validates text mapping and enqueues text packets.
验证文本映射并入队文本数据包。

Methods / 方法

| Method / 方法 | Description / 说明 |
|--------|-------------|
| enqueueText(text, source) | Queue text for validation and sending / 将文本加入验证和发送队列 |
| flushNext() | Process next queued request / 处理下一个队列请求 |

Signals / 信号

- queueSizeChanged()
- mappingChecked(request, mappedChars, totalChars, unmappedCharacters)
- textQueued(text, source, transport)
- sendRejected(source, length)

ClipboardTextSource / 剪贴板文本源

File / 文件: ClipboardTextSource.h/.cpp

Watches system clipboard and produces text events.
监视系统剪贴板并产生文本事件。

Methods / 方法

| Method / 方法 | Description / 说明 |
|--------|-------------|
| requestText() | Read current clipboard text / 读取当前剪贴板文本 |
| setWatching(enable) | Enable/disable continuous clipboard watch / 启用/禁用持续剪贴板监视 |

Signals / 信号

- watchingChanged(), textProduced(text, source), emptyText(), errorOccurred(error)

Deduplicates: tracks last text, only emits on change.
去重：跟踪上次文本，仅在变化时发射。

---

ConnectionStateManager / 连接状态管理器

File / 文件: ConnectionStateManager.h/.cpp

Aggregates all transport states into single coherent view. Handles auto-reconnect.
将所有传输状态聚合为统一视图。处理自动重连。

Signals / 信号

- connectedChanged()
- hudpTargetChanged(), hudpPortChanged(), hudpConnectingChanged(), hudpStatusChanged()
- bleScanningChanged(), bleConnectingChanged(), bleConnectedChanged(), bleStatusChanged()
- uartPortChanged(), uartBaudrateChanged(), uartConnectedChanged(), uartConnectingChanged(), uartStatusChanged()
- connectionErrorChanged(), pairedDeviceNameChanged(), pairedDeviceAddressChanged()
- connectedTimeChanged()
- diagnostic(level, module, message)

Key behaviors / 关键行为:
- Recalculates aggregate connected from all transport states / 从所有传输状态重新计算聚合 connected
- On disconnect: stops auto-clicker and mouse capture via protectOnNoTransport() / 断开时：通过 protectOnNoTransport() 停止自动点击和鼠标捕获
- BLE restore: watches BleManager::devicesChanged and auto-connects when remembered device appears / BLE 恢复：监视 BleManager::devicesChanged，当记住的设备出现时自动连接
- Handles SettingsManager restore signals on startup (300ms delay) / 启动时处理 SettingsManager 恢复信号（300ms 延迟）

---

SettingsManager / 设置管理器

File / 文件: SettingsManager.h/.cpp

Persistent JSON configuration, auto-start, import/export.
持久化 JSON 配置、开机自启、导入/导出。

Getters / 获取器

| Getter / 获取器 | Type / 类型 | Default / 默认值 |
|--------|------|---------|
| autoStart() | bool | false |
| rememberLastConnection() | bool | false |
| configFilePath() | QString | resolved at construction / 构造时解析 |
| logFilePath() | QString | app.log alongside config / 与配置文件同目录 |
| status() | QString | "" |
| uiLanguage() | QString | "zh-CN" |
| darkMode() | int | 0 (auto / 自动) |
| lastConnectionTransport() | QString | "" |
| lastConnectionConnected() | bool | false |
| lastUartPort() | QString | "" |
| lastUartBaudrate() | int | 115200 |
| lastBleAddress() | QString | "" |
| lastHudpTarget() | QString | "" |
| lastHudpPort() | int | 45820 |

Signals / 信号

- configFilePathChanged(), statusChanged()
- restoreUartRequested(port, baudrate)
- restoreBleRequested(address)
- restoreHudpRequested(target, port)

Persistence / 持久化

JSON file via QSaveFile (atomic writes). Config path resolution:
通过 QSaveFile 写入 JSON 文件（原子写入）。配置路径解析：

```
QStandardPaths::AppConfigLocation/settings.json
  fallback / 回退: AppDataLocation/settings.json
  fallback / 回退: ~/.config/ESP32S3KeyboardBridge/settings.json
```

JSON Structure / JSON 结构

```json
{
  "version": 1,
  "settings": {
    "autoStart": false,
    "rememberLastConnection": false,
    "uiLanguage": "zh-CN",
    "darkMode": 0
  },
  "keySettings": {
    "captureEnabled": true,
    "captureHotkey": "Ctrl+`",
    "capturePasteHotkey": "Ctrl+V"
  },
  "lastConnection": {
    "transport": "",
    "connected": false,
    "uart": { "port": "", "baudrate": 115200 },
    "ble": { "address": "" },
    "hudp": { "target": "", "port": 45820 }
  }
}
```

Migration / 迁移: old key "kmudp" maps to "hudp", legacy "KMUDP" transport
values rewritten to "HUDP".
旧键 "kmudp" 映射为 "hudp"，遗留的 "KMUDP" 传输值重写为 "HUDP"。

Auto-start Implementation / 开机自启实现

- Windows: HKCU\Software\Microsoft\Windows\CurrentVersion\Run
- macOS: LaunchAgent plist in ~/Library/LaunchAgents/
- Linux: XDG .desktop file in ~/.config/autostart/

---

KeySettingManager / 按键设置管理器

File / 文件: KeySettingManager.h/.cpp

Stores hotkey configuration. Uses QSettings("ESP32KB", "Bridge").
存储热键配置。使用 QSettings("ESP32KB", "Bridge")。

Q_PROPERTY

| Property / 属性 | Type / 类型 | Default / 默认值 | NOTIFY |
|----------|------|---------|--------|
| captureEnabled | bool | true | captureEnabledChanged |
| captureHotkey | QString | "Ctrl+`" | captureHotkeyChanged |
| capturePasteHotkey | QString | "Ctrl+V" | capturePasteHotkeyChanged |

Q_INVOKABLE

| Method / 方法 | Description / 说明 |
|--------|-------------|
| resetToDefaults() | Reset to factory defaults / 恢复出厂设置 |
| loadFromSettings() | Load from QSettings / 从 QSettings 加载 |
| saveToSettings() | Save to QSettings / 保存到 QSettings |
| exportToJson() | Export as JSON string / 导出为 JSON 字符串 |
| importFromJson(json) | Import from JSON string / 从 JSON 字符串导入 |
| exportToFile(filePath) | Export to JSON file / 导出到 JSON 文件 |
| importFromFile(filePath) | Import from JSON file / 从 JSON 文件导入 |

Signals / 信号

- captureEnabledChanged(), captureHotkeyChanged(), capturePasteHotkeyChanged()
- configImported(), configExported()

---

LogManager / 日志管理器

File / 文件: LogManager.h/.cpp

In-memory log storage with level filtering and UI debouncing.
内存日志存储，支持级别过滤和 UI 防抖。

Log Levels / 日志级别

| Value / 值 | Name / 名称 |
|-------|------|
| 0 | INFO |
| 1 | WARN |
| 2 | ERROR |
| 3 | DEBUG |

Getters / 获取器

| Getter / 获取器 | Type / 类型 |
|--------|------|
| currentLogLevel() | int |
| infoCount() | int |
| warningCount() | int |
| errorLogCount() | int |
| debugCount() | int |
| logEntries() | QVariantList (filtered visible entries / 过滤后的可见条目) |
| allLogs() | QVariantList (full chronological snapshot / 完整时间顺序快照) |
| selectedLogDetail() | QString |
| logViewRevision() | int |

Signals / 信号

- currentLogLevelChanged(), infoCountChanged(), warningCountChanged()
- errorLogCountChanged(), debugCountChanged(), logEntriesChanged()
- selectedLogDetailChanged(), logViewRevisionChanged()

Log entry format / 日志条目格式: { "timestamp", "level", "module", "message", "detail" }.
Storage / 存储: in-memory ring buffer / 内存环形缓冲区。100ms debounce for UI updates / UI 更新防抖 100ms。
Export / 导出: exportJson(path) writes JSON array to disk / 将 JSON 数组写入磁盘。

---

AutoClickerManager / 自动点击管理器

File / 文件: AutoClickerManager.h/.cpp

Timed key-repeat sequences. Single-shot timer per cycle.
定时按键重复序列。每个周期使用单次触发定时器。

State / 状态

| Getter / 获取器 | Type / 类型 | Default / 默认值 |
|--------|------|---------|
| keys() | QList<quint16> | empty / 空 |
| intervalMs() | int | 100 (range / 范围: 10-60000) |
| repeatCount() | int | 0 (infinite / 无限) |
| completedCycles() | int | 0 |
| running() | bool | false |
| status() | QString | computed / 计算得出 |

Signals / 信号

- keysChanged(), intervalChanged(), repeatCountChanged()
- completedCyclesChanged(), runningChanged(), statusChanged()
- clickRequested(keys), cycleEnqueued(keyCount, cycleNumber), sendRejected()

Cycle / 周期: for each key, enqueues key-down then key-up via InputQueue. Mouse
buttons (0xff01=left, 0xff02=right, 0xff04=middle) use
MouseInputManager::setButton().

对每个按键，通过 InputQueue 先入队按下再入队释放。鼠标按钮
（0xff01=左键, 0xff02=右键, 0xff04=中键）使用 MouseInputManager::setButton()。

---

DebugTrace / 调试跟踪

File / 文件: DebugTrace.h/.cpp

Static utility class controlling global debug output.
控制全局调试输出的静态工具类。

| Method / 方法 | Description / 说明 |
|--------|-------------|
| setEnabled(enabled) | Enable/disable debug trace globally / 全局启用/禁用调试跟踪 |
| enabled() | Check if debug trace is enabled / 检查调试跟踪是否启用 |

Guards qDebug() output and LogManager::append for DEBUG level (level 3).
保护 qDebug() 输出和 LogManager 的 DEBUG 级别（级别 3）日志追加。

---

Data Flow / 数据流

Output Path / 输出路径 (User Input -> Transport / 用户输入 -> 传输)

Path A: Keyboard passthrough / 路径 A：键盘直通

```
CaptureManager (nativeEventFilter)
  -> appKeyCaptured(key, text, pressed, autoRepeat, modifiers)
  -> InputOutputManager::handleCapturedKey()
     -> KeyboardProtocol::keyUsageForCapturedInput() -> HID usage code / HID 用法码
     -> KeyboardProtocol::encodeRawKeycode(usage, pressed) -> Packet
     -> InputQueue::enqueue(packet)
```

Auto-repeat events dropped. Unmapped keys emit keyUnmapped.
自动重复事件被丢弃。未映射的按键发射 keyUnmapped 信号。

Path B: Mouse passthrough / 路径 B：鼠标直通

```
CaptureManager (nativeEventFilter)
  -> appMouseCaptured(buttons, dx, dy)
  -> MouseInputManager::setButtons() + moveBy()
     -> 4ms flush timer / 4ms 刷新定时器
     -> reportRequested(buttons, dx, dy, wheel)
     -> InputOutputManager (lambda)
        -> MouseProtocol::encodeReport() -> Packet
        -> InputQueue::enqueue(packet)
```

Path C: Text input / 路径 C：文本输入

```
QML UI or ClipboardTextSource
  -> textProduced(text, source)
  -> TextInputSource::enqueueText()
     -> flushNext()
        -> KeyboardProtocol::validateTextMapping()
        -> KeyboardProtocol::encodeTextPacket() -> Packet
        -> InputQueue::enqueue(packet)
```

Queue -> Transport / 队列 -> 传输:

```
InputQueue::tryFlush() (10ms timer + manual / 10ms 定时器 + 手动)
  -> while queue non-empty AND activeTransport connected:
       activeTransport->write(packet.bytes)
       emit packetWritten (logged / 已记录日志)
  -> if no transport: emit errorOccurred("No active transport")
     如果没有传输: 发射 errorOccurred("No active transport")
```

Input Path / 输入路径 (Incoming Data / 接收数据)

BLE and Serial transports emit readyRead(data) via their adapter classes.
BLE 和串口传输通过适配器类发射 readyRead(data) 信号。
HUDP is write-only in current implementation. No visible subscriber to
Transport::readyRead exists in the current codebase.

当前实现中 HUDP 仅支持写入。当前代码库中没有 Transport::readyRead 的可见订阅者。

---

Persistence Files / 持久化文件

| File / 文件 | Location / 位置 | Purpose / 用途 |
|------|----------|---------|
| settings.json | QStandardPaths::AppConfigLocation/ | Application configuration / 应用配置 |
| app.log | Same directory as settings.json / 与 settings.json 同目录 | Application log entries / 应用日志条目 |
| trace.log | Same directory as settings.json / 与 settings.json 同目录 | Qt qDebug/warning/critical trace / Qt 调试跟踪 |

Do not add absolute local paths. Use SettingsManager::defaultConfigFilePath()
or the same fallback algorithm during bootstrap.

不要添加绝对本地路径。使用 SettingsManager::defaultConfigFilePath() 或启动时使用相同的回退算法。

---

Build / 构建

Desktop build / 桌面构建 (CMakePresets.json):
```powershell
cmake --preset debug
cmake --build --preset debug
```

Unit tests / 单元测试:
```powershell
cmake --build D:/github/wirelesskm/src/test/build_tests --config Debug
ctest --test-dir D:/github/wirelesskm/src/test/build_tests -C Debug --output-on-failure
```

src/test/build_tests/ is generated output. Qt tests for keyboard protocol,
input queue, transport selection, logging and auto clicker are self-contained.

src/test/build_tests/ 是生成的输出目录。键盘协议、输入队列、传输选择、日志和
自动点击的 Qt 测试是自包含的。

---

Change Checklist / 变更检查清单

1. New behavior with state or timers goes in a dedicated manager
   带有状态或定时器的新行为放在专用管理器中
2. Add route file entry, expose only required properties/signals through InterfaceLayer
   添加路由文件条目，仅通过 InterfaceLayer 暴露所需的属性/信号
3. Add QML page to both main.qml navigation and qml.qrc
   将 QML 页面添加到 main.qml 导航和 qml.qrc 中
4. Route HID output through InputQueue
   通过 InputQueue 路由 HID 输出
5. Release held mouse buttons on page close, capture stop, focus loss, or transport loss
   在页面关闭、捕获停止、焦点丢失或传输丢失时释放按住的鼠标按钮
6. Persist only through SettingsManager; keep logs next to settings.json
   仅通过 SettingsManager 持久化；日志保存在 settings.json 旁边
7. Add focused tests, run Debug build and git diff --check before commit
   添加针对性测试，提交前运行 Debug 构建和 git diff --check

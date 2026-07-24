#include "BleManager.h"

#include "DebugTrace.h"
#include "KeyboardProtocol.h"

#include <QDebug>
#include <QLowEnergyDescriptor>
#include <QUuid>
#include <utility>

#ifdef Q_OS_WIN
#include "BleSystemRouter.h"
#include "WinRtBleTransport.h"
#endif

namespace {
const QBluetoothUuid NordicUartService(QStringLiteral("6e400001-b5a3-f393-e0a9-e50e24dcca9e"));
const QBluetoothUuid NordicUartRx(QStringLiteral("6e400002-b5a3-f393-e0a9-e50e24dcca9e"));

constexpr int ScanTimeoutMs = 8000;
constexpr int ConnectTimeoutMs = 10000;
constexpr int ServiceDiscoveryTimeoutMs = 10000;
constexpr int DetailDiscoveryTimeoutMs = 10000;
constexpr int WriteResponseTimeoutMs = 2000;
constexpr int WriteWithoutResponsePaceMs = 12;
constexpr int ConservativePayloadSize = 20;

// Format a Bluetooth UUID as a lowercase string without braces
// 将蓝牙 UUID 格式化为不带花括号的小写字符串
QString uuidText(const QBluetoothUuid &uuid)
{
    return uuid.toString(QUuid::WithoutBraces).toLower();
}

// Convert a BLE service error enum to a human-readable string
// 将 BLE 服务错误枚举转换为人类可读字符串
QString serviceErrorText(QLowEnergyService::ServiceError error)
{
    switch (error) {
    case QLowEnergyService::OperationError: return "operation error";
    case QLowEnergyService::CharacteristicWriteError: return "characteristic write error";
    case QLowEnergyService::DescriptorWriteError: return "descriptor write error";
    case QLowEnergyService::CharacteristicReadError: return "characteristic read error";
    case QLowEnergyService::DescriptorReadError: return "descriptor read error";
    case QLowEnergyService::UnknownError: return "unknown service error";
    case QLowEnergyService::NoError: return "no error";
    }
    return "unrecognized service error";
}
}

// Constructor: initialise discovery agent, stage/write timers, and Windows native transport
// 构造函数: 初始化发现代理、阶段/写入定时器和 Windows 原生传输
BleManager::BleManager(QObject *parent)
    : QObject(parent)
{
    m_agent = new QBluetoothDeviceDiscoveryAgent(this);
    m_agent->setLowEnergyDiscoveryTimeout(ScanTimeoutMs);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BleManager::onDeviceDiscovered);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
            this, &BleManager::onScanError);
    connect(m_agent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BleManager::onScanFinished);

    m_stageTimer = new QTimer(this);
    m_stageTimer->setSingleShot(true);
    connect(m_stageTimer, &QTimer::timeout, this, [this] {
        failConnection("BLE timeout while " + m_stage);
    });

    m_writeTimer = new QTimer(this);
    m_writeTimer->setSingleShot(true);
    connect(m_writeTimer, &QTimer::timeout, this, [this] {
        if (!m_writeInFlight)
            return;
        if (m_inFlightMode == QLowEnergyService::WriteWithoutResponse) {
            finishCurrentWrite();
            return;
        }
        const QString message = "BLE write timeout uuid=" + uuidText(m_writeChar.uuid())
            + " len=" + QString::number(m_inFlightWrite.size());
        emit diagnostic(message);
        emit errorOccurred(message);
        clearWrites();
    });

#ifdef Q_OS_WIN
    m_systemRouter = new BleSystemRouter(this);
    auto *native = m_systemRouter->nativeTransport();
    connect(native, &WinRtBleTransport::connectingChanged, this, [this, native] {
        setConnecting(native->isConnecting());
    });
    connect(native, &WinRtBleTransport::connectedChanged, this, [this, native] {
        setConnected(native->isConnected());
    });
    connect(native, &WinRtBleTransport::sendingChanged, this, [this, native] {
        setSending(native->isSending());
    });
    connect(native, &WinRtBleTransport::statusTextChanged, this, [this, native] {
        setStatus(native->statusText());
    });
    connect(native, &WinRtBleTransport::errorOccurred, this, &BleManager::errorOccurred);
    connect(native, &WinRtBleTransport::diagnostic, this, &BleManager::diagnostic);
    connect(native, &WinRtBleTransport::txPacket, this, &BleManager::txPacket);
#endif
}

// Start scanning for nearby BLE devices using low-energy discovery
// 使用低功耗发现开始扫描附近的 BLE 设备
void BleManager::startScan()
{
    if (m_scanning)
        return;
    if (m_agent->isActive())
        m_agent->stop();

    m_devices.clear();
    m_deviceInfos.clear();
    emit devicesChanged();
    setStatus("Scanning...");
    m_scanning = true;
    emit scanningChanged();
    m_agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

// Stop the current BLE device scan
// 停止当前 BLE 设备扫描
void BleManager::stopScan()
{
    if (!m_scanning)
        return;
    m_agent->stop();
    m_scanning = false;
    emit scanningChanged();
    setStatus("Scan stopped");
}

// Look up the device by address, stop scanning, and initiate a BLE connection
// 通过地址查找设备，停止扫描，并发起 BLE 连接
void BleManager::connectToDevice(const QString &address)
{
    if (address.isEmpty()) {
        emit errorOccurred("BLE device address is empty");
        return;
    }

    QBluetoothDeviceInfo target;
    bool found = false;
    for (const auto &info : std::as_const(m_deviceInfos)) {
        if (info.address().toString().compare(address, Qt::CaseInsensitive) == 0) {
            target = info;
            found = true;
            break;
        }
    }
    if (!found) {
        emit errorOccurred("BLE device not found in scan list: " + address);
        return;
    }

    stopScan();
#ifdef Q_OS_WIN
    resetTransport();
    m_currentDeviceName = target.name().isEmpty() ? "Unknown" : target.name();
    m_currentDeviceAddress = target.address().toString();
    emit currentDeviceChanged();
    m_systemRouter->nativeTransport()->connectToAddress(m_currentDeviceAddress, m_currentDeviceName);
    return;
#endif
    resetTransport();
    m_currentDeviceName = target.name().isEmpty() ? "Unknown" : target.name();
    m_currentDeviceAddress = target.address().toString();
    emit currentDeviceChanged();
    m_phase = Phase::Connecting;
    setConnecting(true);
    setStatus("Connecting " + m_currentDeviceName + "...");
    startStageTimeout(ConnectTimeoutMs, "connecting to " + m_currentDeviceName);

    m_controller = QLowEnergyController::createCentral(target, this);
    connect(m_controller, &QLowEnergyController::connected, this, &BleManager::onControllerConnected);
    connect(m_controller, &QLowEnergyController::disconnected, this, &BleManager::onControllerDisconnected);
    connect(m_controller, &QLowEnergyController::serviceDiscovered, this, &BleManager::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished, this, &BleManager::onServiceDiscoveryFinished);
    connect(m_controller, &QLowEnergyController::errorOccurred, this, &BleManager::onControllerError);
    m_controller->connectToDevice();
}

// Disconnect from the current BLE device and reset transport state
// 断开与当前 BLE 设备的连接并重置传输状态
void BleManager::disconnectDevice()
{
#ifdef Q_OS_WIN
    m_systemRouter->nativeTransport()->disconnectDevice();
    return;
#endif
    m_stageTimer->stop();
    clearWrites();
    if (!m_controller) {
        m_phase = Phase::Idle;
        setConnected(false);
        setConnecting(false);
        setStatus("Disconnected");
        return;
    }
    m_phase = Phase::Disconnecting;
    setStatus("Disconnecting...");
    m_controller->disconnectFromDevice();
}

// Write a data frame to the Nordic UART RX characteristic via the BLE service
// 通过 BLE 服务向 Nordic UART RX 特征值写入一帧数据
void BleManager::writeFrame(const QByteArray &frame)
{
#ifdef Q_OS_WIN
    m_systemRouter->nativeTransport()->writeFrame(frame);
    return;
#endif
    if (frame.isEmpty())
        return;
    if (!m_connected || !m_service || !m_writeChar.isValid()) {
        emit errorOccurred("BLE is not ready to send");
        return;
    }
    enqueueWrite(frame, QString("BLE_FRAME (%1 bytes)").arg(frame.size()));
}

// Handle a newly discovered BLE device: add to list and emit signals
// 处理新发现的 BLE 设备: 添加到列表并发出信号
void BleManager::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    if (!(info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration))
        return;
    const QString address = info.address().toString();
    for (const QVariant &device : std::as_const(m_devices)) {
        if (device.toMap().value("address").toString().compare(address, Qt::CaseInsensitive) == 0)
            return;
    }
    const QString name = info.name().isEmpty() ? "Unknown" : info.name();
    QVariantMap device;
    device.insert("name", name);
    device.insert("address", address);
    device.insert("rssi", info.rssi());
    m_devices.append(device);
    m_deviceInfos.append(info);
    emit devicesChanged();
    emit deviceDiscovered(name, address, info.rssi());
}

// Handle a BLE scan error and update status
// 处理 BLE 扫描错误并更新状态
void BleManager::onScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    const QString message = error == QBluetoothDeviceDiscoveryAgent::PoweredOffError
        ? "Bluetooth is off" : "BLE scan failed";
    m_scanning = false;
    emit scanningChanged();
    setStatus(message);
    emit errorOccurred(message);
}

// Handle BLE scan completion and report the device count
// 处理 BLE 扫描完成并报告设备数量
void BleManager::onScanFinished()
{
    if (!m_scanning)
        return;
    m_scanning = false;
    emit scanningChanged();
    setStatus(QString("Scan finished, %1 device(s)").arg(m_devices.size()));
}

// Handle BLE controller connection: start service discovery
// 处理 BLE 控制器连接: 开始服务发现
void BleManager::onControllerConnected()
{
    if (!isCurrentControllerSignal())
        return;
    m_phase = Phase::DiscoveringServices;
    setStatus("Discovering Nordic UART service...");
    startStageTimeout(ServiceDiscoveryTimeoutMs, "discovering BLE services");
    m_controller->discoverServices();
}

// Handle BLE controller disconnection: reset transport and update state
// 处理 BLE 控制器断开连接: 重置传输并更新状态
void BleManager::onControllerDisconnected()
{
    if (!isCurrentControllerSignal())
        return;
    resetTransport();
    m_phase = Phase::Idle;
    setConnected(false);
    setConnecting(false);
    setStatus("Disconnected");
}

// Handle a BLE controller error and fail the connection
// 处理 BLE 控制器错误并标记连接失败
void BleManager::onControllerError(QLowEnergyController::Error error)
{
    Q_UNUSED(error);
    if (!isCurrentControllerSignal())
        return;
    failConnection(m_controller->errorString().isEmpty() ? "BLE controller error" : m_controller->errorString());
}

// Record a newly discovered BLE service UUID during service enumeration
// 在服务枚举期间记录新发现的 BLE 服务 UUID
void BleManager::onServiceDiscovered(const QBluetoothUuid &uuid)
{
    if (!isCurrentControllerSignal())
        return;
    m_serviceUuids.append(uuid);
    emit diagnostic("Service discovered: " + uuidText(uuid));
}

// Handle completion of BLE service discovery: locate Nordic UART service
// 处理 BLE 服务发现完成: 定位 Nordic UART 服务
void BleManager::onServiceDiscoveryFinished()
{
    if (!isCurrentControllerSignal())
        return;
    discoverNordicUartService();
}

// Locate and initialise the Nordic UART service object from discovered services
// 从已发现的服务中定位并初始化 Nordic UART 服务对象
void BleManager::discoverNordicUartService()
{
    if (!m_controller)
        return;
    m_stageTimer->stop();
    if (!m_serviceUuids.contains(NordicUartService)) {
        failConnection("Nordic UART service was not found");
        return;
    }
    m_service = m_controller->createServiceObject(NordicUartService, this);
    if (!m_service) {
        failConnection("Could not create Nordic UART service object");
        return;
    }
    connect(m_service, &QLowEnergyService::stateChanged, this, &BleManager::onServiceStateChanged);
    connect(m_service, &QLowEnergyService::errorOccurred, this, &BleManager::onServiceError);
    connect(m_service, &QLowEnergyService::characteristicChanged, this, &BleManager::onCharacteristicChanged);
    connect(m_service, &QLowEnergyService::characteristicWritten, this, &BleManager::onCharacteristicWritten);
    m_phase = Phase::DiscoveringDetails;
    setStatus("Discovering Nordic UART characteristics...");
    startStageTimeout(DetailDiscoveryTimeoutMs, "discovering Nordic UART characteristics");
    m_service->discoverDetails();
}

// Handle Nordic UART service state change: select write characteristic when ready
// 处理 Nordic UART 服务状态变化: 就绪时选择写入特征值
void BleManager::onServiceStateChanged(QLowEnergyService::ServiceState state)
{
    if (!isCurrentServiceSignal() || state != QLowEnergyService::RemoteServiceDiscovered)
        return;
    m_stageTimer->stop();
    if (!selectNordicUartCharacteristics()) {
        failConnection("Nordic UART RX write characteristic was not found");
        return;
    }
    finishConnection();
}

// Handle a BLE service error during connection or while ready
// 处理连接期间或就绪状态下的 BLE 服务错误
void BleManager::onServiceError(QLowEnergyService::ServiceError error)
{
    if (!isCurrentServiceSignal())
        return;
    const QString message = "BLE service error: " + serviceErrorText(error);
    emit diagnostic(message);
    if (m_phase != Phase::Ready)
        failConnection(message);
    else
        emit errorOccurred(message);
}

// Handle a BLE characteristic notification: emit raw bytes and diagnostic
// 处理 BLE 特征值通知: 发出原始字节和诊断信息
void BleManager::onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    if (!isCurrentServiceSignal())
        return;
    emit diagnostic("Notify from uuid=" + uuidText(characteristic.uuid())
                    + " len=" + QString::number(value.size())
                    + " hex=" + KeyboardProtocol::hexDump(value));
    emit rawBytesReceived(value);
    emit rxPacket(KeyboardProtocol::hexDump(value), "BLE_NOTIFY");
}

// Handle acknowledgement of a BLE characteristic write operation
// 处理 BLE 特征值写入操作的确认
void BleManager::onCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    if (!isCurrentServiceSignal())
        return;
    emit diagnostic("Characteristic written uuid=" + uuidText(characteristic.uuid())
                    + " len=" + QString::number(value.size()));
    if (m_writeInFlight && m_inFlightMode == QLowEnergyService::WriteWithResponse
        && characteristic.uuid() == m_writeChar.uuid()) {
        finishCurrentWrite();
    }
}

// Set connecting state and emit signal if changed
// 设置连接中状态并在变化时发出信号
void BleManager::setConnecting(bool connecting)
{
    if (m_connecting == connecting)
        return;
    m_connecting = connecting;
    emit connectingChanged();
}

// Set connected state and emit signal if changed
// 设置已连接状态并在变化时发出信号
void BleManager::setConnected(bool connected)
{
    if (m_connected == connected)
        return;
    m_connected = connected;
    emit connectedChanged();
}

// Set sending state and emit signal if changed
// 设置发送中状态并在变化时发出信号
void BleManager::setSending(bool sending)
{
    if (m_sending == sending)
        return;
    m_sending = sending;
    emit sendingChanged();
}

// Set status text and emit signal if changed
// 设置状态文本并在变化时发出信号
void BleManager::setStatus(const QString &status)
{
    if (m_statusText == status)
        return;
    m_statusText = status;
    emit statusTextChanged();
}

// Stop timers, clear writes, and release the service (and optionally the controller)
// 停止定时器、清除写入队列，并释放服务（可选地释放控制器）
void BleManager::resetTransport(bool deleteController)
{
    m_stageTimer->stop();
    clearWrites();
    m_writeChar = QLowEnergyCharacteristic();
    m_serviceUuids.clear();
    if (m_service) {
        m_service->deleteLater();
        m_service = nullptr;
    }
    if (deleteController && m_controller) {
        m_controller->deleteLater();
        m_controller = nullptr;
    }
}

// Handle a fatal connection error: emit diagnostics, reset state, and disconnect
// 处理致命连接错误: 发出诊断信息、重置状态并断开连接
void BleManager::failConnection(const QString &message)
{
    if (m_phase == Phase::Idle)
        return;
    emit diagnostic(message);
    emit errorOccurred(message);
    m_stageTimer->stop();
    clearWrites();
    setConnected(false);
    setConnecting(false);
    setStatus(message);
    m_phase = Phase::Idle;
    if (m_controller)
        m_controller->disconnectFromDevice();
}

// Start a stage-specific timeout timer for connection progress monitoring
// 启动特定阶段的超时定时器以监控连接进度
void BleManager::startStageTimeout(int milliseconds, const QString &stage)
{
    m_stage = stage;
    m_stageTimer->start(milliseconds);
}

// Scan service characteristics to find the Nordic UART RX write characteristic
// 扫描服务特征值以找到 Nordic UART RX 写入特征值
bool BleManager::selectNordicUartCharacteristics()
{
    m_writeChar = QLowEnergyCharacteristic();
    for (const QLowEnergyCharacteristic &characteristic : m_service->characteristics()) {
        emit diagnostic("Characteristic discovered service=" + uuidText(NordicUartService)
                        + " uuid=" + uuidText(characteristic.uuid())
                        + " props=0x" + QString::number(static_cast<int>(characteristic.properties()), 16));
        if (characteristic.uuid() == NordicUartRx
            && (characteristic.properties() & (QLowEnergyCharacteristic::Write
                                               | QLowEnergyCharacteristic::WriteNoResponse))) {
            m_writeChar = characteristic;
        }
    }
    return m_writeChar.isValid();
}

// Transition to Ready phase after successful BLE connection
// 成功 BLE 连接后转换到就绪阶段
void BleManager::finishConnection()
{
    m_phase = Phase::Ready;
    setConnecting(false);
    setConnected(true);
    setStatus("Connected " + m_currentDeviceName);
    emit diagnostic("Nordic UART write transport ready; CCCD subscription is intentionally skipped");
}

// Split data into conservative-sized chunks and enqueue them for BLE writing
// 将数据分割为保守大小的块并加入 BLE 写入队列
void BleManager::enqueueWrite(const QByteArray &data, const QString &description)
{
    for (int offset = 0; offset < data.size(); offset += ConservativePayloadSize)
        m_pendingWrites.enqueue({data.mid(offset, ConservativePayloadSize), description});
    setSending(true);
    pumpWrites();
}

// Dequeue and write the next pending chunk to the BLE characteristic
// 从队列中取出下一个待写块并写入 BLE 特征值
void BleManager::pumpWrites()
{
    if (m_writeInFlight || m_pendingWrites.isEmpty() || !m_service || !m_writeChar.isValid())
        return;
    const PendingWrite pending = m_pendingWrites.dequeue();
    m_inFlightWrite = pending.bytes;
    m_writeInFlight = true;
    // WinRT can acknowledge WriteWithoutResponse locally while never putting the
    // ATT command on air. Prefer a request/response write whenever the peer
    // exposes it, so the ESP32's GATT response is the delivery acknowledgement.
    m_inFlightMode = (m_writeChar.properties() & QLowEnergyCharacteristic::Write)
        ? QLowEnergyService::WriteWithResponse : QLowEnergyService::WriteWithoutResponse;
    const QString mode = m_inFlightMode == QLowEnergyService::WriteWithoutResponse
        ? "without-response" : "with-response";
    emit diagnostic("Writing Nordic UART RX uuid=" + uuidText(m_writeChar.uuid())
                    + " mode=" + mode + " len=" + QString::number(m_inFlightWrite.size())
                    + " hex=" + KeyboardProtocol::hexDump(m_inFlightWrite));
    emit txPacket(KeyboardProtocol::hexDump(m_inFlightWrite), pending.description);
    m_service->writeCharacteristic(m_writeChar, m_inFlightWrite, m_inFlightMode);
    if (m_inFlightMode == QLowEnergyService::WriteWithoutResponse) {
        m_writeTimer->start(WriteWithoutResponsePaceMs);
    } else {
        m_writeTimer->start(WriteResponseTimeoutMs);
    }
}

// Acknowledge completion of the current in-flight write and pump the next
// 确认当前正在传输的写操作已完成并处理下一笔
void BleManager::finishCurrentWrite()
{
    if (!m_writeInFlight)
        return;
    m_writeTimer->stop();
    m_writeInFlight = false;
    m_inFlightWrite.clear();
    if (m_pendingWrites.isEmpty())
        setSending(false);
    pumpWrites();
}

// Clear all pending writes and stop the write timer
// 清除所有待写数据并停止写入定时器
void BleManager::clearWrites()
{
    m_pendingWrites.clear();
    m_inFlightWrite.clear();
    m_writeInFlight = false;
    if (m_writeTimer)
        m_writeTimer->stop();
    setSending(false);
}

// Check that the signal sender is the current BLE controller
// 检查信号发送者是否为当前 BLE 控制器
bool BleManager::isCurrentControllerSignal() const
{
    return sender() == m_controller;
}

// Check that the signal sender is the current BLE service
// 检查信号发送者是否为当前 BLE 服务
bool BleManager::isCurrentServiceSignal() const
{
    return sender() == m_service;
}

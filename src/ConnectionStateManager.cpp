#include "ConnectionStateManager.h"
#include "AutoClickerManager.h"
#include "BleManager.h"
#include "CaptureManager.h"
#include "HudpTransport.h"
#include "MouseInputManager.h"
#include "SerialManager.h"
#include "SettingsManager.h"
#include "TransportSelector.h"
#include <QTimer>

// Construct and wire up all transport manager signals to aggregate connection state.
// 构造并连接所有传输管理器信号以聚合连接状态。
ConnectionStateManager::ConnectionStateManager(SerialManager *serial, BleManager *ble, HudpTransport *hudp,
                                                 SettingsManager *settings, CaptureManager *capture,
                                                 MouseInputManager *mouse, AutoClickerManager *clicker, TransportSelector *selector, QObject *parent)
    : QObject(parent), m_serial(serial), m_ble(ble), m_hudp(hudp), m_settings(settings), m_capture(capture), m_mouse(mouse), m_clicker(clicker), m_selector(selector), m_timer(new QTimer(this))
{
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, [this] { ++m_connectedSeconds; emit connectedTimeChanged(); });
    connect(m_serial, &SerialManager::currentPortChanged, this, [this] { m_uartPort = m_serial->currentPort(); emit uartPortChanged(); });
    connect(m_serial, &SerialManager::statusTextChanged, this, [this] { m_uartStatus = m_serial->statusText(); emit uartStatusChanged(); });
    connect(m_serial, &SerialManager::errorOccurred, this, &ConnectionStateManager::setConnectionError);
    connect(m_serial, &SerialManager::connectedChanged, this, [this] {
        m_uartConnected = m_serial->connected(); m_uartStatus = m_serial->statusText();
        emit uartConnectedChanged(); emit uartConnectingChanged(); emit uartStatusChanged(); updateConnected();
        if (m_uartConnected) { clearConnectionError(); startConnectedTimer(); setPairedDevice(m_serial->currentPort(), "UART"); m_settings->rememberConnection("UART", m_uartPort, m_uartBaudrate); }
        else if (!m_bleConnected && !m_hudp->connected()) { m_settings->markConnectionDisconnected("UART"); stopConnectedTimer(); clearPairedDevice(); }
    });
    connect(m_ble, &BleManager::scanningChanged, this, [this] { m_bleScanning = m_ble->isScanning(); emit bleScanningChanged(); });
    connect(m_ble, &BleManager::connectingChanged, this, [this] { m_bleConnecting = m_ble->isConnecting(); emit bleConnectingChanged(); });
    connect(m_ble, &BleManager::statusTextChanged, this, [this] { m_bleStatus = m_ble->statusText(); emit bleStatusChanged(); });
    connect(m_ble, &BleManager::errorOccurred, this, &ConnectionStateManager::setConnectionError);
    connect(m_ble, &BleManager::devicesChanged, this, &ConnectionStateManager::restoreBleWhenFound);
    connect(m_ble, &BleManager::connectedChanged, this, [this] {
        m_bleConnected = m_ble->isConnected(); emit bleConnectedChanged(); updateConnected();
        if (m_bleConnected) { m_pendingBleRestoreAddress.clear(); clearConnectionError(); startConnectedTimer(); setPairedDevice(m_ble->currentDeviceName(), m_ble->currentDeviceAddress()); m_settings->rememberConnection("BLE", m_pairedDeviceAddress); }
        else if (!m_uartConnected && !m_hudp->connected()) { m_settings->markConnectionDisconnected("BLE"); stopConnectedTimer(); clearPairedDevice(); }
    });
    connect(m_hudp, &HudpTransport::connectedChanged, this, [this] { updateConnected(); if (!m_hudp->connected() && !m_uartConnected && !m_bleConnected) protectOnNoTransport(); });
    connect(m_selector, &TransportSelector::activeTransportChanged, this, [this] {
        if (!m_selector->activeTransport())
            protectOnNoTransport();
    });
    connect(m_settings, &SettingsManager::restoreUartRequested, this, &ConnectionStateManager::connectUart);
    connect(m_settings, &SettingsManager::restoreBleRequested, this, [this](const QString &address) { m_pendingBleRestoreAddress = address; m_ble->startScan(); emit diagnostic(0, "Settings", "Scanning to restore BLE connection: " + address); });
    connect(m_settings, &SettingsManager::restoreHudpRequested, this, [this](const QString &target, int port) { connectHudp(target, port, true); });
}

// Return the connected time as a formatted "MM:SS" string.
// 返回格式化为 "MM:SS" 的连接时间字符串。
QString ConnectionStateManager::connectedTimeStr() const { return QString("%1:%2").arg(m_connectedSeconds / 60, 2, 10, QChar('0')).arg(m_connectedSeconds % 60, 2, 10, QChar('0')); }
// Connect to a UART serial port at the given baud rate.
// 以指定波特率连接 UART 串口。
void ConnectionStateManager::connectUart(const QString &port, int baudrate) { m_uartBaudrate = baudrate; emit uartBaudrateChanged(); clearConnectionError(); m_serial->connectToPort(port, baudrate); }
// Disconnect from the current UART serial port.
// 断开当前 UART 串口连接。
void ConnectionStateManager::disconnectUart() { m_serial->disconnectPort(); }
// Start a BLE device scan.
// 启动 BLE 设备扫描。
void ConnectionStateManager::startBleScan() { clearConnectionError(); m_ble->startScan(); }
// Stop the current BLE device scan.
// 停止当前 BLE 设备扫描。
void ConnectionStateManager::stopBleScan() { m_ble->stopScan(); }
// Connect to a BLE device by address.
// 通过地址连接 BLE 设备。
void ConnectionStateManager::connectBle(const QString &address) { clearConnectionError(); m_ble->connectToDevice(address); }
// Disconnect from the current BLE device.
// 断开当前 BLE 设备连接。
void ConnectionStateManager::disconnectBle() { m_ble->disconnectDevice(); }
// Connect to a HUDP endpoint; restoring flag indicates a settings restore.
// 连接 HUDP 端点；restoring 标志表示设置恢复。
void ConnectionStateManager::connectHudp(const QString &target, int port, bool restoring) { m_hudpTarget = target; m_hudpPort = port; m_hudpConnecting = true; emit hudpTargetChanged(); emit hudpPortChanged(); emit hudpConnectingChanged(); m_hudp->connectToTarget(target, quint16(port)); m_hudpConnecting = false; m_hudpStatus = m_hudp->connected() ? (restoring ? "Restored" : "HID UDP ready") : "HID UDP endpoint error"; emit hudpConnectingChanged(); emit hudpStatusChanged(); updateConnected(); if (m_hudp->connected()) { clearConnectionError(); startConnectedTimer(); setPairedDevice(target, "HUDP"); m_settings->rememberConnection("HUDP", target, port); } }
// Disconnect from the current HUDP endpoint.
// 断开当前 HUDP 端点连接。
void ConnectionStateManager::disconnectHudp() { m_hudp->disconnectFromTarget(); m_settings->markConnectionDisconnected("HUDP"); m_hudpStatus = "Disconnected"; emit hudpStatusChanged(); updateConnected(); if (!m_connected) { stopConnectedTimer(); clearPairedDevice(); protectOnNoTransport(); } }
// Enable or disable remembering the last successful connection for auto-restore.
// 启用或禁用记住上次成功连接以自动恢复。
void ConnectionStateManager::setRememberLastConnection(bool enabled) { m_settings->setRememberLastConnection(enabled); if (!enabled) m_settings->setLastConnection(m_settings->lastConnectionTransport(), false); else if (m_bleConnected) m_settings->rememberConnection("BLE", m_pairedDeviceAddress); else if (m_uartConnected) m_settings->rememberConnection("UART", m_uartPort, m_uartBaudrate); else if (m_hudp->connected()) m_settings->rememberConnection("HUDP", m_hudpTarget, m_hudpPort); }
// Clear the current connection error string.
// 清除当前连接错误字符串。
void ConnectionStateManager::clearConnectionError() { if (m_connectionError.isEmpty()) return; m_connectionError.clear(); emit connectionErrorChanged(); }
// Set a new connection error string.
// 设置新的连接错误字符串。
void ConnectionStateManager::setConnectionError(const QString &error) { if (m_connectionError == error) return; m_connectionError = error; emit connectionErrorChanged(); }
// Handle an output error from any transport and update state accordingly.
// 处理来自任何传输的输出错误并相应更新状态。
void ConnectionStateManager::handleOutputError(const QString &error) { setConnectionError(error); protectOnNoTransport(); }
// Recalculate the aggregate connected state from individual transport states.
// 根据各传输状态重新计算聚合连接状态。
void ConnectionStateManager::updateConnected() { const bool value = m_uartConnected || m_bleConnected || m_hudp->connected(); if (m_connected == value) return; m_connected = value; emit connectedChanged(); if (!value) protectOnNoTransport(); }
// Store the paired device name and address after a successful connection.
// 在成功连接后存储配对设备的名称和地址。
void ConnectionStateManager::setPairedDevice(const QString &name, const QString &address) { m_pairedDeviceName = name; m_pairedDeviceAddress = address; emit pairedDeviceNameChanged(); emit pairedDeviceAddressChanged(); }
// Clear the stored paired device name and address.
// 清除存储的配对设备名称和地址。
void ConnectionStateManager::clearPairedDevice() { m_pairedDeviceName.clear(); m_pairedDeviceAddress.clear(); emit pairedDeviceNameChanged(); emit pairedDeviceAddressChanged(); }
// Start the connected-duration timer from zero.
// 从零开始启动连接持续时间计时器。
void ConnectionStateManager::startConnectedTimer() { m_connectedSeconds = 0; emit connectedTimeChanged(); m_timer->start(); }
// Stop the connected-duration timer and reset the counter.
// 停止连接持续时间计时器并重置计数器。
void ConnectionStateManager::stopConnectedTimer() { m_timer->stop(); m_connectedSeconds = 0; emit connectedTimeChanged(); }
// Safety measure: stop auto-clicker and mouse capture when no transport is available.
// 安全措施：在没有可用传输时停止自动点击器和鼠标捕获。
void ConnectionStateManager::protectOnNoTransport() { if (m_clicker && m_clicker->running()) { m_clicker->stop(); emit diagnostic(2, "AutoClicker", "Stopped: no active transport"); } if (m_capture) m_capture->setMouseCaptureListening(false); if (m_mouse) m_mouse->resetState(); }
// Attempt to restore a BLE connection when the target device is found during scan.
// 在扫描期间找到目标设备时尝试恢复 BLE 连接。
void ConnectionStateManager::restoreBleWhenFound() { if (m_pendingBleRestoreAddress.isEmpty()) return; for (const QVariant &item : m_ble->devices()) { if (item.toMap().value("address").toString() == m_pendingBleRestoreAddress) { const QString address = m_pendingBleRestoreAddress; m_pendingBleRestoreAddress.clear(); m_ble->connectToDevice(address); emit diagnostic(0, "Settings", "Restoring BLE connection: " + address); break; } } }

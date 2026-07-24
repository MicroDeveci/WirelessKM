#pragma once

#include <QObject>
#include <QString>

class SerialManager;
class BleManager;
class HudpTransport;
class SettingsManager;
class CaptureManager;
class MouseInputManager;
class AutoClickerManager;
class TransportSelector;

// Owns the application-level connection view and recovery workflow.  It keeps
// transport-specific managers independent while presenting one coherent state.
class ConnectionStateManager : public QObject
{
    Q_OBJECT
public:
    // Construct with references to all transport managers and settings.
    // 使用所有传输管理器和设置的引用进行构造。
    ConnectionStateManager(SerialManager *serial, BleManager *ble, HudpTransport *hudp,
                           SettingsManager *settings, CaptureManager *capture,
                           MouseInputManager *mouse, AutoClickerManager *clicker, TransportSelector *selector,
                           QObject *parent = nullptr);

    // Whether any transport is currently connected.
    // 是否有任何传输当前已连接。
    bool connected() const { return m_connected; }
    // Return the HUDP target host string.
    // 返回 HUDP 目标主机字符串。
    QString hudpTarget() const { return m_hudpTarget; }
    // Return the HUDP target port.
    // 返回 HUDP 目标端口。
    int hudpPort() const { return m_hudpPort; }
    // Whether a HUDP connection attempt is in progress.
    // HUDP 连接尝试是否正在进行中。
    bool hudpConnecting() const { return m_hudpConnecting; }
    // Return the HUDP connection status text.
    // 返回 HUDP 连接状态文本。
    QString hudpStatus() const { return m_hudpStatus; }
    // Whether a BLE scan is in progress.
    // BLE 扫描是否正在进行中。
    bool bleScanning() const { return m_bleScanning; }
    // Whether a BLE connection attempt is in progress.
    // BLE 连接尝试是否正在进行中。
    bool bleConnecting() const { return m_bleConnecting; }
    // Whether BLE is currently connected.
    // BLE 是否当前已连接。
    bool bleConnected() const { return m_bleConnected; }
    // Return the BLE connection status text.
    // 返回 BLE 连接状态文本。
    QString bleStatus() const { return m_bleStatus; }
    // Return the UART port name.
    // 返回 UART 端口名称。
    QString uartPort() const { return m_uartPort; }
    // Return the UART baud rate.
    // 返回 UART 波特率。
    int uartBaudrate() const { return m_uartBaudrate; }
    // Whether UART is currently connected.
    // UART 是否当前已连接。
    bool uartConnected() const { return m_uartConnected; }
    // Return the UART connection status text.
    // 返回 UART 连接状态文本。
    QString uartStatus() const { return m_uartStatus; }
    // Return the current connection error string.
    // 返回当前连接错误字符串。
    QString connectionError() const { return m_connectionError; }
    // Return the name of the last paired device.
    // 返回上次配对设备的名称。
    QString pairedDeviceName() const { return m_pairedDeviceName; }
    // Return the address of the last paired device.
    // 返回上次配对设备的地址。
    QString pairedDeviceAddress() const { return m_pairedDeviceAddress; }
    // Return the number of seconds since connection was established.
    // 返回自连接建立以来的秒数。
    int connectedTime() const { return m_connectedSeconds; }
    // Return the connected time as a formatted "MM:SS" string.
    // 返回格式化为 "MM:SS" 的连接时间字符串。
    QString connectedTimeStr() const;

    // Connect to a UART serial port at the given baud rate.
    // 以指定波特率连接 UART 串口。
    void connectUart(const QString &port, int baudrate);
    // Disconnect from the current UART serial port.
    // 断开当前 UART 串口连接。
    void disconnectUart();
    // Start a BLE device scan.
    // 启动 BLE 设备扫描。
    void startBleScan();
    // Stop the current BLE device scan.
    // 停止当前 BLE 设备扫描。
    void stopBleScan();
    // Connect to a BLE device by address.
    // 通过地址连接 BLE 设备。
    void connectBle(const QString &address);
    // Disconnect from the current BLE device.
    // 断开当前 BLE 设备连接。
    void disconnectBle();
    // Connect to a HUDP endpoint; restoring flag indicates a settings restore.
    // 连接 HUDP 端点；restoring 标志表示设置恢复。
    void connectHudp(const QString &target, int port, bool restoring = false);
    // Disconnect from the current HUDP endpoint.
    // 断开当前 HUDP 端点连接。
    void disconnectHudp();
    // Enable or disable remembering the last successful connection for auto-restore.
    // 启用或禁用记住上次成功连接以自动恢复。
    void setRememberLastConnection(bool enabled);
    // Clear the current connection error string.
    // 清除当前连接错误字符串。
    void clearConnectionError();
    // Set a new connection error string.
    // 设置新的连接错误字符串。
    void setConnectionError(const QString &error);
    // Handle an output error from any transport and update state accordingly.
    // 处理来自任何传输的输出错误并相应更新状态。
    void handleOutputError(const QString &error);

signals:
    void connectedChanged(); void hudpTargetChanged(); void hudpPortChanged();
    void hudpConnectingChanged(); void hudpStatusChanged(); void bleScanningChanged();
    void bleConnectingChanged(); void bleConnectedChanged(); void bleStatusChanged();
    void uartPortChanged(); void uartBaudrateChanged(); void uartConnectedChanged();
    void uartConnectingChanged(); void uartStatusChanged(); void connectionErrorChanged();
    void pairedDeviceNameChanged(); void pairedDeviceAddressChanged(); void connectedTimeChanged();
    void diagnostic(int level, const QString &module, const QString &message);

private:
    // Recalculate the aggregate connected state from individual transport states.
    // 根据各传输状态重新计算聚合连接状态。
    void updateConnected();
    // Store the paired device name and address after a successful connection.
    // 在成功连接后存储配对设备的名称和地址。
    void setPairedDevice(const QString &name, const QString &address);
    // Clear the stored paired device name and address.
    // 清除存储的配对设备名称和地址。
    void clearPairedDevice();
    // Start the connected-duration timer from zero.
    // 从零开始启动连接持续时间计时器。
    void startConnectedTimer();
    // Stop the connected-duration timer and reset the counter.
    // 停止连接持续时间计时器并重置计数器。
    void stopConnectedTimer();
    // Safety measure: stop auto-clicker and mouse capture when no transport is available.
    // 安全措施：在没有可用传输时停止自动点击器和鼠标捕获。
    void protectOnNoTransport();
    // Attempt to restore a BLE connection when the target device is found during scan.
    // 在扫描期间找到目标设备时尝试恢复 BLE 连接。
    void restoreBleWhenFound();

    SerialManager *m_serial; BleManager *m_ble; HudpTransport *m_hudp;
    SettingsManager *m_settings; CaptureManager *m_capture; MouseInputManager *m_mouse;
    AutoClickerManager *m_clicker;
    TransportSelector *m_selector;
    QTimer *m_timer;
    bool m_connected = false, m_hudpConnecting = false, m_bleScanning = false, m_bleConnecting = false, m_bleConnected = false, m_uartConnected = false;
    QString m_hudpTarget, m_hudpStatus, m_bleStatus, m_uartPort, m_uartStatus, m_connectionError, m_pairedDeviceName, m_pairedDeviceAddress, m_pendingBleRestoreAddress;
    int m_hudpPort = 45820, m_uartBaudrate = 115200, m_connectedSeconds = 0;
};

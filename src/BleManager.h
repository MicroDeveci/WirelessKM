#pragma once

#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QQueue>
#include <QTimer>
#include <QVariantList>

class BleSystemRouter;

// Desktop-side transport for the ESP32 Nordic UART Service.  It deliberately
// treats the RX write characteristic as the transport readiness boundary:
// Windows can cache or reject CCCD writes even though the write path works.
class BleManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool scanning READ isScanning NOTIFY scanningChanged)
    Q_PROPERTY(bool connecting READ isConnecting NOTIFY connectingChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool sending READ isSending NOTIFY sendingChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString currentDeviceName READ currentDeviceName NOTIFY currentDeviceChanged)
    Q_PROPERTY(QString currentDeviceAddress READ currentDeviceAddress NOTIFY currentDeviceChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)

public:
    // Constructor: initialise BLE discovery agent, timers, and platform transport
    // 构造函数: 初始化 BLE 发现代理、定时器和平台传输
    explicit BleManager(QObject *parent = nullptr);

    // Check if BLE scanning is in progress
    // 检查 BLE 扫描是否正在进行中
    bool isScanning() const { return m_scanning; }
    // Check if a BLE connection is being established
    // 检查是否正在建立 BLE 连接
    bool isConnecting() const { return m_connecting; }
    // Check if BLE is currently connected
    // 检查 BLE 是否已连接
    bool isConnected() const { return m_connected; }
    // Check if a write operation is in progress
    // 检查写操作是否正在进行中
    bool isSending() const { return m_sending; }
    // Get the current status text
    // 获取当前状态文本
    QString statusText() const { return m_statusText; }
    // Get the name of the currently connected device
    // 获取当前连接设备的名称
    QString currentDeviceName() const { return m_currentDeviceName; }
    // Get the Bluetooth address of the currently connected device
    // 获取当前连接设备的蓝牙地址
    QString currentDeviceAddress() const { return m_currentDeviceAddress; }
    // Get the list of discovered BLE devices
    // 获取已发现的 BLE 设备列表
    QVariantList devices() const { return m_devices; }

    // Start scanning for nearby BLE devices
    // 开始扫描附近的 BLE 设备
    Q_INVOKABLE void startScan();
    // Stop the current BLE scan
    // 停止当前 BLE 扫描
    Q_INVOKABLE void stopScan();
    // Connect to a BLE device by its Bluetooth address
    // 通过蓝牙地址连接到 BLE 设备
    Q_INVOKABLE void connectToDevice(const QString &address);
    // Disconnect from the current BLE device
    // 断开与当前 BLE 设备的连接
    Q_INVOKABLE void disconnectDevice();
    // Write a data frame to the Nordic UART RX characteristic
    // 向 Nordic UART RX 特征值写入一帧数据
    void writeFrame(const QByteArray &frame);

signals:
    void scanningChanged();
    void connectingChanged();
    void connectedChanged();
    void sendingChanged();
    void statusTextChanged();
    void currentDeviceChanged();
    void devicesChanged();
    void deviceDiscovered(const QString &name, const QString &address, int rssi);
    void txPacket(const QString &hex, const QString &desc);
    void rxPacket(const QString &hex, const QString &desc);
    void rawBytesReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void diagnostic(const QString &message);

private slots:
    // Handle a newly discovered BLE device during scanning
    // 处理扫描期间新发现的 BLE 设备
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    // Handle a BLE scanning error
    // 处理 BLE 扫描错误
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);
    // Handle BLE scan completion
    // 处理 BLE 扫描完成
    void onScanFinished();
    // Handle BLE controller connection event
    // 处理 BLE 控制器连接事件
    void onControllerConnected();
    // Handle BLE controller disconnection event
    // 处理 BLE 控制器断开连接事件
    void onControllerDisconnected();
    // Handle a BLE controller error
    // 处理 BLE 控制器错误
    void onControllerError(QLowEnergyController::Error error);
    // Handle discovery of a new BLE service UUID
    // 处理新发现的 BLE 服务 UUID
    void onServiceDiscovered(const QBluetoothUuid &uuid);
    // Handle completion of BLE service discovery phase
    // 处理 BLE 服务发现阶段完成
    void onServiceDiscoveryFinished();
    // Handle Nordic UART service state change
    // 处理 Nordic UART 服务状态变化
    void onServiceStateChanged(QLowEnergyService::ServiceState state);
    // Handle a BLE service error
    // 处理 BLE 服务错误
    void onServiceError(QLowEnergyService::ServiceError error);
    // Handle a BLE characteristic value notification
    // 处理 BLE 特征值通知
    void onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    // Handle acknowledgement of a BLE characteristic write
    // 处理 BLE 特征值写入确认
    void onCharacteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);

private:
    enum class Phase { Idle, Connecting, DiscoveringServices, DiscoveringDetails, Ready, Disconnecting };
    struct PendingWrite { QByteArray bytes; QString description; };

    // Set connecting state and emit signal if changed
    // 设置连接中状态并在变化时发出信号
    void setConnecting(bool connecting);
    // Set connected state and emit signal if changed
    // 设置已连接状态并在变化时发出信号
    void setConnected(bool connected);
    // Set sending state and emit signal if changed
    // 设置发送中状态并在变化时发出信号
    void setSending(bool sending);
    // Set status text and emit signal if changed
    // 设置状态文本并在变化时发出信号
    void setStatus(const QString &status);
    // Release BLE service/controller resources and reset transport state
    // 释放 BLE 服务/控制器资源并重置传输状态
    void resetTransport(bool deleteController = true);
    // Handle a fatal connection error: disconnect and reset state
    // 处理致命连接错误: 断开连接并重置状态
    void failConnection(const QString &message);
    // Start a stage-specific timeout timer
    // 启动特定阶段的超时定时器
    void startStageTimeout(int milliseconds, const QString &stage);
    // Locate the Nordic UART service from discovered service UUIDs
    // 从已发现的服务 UUID 中定位 Nordic UART 服务
    void discoverNordicUartService();
    // Select the Nordic UART RX write characteristic from the service
    // 从服务中选择 Nordic UART RX 写入特征值
    bool selectNordicUartCharacteristics();
    // Transition to the Ready phase after successful connection
    // 成功连接后转换到就绪阶段
    void finishConnection();
    // Split data into conservative chunks and enqueue for writing
    // 将数据分割为保守大小的块并加入写入队列
    void enqueueWrite(const QByteArray &data, const QString &description);
    // Dequeue and write the next pending chunk to the BLE characteristic
    // 从队列中取出下一个待写块并写入 BLE 特征值
    void pumpWrites();
    // Acknowledge completion of the current in-flight write
    // 确认当前正在传输的写操作已完成
    void finishCurrentWrite();
    // Clear all pending writes and reset the write queue
    // 清除所有待写数据并重置写入队列
    void clearWrites();
    // Check that the signal sender is the current BLE controller
    // 检查信号发送者是否为当前 BLE 控制器
    bool isCurrentControllerSignal() const;
    // Check that the signal sender is the current BLE service
    // 检查信号发送者是否为当前 BLE 服务
    bool isCurrentServiceSignal() const;

    QBluetoothDeviceDiscoveryAgent *m_agent = nullptr;
    QLowEnergyController *m_controller = nullptr;
    QLowEnergyService *m_service = nullptr;
    QTimer *m_stageTimer = nullptr;
    QTimer *m_writeTimer = nullptr;
    bool m_scanning = false;
    bool m_connecting = false;
    bool m_connected = false;
    bool m_sending = false;
    Phase m_phase = Phase::Idle;
    QString m_stage;
    QString m_statusText;
    QString m_currentDeviceName;
    QString m_currentDeviceAddress;
    QVariantList m_devices;
    QList<QBluetoothDeviceInfo> m_deviceInfos;
    QList<QBluetoothUuid> m_serviceUuids;
    QLowEnergyCharacteristic m_writeChar;
    QQueue<PendingWrite> m_pendingWrites;
    QByteArray m_inFlightWrite;
    QLowEnergyService::WriteMode m_inFlightMode = QLowEnergyService::WriteWithoutResponse;
    bool m_writeInFlight = false;
    BleSystemRouter *m_systemRouter = nullptr;
};

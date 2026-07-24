#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>

// Windows-only GATT transport. This bypasses QtBluetooth's WinRT backend for
// operations that must reach the ATT layer.
class WinRtBleTransport final : public QObject
{
    Q_OBJECT
public:
    // Constructor: initialise WinRT COM apartment for BLE operations
    // 构造函数: 初始化 WinRT COM 单元以进行 BLE 操作
    explicit WinRtBleTransport(QObject *parent = nullptr);
    // Destructor: release native GATT objects
    // 析构函数: 释放原生 GATT 对象
    ~WinRtBleTransport() override;

    // Check if a BLE connection is currently being established
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

    // Connect to a BLE device by its Bluetooth address using Windows GATT API
    // 通过蓝牙地址使用 Windows GATT API 连接到 BLE 设备
    void connectToAddress(const QString &address, const QString &name);
    // Disconnect from the current BLE device and release resources
    // 断开与当前 BLE 设备的连接并释放资源
    void disconnectDevice();
    // Write a frame to the Nordic UART RX characteristic via GATT
    // 通过 GATT 向 Nordic UART RX 特征值写入一帧数据
    void writeFrame(const QByteArray &frame);

signals:
    void connectingChanged();
    void connectedChanged();
    void sendingChanged();
    void statusTextChanged();
    void errorOccurred(const QString &error);
    void diagnostic(const QString &message);
    void txPacket(const QString &hex, const QString &desc);

private:
    // Set connecting state and emit signal if changed
    // 设置连接中状态并在变化时发出信号
    void setConnecting(bool value);
    // Set connected state and emit signal if changed
    // 设置已连接状态并在变化时发出信号
    void setConnected(bool value);
    // Set sending state and emit signal if changed
    // 设置发送中状态并在变化时发出信号
    void setSending(bool value);
    // Set status text and emit signal if changed
    // 设置状态文本并在变化时发出信号
    void setStatus(const QString &value);
    // Handle a fatal error: release resources and emit error signals
    // 处理致命错误: 释放资源并发出错误信号
    void fail(const QString &message);
    // Delete and clear the native WinRT GATT state
    // 删除并清空原生 WinRT GATT 状态
    void releaseNativeObjects();

    bool m_connecting = false;
    bool m_connected = false;
    bool m_sending = false;
    QString m_statusText;
    QString m_deviceName;
    void *m_nativeState = nullptr;
};

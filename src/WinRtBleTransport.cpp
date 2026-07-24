#include "WinRtBleTransport.h"

#include "KeyboardProtocol.h"

#include <QBluetoothUuid>
#include <QMetaObject>
#include <QUuid>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace winrt::Windows::Storage::Streams;

namespace {
const winrt::guid NusService{L"6e400001-b5a3-f393-e0a9-e50e24dcca9e"};
const winrt::guid NusRx{L"6e400002-b5a3-f393-e0a9-e50e24dcca9e"};

struct NativeState {
    BluetoothLEDevice device = nullptr;
    GattDeviceService service = nullptr;
    GattCharacteristic writeCharacteristic = nullptr;
};

// Convert a Bluetooth address string (e.g. "AA:BB:CC:DD:EE:FF") to a uint64
// 将蓝牙地址字符串（如 "AA:BB:CC:DD:EE:FF"）转换为 uint64
quint64 bluetoothAddress(const QString &address)
{
    QString value = address;
    value.remove(':');
    value.remove('-');
    bool ok = false;
    const quint64 result = value.toULongLong(&ok, 16);
    return ok && value.size() == 12 ? result : 0;
}
}

// Constructor: initialise the WinRT single-threaded apartment for BLE
// 构造函数: 初始化 WinRT 单线程单元以进行 BLE 操作
WinRtBleTransport::WinRtBleTransport(QObject *parent)
    : QObject(parent)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);
}

// Destructor: release native GATT objects
// 析构函数: 释放原生 GATT 对象
WinRtBleTransport::~WinRtBleTransport()
{
    releaseNativeObjects();
}

// Connect to a BLE device by address using async Windows GATT API chain
// 通过地址使用异步 Windows GATT API 链连接到 BLE 设备
void WinRtBleTransport::connectToAddress(const QString &address, const QString &name)
{
    const quint64 nativeAddress = bluetoothAddress(address);
    if (!nativeAddress) {
        fail("Invalid BLE address: " + address);
        return;
    }
    releaseNativeObjects();
    m_deviceName = name;
    setConnecting(true);
    setStatus("Opening Windows GATT device...");

    // WinRT asserts when .get() is used on the Qt GUI STA thread. Chain the
    // operations through completion handlers instead of blocking the event loop.
    BluetoothLEDevice::FromBluetoothAddressAsync(nativeAddress).Completed(
        [this](auto const &deviceOp, winrt::Windows::Foundation::AsyncStatus status) {
            if (status != winrt::Windows::Foundation::AsyncStatus::Completed) {
                QMetaObject::invokeMethod(this, [this] { fail("Windows could not open BLE device"); }, Qt::QueuedConnection);
                return;
            }
            try {
                auto device = deviceOp.GetResults();
                QMetaObject::invokeMethod(this, [this] { setStatus("Opening Nordic UART service (uncached)..."); }, Qt::QueuedConnection);
                device.GetGattServicesForUuidAsync(NusService, BluetoothCacheMode::Uncached).Completed(
                    [this, device](auto const &serviceOp, winrt::Windows::Foundation::AsyncStatus serviceStatus) {
                        if (serviceStatus != winrt::Windows::Foundation::AsyncStatus::Completed) {
                            QMetaObject::invokeMethod(this, [this] { fail("Windows GATT service lookup did not complete"); }, Qt::QueuedConnection);
                            return;
                        }
                        try {
                            const auto services = serviceOp.GetResults();
                            if (services.Status() != GattCommunicationStatus::Success || services.Services().Size() != 1) {
                                QMetaObject::invokeMethod(this, [this] { fail("Windows GATT service lookup failed"); }, Qt::QueuedConnection);
                                return;
                            }
                            auto service = services.Services().GetAt(0);
                            QMetaObject::invokeMethod(this, [this] { setStatus("Opening Nordic UART RX characteristic (uncached)..."); }, Qt::QueuedConnection);
                            service.GetCharacteristicsForUuidAsync(NusRx, BluetoothCacheMode::Uncached).Completed(
                                [this, device, service](auto const &charOp, winrt::Windows::Foundation::AsyncStatus charStatus) {
                                    if (charStatus != winrt::Windows::Foundation::AsyncStatus::Completed) {
                                        QMetaObject::invokeMethod(this, [this] { fail("Windows GATT characteristic lookup did not complete"); }, Qt::QueuedConnection);
                                        return;
                                    }
                                    try {
                                        const auto characteristics = charOp.GetResults();
                                        if (characteristics.Status() != GattCommunicationStatus::Success || characteristics.Characteristics().Size() != 1) {
                                            QMetaObject::invokeMethod(this, [this] { fail("Windows GATT characteristic lookup failed"); }, Qt::QueuedConnection);
                                            return;
                                        }
                                        auto characteristic = characteristics.Characteristics().GetAt(0);
                                        const auto writeProperties = characteristic.CharacteristicProperties();
                                        if ((writeProperties & (GattCharacteristicProperties::Write
                                                                | GattCharacteristicProperties::WriteWithoutResponse))
                                            == GattCharacteristicProperties::None) {
                                            QMetaObject::invokeMethod(this, [this] { fail("Nordic UART RX is not writable"); }, Qt::QueuedConnection);
                                            return;
                                        }
                                        QMetaObject::invokeMethod(this, [this, device, service, characteristic] {
                                            releaseNativeObjects();
                                            m_nativeState = new NativeState{device, service, characteristic};
                                            setConnecting(false);
                                            setConnected(true);
                                            setStatus("Connected " + m_deviceName + " (Windows GATT)");
                                            emit diagnostic("Windows WinRT NUS transport ready; service and characteristic resolved uncached");
                                        }, Qt::QueuedConnection);
                                    } catch (const winrt::hresult_error &error) {
                                        const QString message = QString::fromWCharArray(error.message().c_str());
                                        QMetaObject::invokeMethod(this, [this, message] { fail("Windows GATT characteristic error: " + message); }, Qt::QueuedConnection);
                                    }
                                });
                        } catch (const winrt::hresult_error &error) {
                            const QString message = QString::fromWCharArray(error.message().c_str());
                            QMetaObject::invokeMethod(this, [this, message] { fail("Windows GATT service error: " + message); }, Qt::QueuedConnection);
                        }
                    });
            } catch (const winrt::hresult_error &error) {
                const QString message = QString::fromWCharArray(error.message().c_str());
                QMetaObject::invokeMethod(this, [this, message] { fail("Windows GATT device error: " + message); }, Qt::QueuedConnection);
            }
        });
}

// Disconnect from the current BLE device and release GATT resources
// 断开与当前 BLE 设备的连接并释放 GATT 资源
void WinRtBleTransport::disconnectDevice()
{
    releaseNativeObjects();
    setConnecting(false);
    setConnected(false);
    setStatus("Disconnected");
}

// Write a frame to the Nordic UART RX characteristic via GATT WriteWithoutResponse
// 通过 GATT WriteWithoutResponse 向 Nordic UART RX 特征值写入一帧数据
void WinRtBleTransport::writeFrame(const QByteArray &frame)
{
    if (!m_connected || !m_nativeState || frame.isEmpty()) {
        emit errorOccurred("Windows GATT transport is not ready to send");
        return;
    }
    setSending(true);
    try {
        auto *state = static_cast<NativeState *>(m_nativeState);
        DataWriter writer;
        const auto bytes = winrt::array_view<const uint8_t>(reinterpret_cast<const uint8_t *>(frame.constData()), frame.size());
        writer.WriteBytes(bytes);
        const QString hex = KeyboardProtocol::hexDump(frame);
        // Mouse traffic is a real-time stream.  Windows serializes
        // WriteWithResponse requests and was releasing them in roughly 500 ms
        // batches.  The ESP NUS RX characteristic advertises WRITE_NR, so use
        // ATT Write Command: it is transmitted at connection-event cadence
        // rather than waiting for one response per 8-byte frame.
        state->writeCharacteristic.WriteValueAsync(writer.DetachBuffer(), GattWriteOption::WriteWithoutResponse).Completed(
            [this, hex, length = frame.size()](auto const &writeOp, winrt::Windows::Foundation::AsyncStatus status) {
                if (status != winrt::Windows::Foundation::AsyncStatus::Completed || writeOp.GetResults() != GattCommunicationStatus::Success) {
                    QMetaObject::invokeMethod(this, [this] { fail("Windows GATT write failed"); }, Qt::QueuedConnection);
                    return;
                }
                QMetaObject::invokeMethod(this, [this, hex, length] {
                    emit diagnostic("Windows GATT write queued (no response) len=" + QString::number(length) + " hex=" + hex);
                    emit txPacket(hex, QString("BLE_FRAME (%1 bytes)").arg(length));
                    setSending(false);
                }, Qt::QueuedConnection);
            });
    } catch (const winrt::hresult_error &error) {
        fail("Windows GATT write error 0x" + QString::number(static_cast<quint32>(error.code().value), 16)
             + ": " + QString::fromWCharArray(error.message().c_str()));
    }
}

// Set connecting state and emit signal if changed / 设置连接中状态并在变化时发出信号
void WinRtBleTransport::setConnecting(bool value) { if (m_connecting != value) { m_connecting = value; emit connectingChanged(); } }
// Set connected state and emit signal if changed / 设置已连接状态并在变化时发出信号
void WinRtBleTransport::setConnected(bool value) { if (m_connected != value) { m_connected = value; emit connectedChanged(); } }
// Set sending state and emit signal if changed / 设置发送中状态并在变化时发出信号
void WinRtBleTransport::setSending(bool value) { if (m_sending != value) { m_sending = value; emit sendingChanged(); } }
// Set status text and emit signal if changed / 设置状态文本并在变化时发出信号
void WinRtBleTransport::setStatus(const QString &value) { if (m_statusText != value) { m_statusText = value; emit statusTextChanged(); } }

// Handle a fatal error: release all resources and emit error signals
// 处理致命错误: 释放所有资源并发出错误信号
void WinRtBleTransport::fail(const QString &message)
{
    releaseNativeObjects();
    setConnecting(false);
    setConnected(false);
    setSending(false);
    setStatus(message);
    emit diagnostic(message);
    emit errorOccurred(message);
}

// Delete and null the native WinRT GATT state pointer
// 删除并置空原生 WinRT GATT 状态指针
void WinRtBleTransport::releaseNativeObjects()
{
    delete static_cast<NativeState *>(m_nativeState);
    m_nativeState = nullptr;
}

#pragma once

#include <QObject>

class WinRtBleTransport;

// Selects the implementation below the application-facing BLE manager.
// Windows uses native WinRT GATT; other platforms retain QtBluetooth.
class BleSystemRouter final : public QObject
{
    Q_OBJECT
public:
    // Constructor / 构造函数
    explicit BleSystemRouter(QObject *parent = nullptr);
    // Check whether this platform uses native WinRT GATT transport
    // 检查当前平台是否使用原生 WinRT GATT 传输
    bool usesNativeTransport() const;
    // Get the native WinRT BLE transport (null on non-Windows)
    // 获取原生 WinRT BLE 传输对象（非 Windows 平台返回空指针）
    WinRtBleTransport *nativeTransport() const;

private:
#ifdef Q_OS_WIN
    WinRtBleTransport *m_winrtTransport = nullptr;
#endif
};

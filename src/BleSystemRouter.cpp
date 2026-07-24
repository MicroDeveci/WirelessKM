#include "BleSystemRouter.h"

#ifdef Q_OS_WIN
#include "WinRtBleTransport.h"
#endif

// Constructor: create the platform-specific native BLE transport
// 构造函数: 创建平台特定的原生 BLE 传输
BleSystemRouter::BleSystemRouter(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_WIN
    m_winrtTransport = new WinRtBleTransport(this);
#endif
}

// Return true on Windows (native WinRT), false on other platforms (QtBluetooth)
// 在 Windows 上返回 true（原生 WinRT），在其他平台上返回 false（QtBluetooth）
bool BleSystemRouter::usesNativeTransport() const
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

// Get the native WinRT BLE transport object (null on non-Windows platforms)
// 获取原生 WinRT BLE 传输对象（非 Windows 平台返回空指针）
WinRtBleTransport *BleSystemRouter::nativeTransport() const
{
#ifdef Q_OS_WIN
    return m_winrtTransport;
#else
    return nullptr;
#endif
}

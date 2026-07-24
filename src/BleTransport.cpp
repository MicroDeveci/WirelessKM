#include "BleTransport.h"
#include "BleManager.h"

// Constructor: wrap a BleManager as a Transport and connect signals
// 构造函数: 将 BleManager 封装为 Transport 并连接信号
BleTransport::BleTransport(BleManager *ble, QObject *parent)
    : Transport(parent)
    , m_ble(ble)
{
    if (!m_ble)
        return;

    connect(m_ble, &BleManager::connectedChanged, this, &Transport::connectedChanged);
    connect(m_ble, &BleManager::rawBytesReceived, this, &Transport::readyRead);
    connect(m_ble, &BleManager::errorOccurred, this, &Transport::errorOccurred);
}

// Check if the underlying BLE manager is connected
// 检查底层 BLE 管理器是否已连接
bool BleTransport::connected() const
{
    return m_ble && m_ble->isConnected();
}

// Write a frame through the BLE transport, emitting an error if disconnected
// 通过 BLE 传输写入一帧数据，断开连接时发出错误信号
void BleTransport::write(const QByteArray &frame)
{
    if (!connected()) {
        emit errorOccurred("BLE transport is not connected");
        return;
    }
    m_ble->writeFrame(frame);
}

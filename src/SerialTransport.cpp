#include "SerialTransport.h"
#include "SerialManager.h"

// Constructor: wrap a SerialManager as a Transport and connect signals
// 构造函数: 将 SerialManager 封装为 Transport 并连接信号
SerialTransport::SerialTransport(SerialManager *serial, QObject *parent)
    : Transport(parent)
    , m_serial(serial)
{
    if (!m_serial)
        return;

    connect(m_serial, &SerialManager::connectedChanged, this, &Transport::connectedChanged);
    connect(m_serial, &SerialManager::rawBytesReceived, this, &Transport::readyRead);
    connect(m_serial, &SerialManager::errorOccurred, this, &Transport::errorOccurred);
}

// Check if the underlying serial port is open and connected
// 检查底层串口是否已打开并连接
bool SerialTransport::connected() const
{
    return m_serial && m_serial->connected();
}

// Write a frame through the serial port, emitting an error if disconnected
// 通过串口写入一帧数据，断开连接时发出错误信号
void SerialTransport::write(const QByteArray &frame)
{
    if (!connected()) {
        emit errorOccurred("UART transport is not connected");
        return;
    }
    m_serial->writeRaw(frame);
}

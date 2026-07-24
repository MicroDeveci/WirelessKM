#pragma once

#include "Transport.h"

class SerialManager;

class SerialTransport : public Transport
{
    Q_OBJECT

public:
    // Constructor: wrap a SerialManager as a Transport interface
    // 构造函数: 将 SerialManager 封装为 Transport 接口
    explicit SerialTransport(SerialManager *serial, QObject *parent = nullptr);

    // Return the transport name "UART"
    // 返回传输名称 "UART"
    QString name() const override { return "UART"; }
    // Check if the underlying serial port is open and connected
    // 检查底层串口是否已打开并连接
    bool connected() const override;
    // Write a frame through the serial port
    // 通过串口写入一帧数据
    void write(const QByteArray &frame) override;

private:
    SerialManager *m_serial = nullptr;
};

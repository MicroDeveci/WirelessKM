#pragma once

#include "Transport.h"

class BleManager;

class BleTransport : public Transport
{
    Q_OBJECT

public:
    // Constructor: wrap a BleManager as a Transport interface
    // 构造函数: 将 BleManager 封装为 Transport 接口
    explicit BleTransport(BleManager *ble, QObject *parent = nullptr);

    // Return the transport name "BLE"
    // 返回传输名称 "BLE"
    QString name() const override { return "BLE"; }
    // Check if the underlying BLE manager is connected
    // 检查底层 BLE 管理器是否已连接
    bool connected() const override;
    // Write a frame through the BLE transport
    // 通过 BLE 传输写入一帧数据
    void write(const QByteArray &frame) override;

private:
    BleManager *m_ble = nullptr;
};

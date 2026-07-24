#pragma once

#include <QObject>

class Transport;

class TransportSelector : public QObject
{
    Q_OBJECT

public:
    // Constructor / 构造函数
    explicit TransportSelector(QObject *parent = nullptr);

    // Register BLE, UART, and HUDP transports and start monitoring connection state
    // 注册 BLE、UART 和 HUDP 传输并开始监控连接状态
    void setTransports(Transport *ble, Transport *uart, Transport *hudp);
    // Return the currently active transport pointer (may be null)
    // 返回当前活跃的传输指针（可能为空）
    Transport *activeTransport() const { return m_activeTransport; }
    // Return the name of the active transport, or "None"
    // 返回活跃传输的名称，无活跃传输时返回 "None"
    QString activeTransportName() const;

signals:
    void activeTransportChanged();

private slots:
    void updateActiveTransport();

private:
    Transport *chooseActiveTransport() const;

    Transport *m_ble = nullptr;
    Transport *m_uart = nullptr;
    Transport *m_hudp = nullptr;
    Transport *m_activeTransport = nullptr;
};

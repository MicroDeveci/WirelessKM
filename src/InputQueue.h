#pragma once

#include "Packet.h"
#include <QObject>
#include <QQueue>
#include <QTimer>

class Transport;

class InputQueue : public QObject
{
    Q_OBJECT

public:
    // Construct and start a periodic flush timer.
    // 构造并启动周期性刷新定时器。
    explicit InputQueue(QObject *parent = nullptr);

    // Return the number of packets waiting in the queue.
    // 返回队列中等待的包数量。
    int queueSize() const { return m_queue.size(); }
    // Whether a packet write is currently in progress.
    // 当前是否正在进行包写入。
    bool sending() const { return m_sending; }
    // Set or change the active transport used for flushing.
    // 设置或更改用于刷新的活跃传输。
    void setActiveTransport(Transport *transport);
    // Add a packet to the queue and attempt to flush.
    // 将包添加到队列并尝试刷新。
    void enqueue(const Packet &packet);

signals:
    void queueSizeChanged();
    void sendingChanged();
    void packetWritten(const QString &transport, const QString &hex, const QString &label);
    void errorOccurred(const QString &error);

private slots:
    // Periodic callback that drains queued packets through the active transport.
    // 周期性回调，通过活跃传输排空队列中的包。
    void tryFlush();

private:
    QQueue<Packet> m_queue;
    QTimer m_flushTimer;
    Transport *m_activeTransport = nullptr;
    bool m_sending = false;
    bool m_blockedNotified = false;
};

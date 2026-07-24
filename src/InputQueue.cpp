#include "InputQueue.h"
#include "KeyboardProtocol.h"
#include "Transport.h"
#include <QTimer>

// Construct and start a 10 ms periodic flush timer.
// 构造并启动 10 毫秒周期的刷新定时器。
InputQueue::InputQueue(QObject *parent)
    : QObject(parent)
{
    m_flushTimer.setInterval(10);
    connect(&m_flushTimer, &QTimer::timeout, this, &InputQueue::tryFlush);
    m_flushTimer.start();
}

// Set or change the active transport used for flushing queued packets.
// 设置或更改用于刷新队列包的活跃传输。
void InputQueue::setActiveTransport(Transport *transport)
{
    m_activeTransport = transport;
    if (m_activeTransport && m_activeTransport->connected())
        m_blockedNotified = false;
    tryFlush();
}

// Add a packet to the queue and attempt to flush immediately.
// 将包添加到队列并立即尝试刷新。
void InputQueue::enqueue(const Packet &packet)
{
    m_queue.enqueue(packet);
    m_blockedNotified = false;
    emit queueSizeChanged();
    tryFlush();
}

// Drain all queued packets through the active transport; emit errors if no transport.
// 通过活跃传输排空所有队列包；若无传输则发出错误。
void InputQueue::tryFlush()
{
    if (!m_activeTransport || !m_activeTransport->connected()) {
        if (!m_queue.isEmpty() && !m_blockedNotified) {
            m_blockedNotified = true;
            emit errorOccurred("No active transport");
        }
        return;
    }

    while (!m_queue.isEmpty() && m_activeTransport && m_activeTransport->connected()) {
        const Packet packet = m_queue.dequeue();
        m_blockedNotified = false;
        emit queueSizeChanged();

        m_sending = true;
        emit sendingChanged();
        m_activeTransport->write(packet.bytes);
        emit packetWritten(m_activeTransport->name(), KeyboardProtocol::hexDump(packet.bytes), packet.label);
    }

    if (m_sending) {
        QTimer::singleShot(50, this, [this]() {
            if (!m_sending)
                return;
            m_sending = false;
            emit sendingChanged();
        });
    }
}

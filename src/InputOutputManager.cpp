#include "InputOutputManager.h"
#include "InputQueue.h"
#include "KeyboardProtocol.h"
#include "MouseInputManager.h"
#include "MouseProtocol.h"
#include "TransportSelector.h"

// Construct and connect mouse report requests to protocol encoding and queueing.
// 构造并将鼠标报告请求连接到协议编码和入队。
InputOutputManager::InputOutputManager(InputQueue *queue, MouseInputManager *mouse,
                                       TransportSelector *selector, QObject *parent)
    : QObject(parent), m_queue(queue), m_mouse(mouse), m_selector(selector)
{
    connect(m_mouse, &MouseInputManager::reportRequested, this,
            [this](quint8 buttons, qint8 dx, qint8 dy, qint8 wheel) {
        if (!m_selector->activeTransport()) {
            m_mouse->resetState();
            emit outputRejected("Mouse", "No active transport");
            return;
        }
        Packet packet;
        packet.bytes = MouseProtocol::encodeReport(buttons, dx, dy, wheel);
        packet.label = MouseProtocol::describeReport(buttons, dx, dy, wheel);
        m_queue->enqueue(packet);
        emit mouseQueued(buttons, dx, dy, wheel, m_selector->activeTransportName());
    });
}

// Handle a captured keyboard event; map to HID usage, encode, and enqueue.
// 处理捕获的键盘事件；映射到 HID usage、编码并入队。
void InputOutputManager::handleCapturedKey(const QString &key, const QString &text, bool pressed, bool autoRepeat, int modifiers)
{
    Q_UNUSED(modifiers);
    if (autoRepeat)
        return;
    quint16 usage = 0;
    if (!KeyboardProtocol::keyUsageForCapturedInput(key, text, &usage)) {
        emit keyUnmapped(key, text);
        return;
    }
    if (!m_selector->activeTransport()) {
        emit outputRejected("Passthrough", "No active transport");
        return;
    }
    Packet packet;
    packet.bytes = KeyboardProtocol::encodeRawKeycode(usage, pressed);
    packet.label = QString("PASSTHROUGH %1 %2 usage=0x%3")
        .arg(pressed ? "DOWN" : "UP", key).arg(usage, 4, 16, QChar('0'));
    m_queue->enqueue(packet);
    emit capturedKeyQueued(key, usage, pressed, m_selector->activeTransportName());
}

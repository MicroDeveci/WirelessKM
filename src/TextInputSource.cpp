#include "TextInputSource.h"
#include "InputQueue.h"
#include "KeyboardProtocol.h"
#include "TransportSelector.h"

// Construct and connect the zero-interval single-shot flush timer.
// 构造并连接零间隔单次触发的刷新定时器。
TextInputSource::TextInputSource(InputQueue *inputQueue, TransportSelector *transportSelector, QObject *parent)
    : QObject(parent), m_inputQueue(inputQueue), m_transportSelector(transportSelector)
{
    m_flushTimer.setInterval(0);
    m_flushTimer.setSingleShot(true);
    connect(&m_flushTimer, &QTimer::timeout, this, &TextInputSource::flushNext);
}

// Enqueue a text string for validation and transmission.
// 将文本字符串入队以进行验证和传输。
void TextInputSource::enqueueText(const QString &text, const QString &source)
{
    m_queue.enqueue({text, source});
    emit queueSizeChanged();

    if (!m_flushTimer.isActive())
        m_flushTimer.start();
}

// Process the next queued text request; validate mapping and enqueue packets.
// 处理下一个队列中的文本请求；验证映射并入队数据包。
void TextInputSource::flushNext()
{
    if (m_queue.isEmpty())
        return;

    const TextInputRequest request = m_queue.dequeue();
    emit queueSizeChanged();
    const TextMappingResult mapping = KeyboardProtocol::validateTextMapping(request.text);
    emit mappingChecked(request, mapping.mappedChars, mapping.totalChars, mapping.unmappedCharacters);

    if (!m_inputQueue || !m_transportSelector || !m_transportSelector->activeTransport()) {
        emit sendRejected(request.source, request.text.length());
    } else {
        const QString transport = m_transportSelector->activeTransportName();
        m_inputQueue->enqueue(KeyboardProtocol::encodeTextPacket(request.text, transport));
        emit textQueued(request.text, request.source, transport);
    }

    if (!m_queue.isEmpty())
        m_flushTimer.start();
}

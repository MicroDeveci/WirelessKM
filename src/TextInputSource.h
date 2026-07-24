#pragma once

#include <QObject>
#include <QMetaType>
#include <QQueue>
#include <QString>
#include <QTimer>

class InputQueue;
class TransportSelector;

struct TextInputRequest
{
    QString text;
    QString source;
};

Q_DECLARE_METATYPE(TextInputRequest)

class TextInputSource : public QObject
{
    Q_OBJECT

public:
    // Construct with references to the input queue and transport selector.
    // 使用输入队列和传输选择器的引用进行构造。
    explicit TextInputSource(InputQueue *inputQueue, TransportSelector *transportSelector, QObject *parent = nullptr);

    // Return the number of text requests waiting in the queue.
    // 返回队列中等待的文本请求数量。
    int queueSize() const { return m_queue.size(); }
    // Enqueue a text string for validation and transmission.
    // 将文本字符串入队以进行验证和传输。
    void enqueueText(const QString &text, const QString &source);

signals:
    void queueSizeChanged();
    void mappingChecked(const TextInputRequest &request, int mappedChars, int totalChars,
                        const QStringList &unmappedCharacters);
    void textQueued(const QString &text, const QString &source, const QString &transport);
    void sendRejected(const QString &source, int length);

private slots:
    // Process the next queued text request; validate mapping and enqueue packets.
    // 处理下一个队列中的文本请求；验证映射并入队数据包。
    void flushNext();

private:
    QQueue<TextInputRequest> m_queue;
    QTimer m_flushTimer;
    InputQueue *m_inputQueue = nullptr;
    TransportSelector *m_transportSelector = nullptr;
};

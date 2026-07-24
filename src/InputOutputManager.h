#pragma once

#include <QObject>

class InputQueue;
class MouseInputManager;
class TransportSelector;

// Bridges input producers to InputQueue. It owns protocol encoding and leaves
// InterfaceLayer with QML-facing state and diagnostics only.
class InputOutputManager : public QObject
{
    Q_OBJECT
public:
    // Construct with references to the input queue, mouse manager, and transport selector.
    // 使用输入队列、鼠标管理器和传输选择器的引用进行构造。
    InputOutputManager(InputQueue *queue, MouseInputManager *mouse,
                       TransportSelector *selector, QObject *parent = nullptr);

public slots:
    // Handle a captured keyboard event from CaptureManager; encode and enqueue it.
    // 处理来自 CaptureManager 的捕获键盘事件；编码并入队。
    void handleCapturedKey(const QString &key, const QString &text, bool pressed, bool autoRepeat, int modifiers);

signals:
    void outputRejected(const QString &source, const QString &detail);
    void keyUnmapped(const QString &key, const QString &text);
    void mouseQueued(quint8 buttons, qint8 dx, qint8 dy, qint8 wheel, const QString &transport);
    void capturedKeyQueued(const QString &key, quint16 usage, bool pressed, const QString &transport);

private:
    InputQueue *m_queue;
    MouseInputManager *m_mouse;
    TransportSelector *m_selector;
};

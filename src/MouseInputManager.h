#pragma once

#include <QObject>
#include <QTimer>
#include <QtGlobal>

class MouseInputManager : public QObject
{
    Q_OBJECT

public:
    // Construct and start the 4 ms flush aggregation timer.
    // 构造并启动 4 毫秒的刷新聚合定时器。
    explicit MouseInputManager(QObject *parent = nullptr);

    // Return the current mouse button state bitmask.
    // 返回当前鼠标按钮状态位掩码。
    quint8 buttons() const { return m_buttons; }
    // Return the flush interval in milliseconds.
    // 返回刷新间隔（毫秒）。
    int flushIntervalMs() const { return m_flushTimer.interval(); }

    // Accumulate relative mouse movement deltas.
    // 累积相对鼠标移动增量。
    void moveBy(int dx, int dy);
    // Accumulate relative mouse wheel delta.
    // 累积相对鼠标滚轮增量。
    void wheelBy(int delta);
    // Set the full mouse button bitmask directly.
    // 直接设置完整的鼠标按钮位掩码。
    void setButtons(quint8 buttons);
    // Press or release a single mouse button by bitmask.
    // 按位掩码按下或释放单个鼠标按钮。
    void setButton(quint8 button, bool pressed);
    // Release all mouse buttons immediately.
    // 立即释放所有鼠标按钮。
    void releaseAll();
    // Reset all pending deltas and button state without sending reports.
    // 重置所有待处理的增量和按钮状态，不发送报告。
    void resetState();
    // Force-flush all accumulated deltas immediately.
    // 立即强制刷新所有累积的增量。
    void flushPending();

signals:
    void buttonsChanged();
    void reportRequested(quint8 buttons, qint8 dx, qint8 dy, qint8 wheel);

private:
    static constexpr int FlushIntervalMs = 4;

    // Start the flush timer if not already active.
    // 若定时器未在运行则启动刷新定时器。
    void scheduleFlush();

    QTimer m_flushTimer;
    qint64 m_pendingX = 0;
    qint64 m_pendingY = 0;
    qint64 m_pendingWheel = 0;
    quint8 m_buttons = 0;
};

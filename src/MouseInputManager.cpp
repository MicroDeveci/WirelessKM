#include "MouseInputManager.h"

#include "MouseProtocol.h"

#include <algorithm>

namespace {
// Keep relative reports small at normal hand speed.  4 ms provides a 250 Hz
// output cadence while remaining well within the UART and USB HID budget.

// Clamp a pending delta to the protocol's representable range.
// 将待处理的增量限制在协议可表示的范围内。
int takeReportDelta(qint64 value)
{
    return static_cast<int>(std::clamp<qint64>(
        value, MouseProtocol::MinimumDelta, MouseProtocol::MaximumDelta));
}
}

// Construct and start the 4 ms flush aggregation timer.
// 构造并启动 4 毫秒的刷新聚合定时器。
MouseInputManager::MouseInputManager(QObject *parent)
    : QObject(parent)
{
    m_flushTimer.setSingleShot(true);
    m_flushTimer.setInterval(FlushIntervalMs);
    connect(&m_flushTimer, &QTimer::timeout, this, &MouseInputManager::flushPending);
}

// Accumulate relative mouse movement deltas and schedule a flush.
// 累积相对鼠标移动增量并调度刷新。
void MouseInputManager::moveBy(int dx, int dy)
{
    if (dx == 0 && dy == 0)
        return;
    m_pendingX += dx;
    m_pendingY += dy;
    scheduleFlush();
}

// Accumulate relative mouse wheel delta and schedule a flush.
// 累积相对鼠标滚轮增量并调度刷新。
void MouseInputManager::wheelBy(int delta)
{
    if (delta == 0)
        return;
    m_pendingWheel += delta;
    scheduleFlush();
}

// Press or release a single mouse button by bitmask.
// 按位掩码按下或释放单个鼠标按钮。
void MouseInputManager::setButton(quint8 button, bool pressed)
{
    if (button == 0 || (button & ~MouseProtocol::ButtonMask) != 0)
        return;

    const quint8 nextButtons = pressed
        ? static_cast<quint8>(m_buttons | button)
        : static_cast<quint8>(m_buttons & ~button);
    setButtons(nextButtons);
}

// Set the full mouse button bitmask; flushes pending motion first.
// 设置完整的鼠标按钮位掩码；先刷新待处理的移动。
void MouseInputManager::setButtons(quint8 buttons)
{
    const quint8 nextButtons = buttons & MouseProtocol::ButtonMask;
    if (nextButtons == m_buttons)
        return;

    // Preserve event ordering: motion accumulated before a button transition
    // must use the previous button state.
    flushPending();
    m_buttons = nextButtons;
    emit buttonsChanged();
    emit reportRequested(m_buttons, 0, 0, 0);
}

// Release all mouse buttons immediately and send a zero-button report.
// 立即释放所有鼠标按钮并发送零按钮报告。
void MouseInputManager::releaseAll()
{
    flushPending();
    if (m_buttons == 0)
        return;
    m_buttons = 0;
    emit buttonsChanged();
    emit reportRequested(0, 0, 0, 0);
}

// Reset all pending deltas and button state without sending reports.
// 重置所有待处理的增量和按钮状态，不发送报告。
void MouseInputManager::resetState()
{
    m_flushTimer.stop();
    m_pendingX = 0;
    m_pendingY = 0;
    m_pendingWheel = 0;
    if (m_buttons == 0)
        return;
    m_buttons = 0;
    emit buttonsChanged();
}

// Force-flush all accumulated movement, wheel, and button deltas now.
// 立即强制刷新所有累积的移动、滚轮和按钮增量。
void MouseInputManager::flushPending()
{
    m_flushTimer.stop();
    while (m_pendingX != 0 || m_pendingY != 0 || m_pendingWheel != 0) {
        const int dx = takeReportDelta(m_pendingX);
        const int dy = takeReportDelta(m_pendingY);
        const int wheel = takeReportDelta(m_pendingWheel);
        m_pendingX -= dx;
        m_pendingY -= dy;
        m_pendingWheel -= wheel;
        emit reportRequested(m_buttons, static_cast<qint8>(dx), static_cast<qint8>(dy),
                             static_cast<qint8>(wheel));
    }
}

// Start the flush timer if not already active (fixed-rate, not a debounce).
// 若定时器未在运行则启动刷新定时器（固定速率，非防抖）。
void MouseInputManager::scheduleFlush()
{
    // Do not restart an active timer: this is a fixed-rate aggregation window,
    // not a debounce that could starve continuous pointer movement.
    if (!m_flushTimer.isActive())
        m_flushTimer.start();
}

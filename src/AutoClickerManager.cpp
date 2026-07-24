#include "AutoClickerManager.h"

#include "InputQueue.h"
#include "KeyboardProtocol.h"
#include "MouseInputManager.h"
#include "MouseProtocol.h"
#include "TransportSelector.h"

#include <algorithm>

namespace {
constexpr int MinimumIntervalMs = 10;
constexpr int MaximumIntervalMs = 60000;
constexpr quint16 AutoClickMouseLeft = 0xff01;
constexpr quint16 AutoClickMouseRight = 0xff02;
constexpr quint16 AutoClickMouseMiddle = 0xff04;

// Map a HID usage ID to the corresponding MouseProtocol button byte.
// 将 HID usage ID 映射到对应的 MouseProtocol 按钮字节。
quint8 mouseButtonForUsage(quint16 usage)
{
    switch (usage) {
    case AutoClickMouseLeft: return MouseProtocol::LeftButton;
    case AutoClickMouseRight: return MouseProtocol::RightButton;
    case AutoClickMouseMiddle: return MouseProtocol::MiddleButton;
    default: return 0;
    }
}

// Return a human-readable label for the given HID usage ID.
// 返回给定 HID usage ID 的人类可读标签。
QString keyLabel(quint16 usage)
{
    for (const KeyDefinition &definition : KeyboardProtocol::keyDefinitions()) {
        if (definition.usage == usage)
            return definition.normal.isNull() ? definition.id : QString(definition.normal).toUpper();
    }
    return QString("0x%1").arg(usage, 4, 16, QChar('0'));
}
}

// Default constructor delegates to the full constructor with null targets.
// 默认构造函数委托给全部参数构造函数，各目标传入空指针。
AutoClickerManager::AutoClickerManager(QObject *parent)
    : AutoClickerManager(nullptr, nullptr, nullptr, parent)
{
}

// Construct with dependency-injected input queue, mouse manager, and transport selector.
// 使用依赖注入的输入队列、鼠标管理器和传输选择器进行构造。
AutoClickerManager::AutoClickerManager(InputQueue *inputQueue, MouseInputManager *mouseInputManager,
                                       TransportSelector *transportSelector, QObject *parent)
    : QObject(parent), m_inputQueue(inputQueue), m_mouseInputManager(mouseInputManager), m_transportSelector(transportSelector)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &AutoClickerManager::triggerCycle);
}

// Return a human-readable status string for the current auto-clicker state.
// 返回当前自动点击器状态的人类可读字符串。
QString AutoClickerManager::status() const
{
    if (m_running) {
        return m_repeatCount == 0
            ? tr("Running · %1 cycles").arg(m_completedCycles)
            : tr("Running · %1 / %2").arg(m_completedCycles).arg(m_repeatCount);
    }
    if (m_completedCycles > 0 && m_repeatCount > 0 && m_completedCycles >= m_repeatCount)
        return tr("Completed · %1 cycles").arg(m_completedCycles);
    return m_keys.isEmpty() ? tr("Add at least one key") : tr("Ready");
}

// Replace the full list of key usage IDs; stops auto-clicker if running.
// 替换完整的按键 usage ID 列表；若正在运行则停止自动点击器。
void AutoClickerManager::setKeys(const QList<quint16> &keys)
{
    QList<quint16> normalized;
    for (quint16 usage : keys) {
        if (usage != 0 && !normalized.contains(usage))
            normalized.append(usage);
    }
    if (m_keys == normalized)
        return;

    stop();
    m_keys = normalized;
    emit keysChanged();
    emit statusChanged();
}

// Add a single key usage ID to the auto-click list if not already present.
// 如果按键 usage ID 不在列表中，向自动点击列表添加该 ID。
void AutoClickerManager::addKey(quint16 usage)
{
    if (usage == 0 || m_keys.contains(usage))
        return;
    QList<quint16> updated = m_keys;
    updated.append(usage);
    setKeys(updated);
}

// Remove all occurrences of the given key usage ID from the auto-click list.
// 从自动点击列表中移除给定按键 usage ID 的所有出现。
void AutoClickerManager::removeKey(quint16 usage)
{
    QList<quint16> updated = m_keys;
    updated.removeAll(usage);
    setKeys(updated);
}

// Set the interval between auto-click cycles, clamped to the allowed range.
// 设置自动点击周期之间的间隔，并限制在允许的范围内。
void AutoClickerManager::setIntervalMs(int intervalMs)
{
    const int normalized = std::clamp(intervalMs, MinimumIntervalMs, MaximumIntervalMs);
    if (m_intervalMs == normalized)
        return;
    m_intervalMs = normalized;
    emit intervalChanged();
}

// Set the repeat count; 0 means infinite looping.
// 设置重复次数，0 表示无限循环。
void AutoClickerManager::setRepeatCount(int repeatCount)
{
    const int normalized = std::max(0, repeatCount);
    if (m_repeatCount == normalized)
        return;
    m_repeatCount = normalized;
    emit repeatCountChanged();
    emit statusChanged();
}

// Start the auto-clicker if not already running and keys are set.
// 如果尚未运行且已设置按键，启动自动点击器。
void AutoClickerManager::start()
{
    if (m_running || m_keys.isEmpty()) {
        emit statusChanged();
        return;
    }

    m_completedCycles = 0;
    m_running = true;
    emit completedCyclesChanged();
    emit runningChanged();
    emit statusChanged();
    triggerCycle();
}

// Stop the auto-clicker and cancel any pending timer.
// 停止自动点击器并取消待处理的定时器。
void AutoClickerManager::stop()
{
    if (!m_running)
        return;
    m_timer.stop();
    m_running = false;
    emit runningChanged();
    emit statusChanged();
}

// Execute one click cycle: enqueue key-down/mouse-down, then key-up/mouse-up.
// 执行一次点击周期：先入队按键按下/鼠标按下，再入队按键抬起/鼠标抬起。
void AutoClickerManager::triggerCycle()
{
    if (!m_running)
        return;

    // Keep the signal-only mode for hardware-free tests and alternate output
    // adapters. The desktop application always provides all three targets.
    if (!m_inputQueue && !m_mouseInputManager && !m_transportSelector) {
        emit clickRequested(m_keys);
    } else if (!m_inputQueue || !m_mouseInputManager || !m_transportSelector || !m_transportSelector->activeTransport()) {
        emit sendRejected();
        stop();
        return;
    } else {
        for (quint16 usage : m_keys) {
            if (!mouseButtonForUsage(usage))
                m_inputQueue->enqueue({KeyboardProtocol::encodeRawKeycode(usage, true), "AUTO_CLICK DOWN " + keyLabel(usage), 0});
        }
        for (quint16 usage : m_keys) {
            const quint8 button = mouseButtonForUsage(usage);
            if (button)
                m_mouseInputManager->setButton(button, true);
        }
        for (quint16 usage : m_keys) {
            const quint8 button = mouseButtonForUsage(usage);
            if (button)
                m_mouseInputManager->setButton(button, false);
            else
                m_inputQueue->enqueue({KeyboardProtocol::encodeRawKeycode(usage, false), "AUTO_CLICK UP " + keyLabel(usage), 0});
        }
        emit clickRequested(m_keys);
        emit cycleEnqueued(m_keys.size(), m_completedCycles + 1);
    }
    ++m_completedCycles;
    emit completedCyclesChanged();

    if (m_repeatCount > 0 && m_completedCycles >= m_repeatCount) {
        m_running = false;
        emit runningChanged();
        emit statusChanged();
        return;
    }

    emit statusChanged();
    m_timer.start(m_intervalMs);
}

#pragma once

#include <QObject>
#include <QList>
#include <QTimer>

class InputQueue;
class MouseInputManager;
class TransportSelector;

class AutoClickerManager : public QObject
{
    Q_OBJECT

public:
    // Constructor for test usage, accepts parent only.
    // 用于测试场景的构造函数，仅接受父对象参数。
    explicit AutoClickerManager(QObject *parent = nullptr);
    // Constructor with full dependency injection for production use.
    // 带完整依赖注入的构造函数，用于生产环境。
    explicit AutoClickerManager(InputQueue *inputQueue, MouseInputManager *mouseInputManager,
                                TransportSelector *transportSelector, QObject *parent = nullptr);

    // Get the list of assigned key usage IDs.
    // 获取已分配的按键 usage ID 列表。
    QList<quint16> keys() const { return m_keys; }
    // Get the interval between auto-click cycles in milliseconds.
    // 获取自动点击周期之间的间隔（毫秒）。
    int intervalMs() const { return m_intervalMs; }
    // Get the repeat count; 0 means infinite.
    // 获取重复次数，0 表示无限重复。
    int repeatCount() const { return m_repeatCount; }
    // Get the number of completed auto-click cycles.
    // 获取已完成的自动点击周期数。
    int completedCycles() const { return m_completedCycles; }
    // Get whether the auto-clicker is currently running.
    // 获取自动点击器是否正在运行。
    bool running() const { return m_running; }
    // Get a human-readable status string for the current state.
    // 获取当前状态的人类可读字符串。
    QString status() const;

    // Set the full list of key usage IDs to auto-click.
    // 设置要自动点击的完整按键 usage ID 列表。
    void setKeys(const QList<quint16> &keys);
    // Add a single key usage ID to the auto-click list.
    // 向自动点击列表中添加单个按键 usage ID。
    void addKey(quint16 usage);
    // Remove a single key usage ID from the auto-click list.
    // 从自动点击列表中移除单个按键 usage ID。
    void removeKey(quint16 usage);
    // Set the interval between auto-click cycles in milliseconds.
    // 设置自动点击周期之间的间隔（毫秒）。
    void setIntervalMs(int intervalMs);
    // Set the repeat count; 0 means infinite.
    // 设置重复次数，0 表示无限重复。
    void setRepeatCount(int repeatCount);
    // Start the auto-clicker cycle.
    // 启动自动点击器周期。
    void start();
    // Stop the auto-clicker and cancel any pending timer.
    // 停止自动点击器并取消待处理的定时器。
    void stop();

signals:
    void keysChanged();
    void intervalChanged();
    void repeatCountChanged();
    void completedCyclesChanged();
    void runningChanged();
    void statusChanged();
    void clickRequested(const QList<quint16> &keys);
    void cycleEnqueued(int keyCount, int cycleNumber);
    void sendRejected();

private slots:
    // Internal timer callback that executes one click cycle.
    // 内部定时器回调，执行一次点击周期。
    void triggerCycle();

private:
    QList<quint16> m_keys;
    QTimer m_timer;
    int m_intervalMs = 100;
    int m_repeatCount = 0;
    int m_completedCycles = 0;
    bool m_running = false;
    InputQueue *m_inputQueue = nullptr;
    MouseInputManager *m_mouseInputManager = nullptr;
    TransportSelector *m_transportSelector = nullptr;
};

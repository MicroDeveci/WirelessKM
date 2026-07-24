#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QVector>

class LogManager : public QObject
{
    Q_OBJECT

public:
    // Construct with a UI update debounce timer.
    // 构造并设置 UI 更新防抖定时器。
    explicit LogManager(QObject *parent = nullptr);

    // Enable or disable inclusion of DEBUG-level messages.
    // 启用或禁用包含 DEBUG 级别的消息。
    void setDebugTrace(bool enabled) { m_debugTrace = enabled; }
    // Set the log file path (kept for API compatibility; no-op).
    // 设置日志文件路径（保留用于 API 兼容性；空操作）。
    // Kept for API compatibility. Runtime logs stay in RAM; exportJson() is
    // the only operation that writes them to disk.
    void setLogFilePath(const QString &path) { Q_UNUSED(path); }

    // Return the currently selected log level filter.
    // 返回当前选定的日志级别过滤器。
    int currentLogLevel() const { return m_currentLogLevel; }
    // Return the count of INFO-level log entries.
    // 返回 INFO 级别日志条目的数量。
    int infoCount() const { return m_infoCount; }
    // Return the count of WARN-level log entries.
    // 返回 WARN 级别日志条目的数量。
    int warningCount() const { return m_warningCount; }
    // Return the count of ERROR-level log entries.
    // 返回 ERROR 级别日志条目的数量。
    int errorLogCount() const { return m_errorLogCount; }
    // Return the count of DEBUG-level log entries.
    // 返回 DEBUG 级别日志条目的数量。
    int debugCount() const { return m_debugCount; }
    // Return the visible log entries for the current level filter.
    // 返回当前级别过滤器的可见日志条目。
    QVariantList logEntries() const { return m_logEntries; }
    // Return a chronological snapshot of all stored log entries.
    // 返回所有存储日志条目的按时间排序快照。
    // Returns a chronological snapshot.  This is intentionally a snapshot so
    // the QML-facing list never owns the million-entry backing store.
    QVariantList allLogs() const;
    // Return the formatted detail text for the selected log entry.
    // 返回所选日志条目的格式化详情文本。
    QString selectedLogDetail() const { return m_selectedLogDetail; }
    // Return the log view revision counter for change tracking.
    // 返回用于变更跟踪的日志视图版本计数器。
    int logViewRevision() const { return m_logViewRevision; }

    // Return the current visible log entries as plain text lines.
    // 将当前可见日志条目返回为纯文本行。
    QString logText() const;
    // Append a log entry at the given level with module and message.
    // 在给定级别追加一条带有模块和消息的日志条目。
    void append(int level, const QString &module, const QString &message, const QString &detail = QString());
    // Switch the visible log level filter and rebuild the view.
    // 切换可见日志级别过滤器并重建视图。
    void switchLevel(int level);
    // Select a log entry by index and populate selectedLogDetail.
    // 按索引选择日志条目并填充 selectedLogDetail。
    void select(int index);
    // Clear all stored log entries of the given level.
    // 清除给定级别的所有存储日志条目。
    void clearLevel(int level);
    // Export all stored log entries to a JSON file; returns true on success.
    // 将所有存储日志条目导出到 JSON 文件；成功返回 true。
    bool exportJson(const QString &path);
    // Rebuild the visible log view from the ring buffer.
    // 从环形缓冲区重建可见日志视图。
    void rebuildView();

    // Convert a numeric log level to a human-readable name.
    // 将数字日志级别转换为人类可读的名称。
    static QString levelName(int level);

signals:
    void currentLogLevelChanged();
    void infoCountChanged();
    void warningCountChanged();
    void errorLogCountChanged();
    void debugCountChanged();
    void logEntriesChanged();
    void selectedLogDetailChanged();
    void logViewRevisionChanged();

private:
    static constexpr int MaxStoredLogs = 1'000'000;
    static constexpr int MaxVisibleLogs = 1'000;

    // Retrieve a stored log entry by logical index from the ring buffer.
    // 通过逻辑索引从环形缓冲区检索存储的日志条目。
    const QVariant &storedLogAt(int logicalIndex) const;
    // Append a log entry to the ring buffer, overwriting the oldest if full.
    // 向环形缓冲区追加日志条目，满时覆盖最旧条目。
    void appendStoredLog(const QVariant &entry);
    // Rebuild the visible view from the full ring buffer for the current level.
    // 根据当前级别从完整环形缓冲区重建可见视图。
    void rebuildVisibleView();
    // Schedule a debounced UI update to avoid per-packet redraws.
    // 调度防抖的 UI 更新以避免逐包重绘。
    void scheduleUiUpdate();
    // Publish pending UI update signals to QML.
    // 将待处理的 UI 更新信号发布到 QML。
    void publishUiUpdate();

    int m_currentLogLevel = 0;
    int m_infoCount = 0;
    int m_warningCount = 0;
    int m_errorLogCount = 0;
    int m_debugCount = 0;
    // Fixed-capacity chronological ring.  Overwriting the oldest entry is
    // O(1), unlike QVariantList::removeFirst() at one million entries.
    QVector<QVariant> m_allLogs;
    int m_logHead = 0;
    int m_logCount = 0;
    QVariantList m_logEntries;
    QString m_selectedLogDetail;
    QTimer m_uiUpdateTimer;
    bool m_uiUpdatePending = false;
    int m_logViewRevision = 0;
    bool m_debugTrace = true;
};

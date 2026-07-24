#include "LogManager.h"

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QVariantMap>
#include <algorithm>

// Construct with a 100 ms single-shot UI update debounce timer.
// 构造并设置 100 毫秒单次触发的 UI 更新防抖定时器。
LogManager::LogManager(QObject *parent)
    : QObject(parent)
{
    m_allLogs.reserve(MaxStoredLogs);
    m_uiUpdateTimer.setSingleShot(true);
    m_uiUpdateTimer.setInterval(100); // Never redraw the QML log view per packet.
    connect(&m_uiUpdateTimer, &QTimer::timeout, this, &LogManager::publishUiUpdate);
}

// Convert a numeric log level to a human-readable name.
// 将数字日志级别转换为人类可读的名称。
QString LogManager::levelName(int level)
{
    switch (level) {
    case 0: return "INFO";
    case 1: return "WARN";
    case 2: return "ERROR";
    case 3: return "DEBUG";
    }
    return "UNKNOWN";
}

// Append a log entry at the given level, updating counts and visible view.
// 在给定级别追加日志条目，更新计数和可见视图。
void LogManager::append(int level, const QString &module, const QString &message, const QString &detail)
{
    if (level == 3 && !m_debugTrace)
        return;

    const QString ts = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QVariantMap entry;
    entry["timestamp"] = ts;
    entry["level"] = level;
    entry["module"] = module;
    entry["message"] = message;
    entry["detail"] = detail;

    appendStoredLog(entry);

    switch (level) {
    case 0: ++m_infoCount; break;
    case 1: ++m_warningCount; break;
    case 2: ++m_errorLogCount; break;
    case 3: ++m_debugCount; break;
    }

    if (level == m_currentLogLevel) {
        if (m_logEntries.size() == MaxVisibleLogs)
            m_logEntries.removeFirst();
        m_logEntries.append(entry);
    }

    scheduleUiUpdate();
}

// Retrieve a stored log entry by logical index from the ring buffer.
// 通过逻辑索引从环形缓冲区检索存储的日志条目。
const QVariant &LogManager::storedLogAt(int logicalIndex) const
{
    return m_allLogs[(m_logHead + logicalIndex) % m_allLogs.size()];
}

// Append a log entry to the ring buffer, overwriting the oldest entry if full.
// 向环形缓冲区追加日志条目，满时覆盖最旧条目。
void LogManager::appendStoredLog(const QVariant &entry)
{
    if (m_logCount < MaxStoredLogs) {
        m_allLogs.append(entry);
        ++m_logCount;
        return;
    }

    const int replacedLevel = m_allLogs[m_logHead].toMap().value("level").toInt();
    switch (replacedLevel) {
    case 0: --m_infoCount; break;
    case 1: --m_warningCount; break;
    case 2: --m_errorLogCount; break;
    case 3: --m_debugCount; break;
    }
    m_allLogs[m_logHead] = entry;
    m_logHead = (m_logHead + 1) % MaxStoredLogs;
}

// Return a chronological snapshot of all stored log entries.
// 返回所有存储日志条目的按时间排序快照。
QVariantList LogManager::allLogs() const
{
    QVariantList snapshot;
    snapshot.reserve(m_logCount);
    for (int i = 0; i < m_logCount; ++i)
        snapshot.append(storedLogAt(i));
    return snapshot;
}

// Return the current visible log entries formatted as plain text lines.
// 将当前可见日志条目格式化为纯文本行返回。
QString LogManager::logText() const
{
    QStringList lines;
    lines.reserve(m_logEntries.size());
    for (const auto &v : m_logEntries) {
        const auto e = v.toMap();
        QString line = QString("%1 [%2] [%3] %4")
            .arg(e["timestamp"].toString(),
                 levelName(e["level"].toInt()),
                 e["module"].toString(),
                 e["message"].toString());
        const QString detail = e["detail"].toString();
        if (!detail.isEmpty())
            line += "\n  " + detail;
        lines.append(line);
    }
    return lines.join("\n");
}

// Switch the visible log level filter and rebuild the view.
// 切换可见日志级别过滤器并重建视图。
void LogManager::switchLevel(int level)
{
    m_currentLogLevel = qBound(0, level, 3);
    rebuildView();
    emit currentLogLevelChanged();
}

// Select a log entry by index and populate the detail view string.
// 按索引选择日志条目并填充详情视图字符串。
void LogManager::select(int index)
{
    if (index >= 0 && index < m_logEntries.size()) {
        const auto e = m_logEntries[index].toMap();
        m_selectedLogDetail = QString("[%1] %2\nModule: %3\nMessage: %4\n%5")
            .arg(e["timestamp"].toString(), levelName(e["level"].toInt()),
                 e["module"].toString(), e["message"].toString(), e["detail"].toString());
    } else {
        m_selectedLogDetail.clear();
    }
    emit selectedLogDetailChanged();
}

// Clear all stored log entries of the given level from the ring buffer.
// 从环形缓冲区清除给定级别的所有存储日志条目。
void LogManager::clearLevel(int level)
{
    const int lv = qBound(0, level, 3);
    QVector<QVariant> retained;
    retained.reserve(MaxStoredLogs);
    for (int i = 0; i < m_logCount; ++i) {
        const QVariant &entry = storedLogAt(i);
        if (entry.toMap().value("level").toInt() != lv)
            retained.append(entry);
    }
    m_allLogs = std::move(retained);
    m_logHead = 0;
    m_logCount = m_allLogs.size();

    switch (lv) {
    case 0: m_infoCount = 0; emit infoCountChanged(); break;
    case 1: m_warningCount = 0; emit warningCountChanged(); break;
    case 2: m_errorLogCount = 0; emit errorLogCountChanged(); break;
    case 3: m_debugCount = 0; emit debugCountChanged(); break;
    }
    rebuildView();
}

// Export all stored log entries to a JSON file; returns true on success.
// 将所有存储日志条目导出到 JSON 文件；成功返回 true。
bool LogManager::exportJson(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QJsonArray arr;
    for (int i = 0; i < m_logCount; ++i)
        arr.append(QJsonObject::fromVariantMap(storedLogAt(i).toMap()));
    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    return true;
}

// Rebuild the visible view and publish the UI update.
// 重建可见视图并发布 UI 更新。
void LogManager::rebuildView()
{
    rebuildVisibleView();
    publishUiUpdate();
}

// Rebuild the visible log view from the ring buffer for the current level.
// 根据当前级别从环形缓冲区重建可见日志视图。
void LogManager::rebuildVisibleView()
{
    QVector<QVariant> ring;
    ring.reserve(MaxVisibleLogs);
    int matched = 0;
    for (int i = 0; i < m_logCount; ++i) {
        const QVariant &entry = storedLogAt(i);
        if (entry.toMap().value("level").toInt() != m_currentLogLevel)
            continue;
        if (ring.size() < MaxVisibleLogs)
            ring.append(entry);
        else
            ring[matched % MaxVisibleLogs] = entry;
        ++matched;
    }
    m_logEntries.clear();
    const int visibleCount = qMin(matched, MaxVisibleLogs);
    const int first = matched > MaxVisibleLogs ? matched % MaxVisibleLogs : 0;
    for (int i = 0; i < visibleCount; ++i)
        m_logEntries.append(ring[(first + i) % MaxVisibleLogs]);
}

// Schedule a debounced UI update to avoid per-packet redraws.
// 调度防抖的 UI 更新以避免逐包重绘。
void LogManager::scheduleUiUpdate()
{
    m_uiUpdatePending = true;
    if (!m_uiUpdateTimer.isActive())
        m_uiUpdateTimer.start();
}

// Publish pending UI update signals to QML (counts, entries, revision).
// 将待处理的 UI 更新信号发布到 QML（计数、条目、版本号）。
void LogManager::publishUiUpdate()
{
    if (!m_uiUpdatePending && m_logEntries.isEmpty())
        return;
    m_uiUpdatePending = false;
    emit infoCountChanged();
    emit warningCountChanged();
    emit errorLogCountChanged();
    emit debugCountChanged();
    emit logEntriesChanged();
    ++m_logViewRevision;
    emit logViewRevisionChanged();
}

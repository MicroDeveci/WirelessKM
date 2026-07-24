#include "InterfaceLayer.h"

#include <QClipboard>
#include <QDateTime>
#include <QGuiApplication>
#include <QStandardPaths>
#include "LogManager.h"

// Register log management command routes (switch level, select, clear, export, refresh, copy)
// 注册日志管理相关的命令路由（切换级别、选择、清除、导出、刷新、复制）
void InterfaceLayer::registerLogRoutes()
{
    m_routes["log.switchLevel"] = [this](const QVariantMap &p) {
        const int requestedLevel = p["level"].toInt();
        const int activeLevel = qBound(0, requestedLevel, 3);
        traceDataFlow("route log.switchLevel", "log level switched", {{"requestedLevel", requestedLevel}, {"activeLevel", activeLevel}});
        m_logManager->switchLevel(activeLevel);
    };
    m_routes["log.select"] = [this](const QVariantMap &p) {
        int idx = p["index"].toInt();
        traceDataFlow("route log.select", "log selection requested", {{"index", idx}, {"visibleEntries", m_logManager->logEntries().size()}});
        m_logManager->select(idx);
    };
    m_routes["log.clear"] = [this](const QVariantMap &p) {
        int lv = qBound(0, p["level"].toInt(), 3);
        traceDataFlow("route log.clear", "clear requested", {{"level", lv}});
        m_logManager->clearLevel(lv);
        appendLog(1, "Log", "Cleared level: " + LogManager::levelName(lv));
    };
    m_routes["log.exportLogs"] = [this](const QVariantMap &) {
        traceDataFlow("route log.exportLogs", "export requested", {{"totalEntries", m_logManager->allLogs().size()}});
        QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
                       + "/esp32kb_log_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".json";
        if (m_logManager->exportJson(path)) {
            appendLog(0, "Log", "Exported to " + path);
        } else appendLog(2, "Log", "Export failed");
    };
    m_routes["log.refresh"] = [this](const QVariantMap &) {
        traceDataFlow("route log.refresh", "refresh requested", {{"activeLevel", m_logManager->currentLogLevel()}, {"totalEntries", m_logManager->allLogs().size()}});
        rebuildLogView();
    };
    m_routes["log.copyDetail"] = [this](const QVariantMap &) {
        traceDataFlow("route log.copyDetail", "copy requested", {{"hasDetail", !selectedLogDetail().isEmpty()}});
        QClipboard *cb = QGuiApplication::clipboard();
        if (cb && !selectedLogDetail().isEmpty()) cb->setText(selectedLogDetail());
    };
}

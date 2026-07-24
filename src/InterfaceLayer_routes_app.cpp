#include "InterfaceLayer.h"

#include <QDesktopServices>
#include <QUrl>

// Register application-level command routes (update check, open GitHub, sponsor link)
// 注册应用层面的命令路由（检查更新、打开GitHub、赞助链接）
void InterfaceLayer::registerAppRoutes()
{
    m_routes["app.checkUpdate"] = [this](const QVariantMap &) {
        traceDataFlow("route app.checkUpdate", "check update requested");
        appendLog(0, "App", "Checking for updates...");
    };
    m_routes["app.openGitHub"] = [this](const QVariantMap &) {
        traceDataFlow("route app.openGitHub", "open url requested");
        QDesktopServices::openUrl(QUrl("https://github.com/MicroDeveci/WirelessKM"));
    };
    m_routes["app.openSponsorLink"] = [this](const QVariantMap &) {
        traceDataFlow("route app.openSponsorLink", "open url requested");
        QDesktopServices::openUrl(QUrl("https://github.com/MicroDeveci/WirelessKM"));
    };
}

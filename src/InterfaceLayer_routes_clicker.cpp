#include "InterfaceLayer.h"

#include "AutoClickerManager.h"
#include "KeyboardProtocol.h"
#include <QKeySequence>

namespace {
constexpr quint16 AutoClickMouseLeft = 0xff01;
constexpr quint16 AutoClickMouseRight = 0xff02;
constexpr quint16 AutoClickMouseMiddle = 0xff04;
}

// Register auto-clicker command routes (add/remove keys, set interval, start/stop)
// 注册自动点击器相关的命令路由（添加/移除按键、设置间隔、启动/停止）
void InterfaceLayer::registerAutoClickerRoutes()
{
    m_routes["clicker.addKey"] = [this](const QVariantMap &params) {
        m_autoClickerManager->addKey(static_cast<quint16>(params.value("usage").toUInt()));
    };
    m_routes["clicker.addCapturedKey"] = [this](const QVariantMap &params) {
        const int qtKey = params.value("qtKey").toInt();
        const QString text = params.value("text").toString();
        quint16 usage = 0;
        const QString key = QKeySequence(qtKey).toString(QKeySequence::PortableText);
        if (KeyboardProtocol::keyUsageForCapturedInput(key, text, &usage))
            m_autoClickerManager->addKey(usage);
    };
    m_routes["clicker.addMouseButton"] = [this](const QVariantMap &params) {
        switch (params.value("button").toInt()) {
        case 1: m_autoClickerManager->addKey(AutoClickMouseLeft); break;
        case 2: m_autoClickerManager->addKey(AutoClickMouseRight); break;
        case 4: m_autoClickerManager->addKey(AutoClickMouseMiddle); break;
        }
    };
    m_routes["clicker.removeKey"] = [this](const QVariantMap &params) {
        m_autoClickerManager->removeKey(static_cast<quint16>(params.value("usage").toUInt()));
    };
    m_routes["clicker.clearKeys"] = [this](const QVariantMap &) {
        m_autoClickerManager->setKeys({});
    };
    m_routes["clicker.setInterval"] = [this](const QVariantMap &params) {
        m_autoClickerManager->setIntervalMs(params.value("intervalMs").toInt());
    };
    m_routes["clicker.setRepeatCount"] = [this](const QVariantMap &params) {
        m_autoClickerManager->setRepeatCount(params.value("repeatCount").toInt());
    };
    m_routes["clicker.start"] = [this](const QVariantMap &) {
        m_autoClickerManager->start();
        if (m_autoClickerManager->running())
            appendLog(0, "AutoClicker", "Started");
    };
    m_routes["clicker.stop"] = [this](const QVariantMap &) {
        m_autoClickerManager->stop();
        appendLog(0, "AutoClicker", "Stopped");
    };
}

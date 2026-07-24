#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTranslator>
#include <QAbstractNativeEventFilter>
#include <QStringList>
#include <cstdio>
#include "InterfaceLayer.h"
#include "CaptureManager.h"
#include "DebugTrace.h"

// 全局日志文件，将所有 qDebug 输出写到文件
static bool g_debugTrace = true;

// Global Qt message handler that routes debug output to stderr.
// 全局 Qt 消息处理器，将调试输出路由到 stderr。
static void traceMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    Q_UNUSED(ctx);
    // Startup/QML diagnostics happen before LogManager exists. Keep them
    // visible in the launching terminal instead of silently exiting.
    if (!g_debugTrace && type == QtDebugMsg)
        return;
    std::fprintf(stderr, "%s\n", qPrintable(msg));
    std::fflush(stderr);
}

// Application entry point; sets up logging, QML engine, and native event filters.
// 应用程序入口点；设置日志、QML 引擎和原生事件过滤器。
int main(int argc, char *argv[])
{
    QStringList args;
    for (int i = 1; i < argc; ++i) {
        args.append(QString::fromLocal8Bit(argv[i]).trimmed().toLower());
    }
    g_debugTrace = true;
    if (args.contains("release") || args.contains("--release")) {
        g_debugTrace = false;
    }
    if (args.contains("debug") || args.contains("--debug")) {
        g_debugTrace = true;
    }
    DebugTrace::setEnabled(g_debugTrace);

    // 初始化日志文件
    qInstallMessageHandler(traceMessageHandler);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ui_" + QLocale(locale).name();
        if (translator.load("./i18n/"+ baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    InterfaceLayer iface(g_debugTrace);

    // 安装 CaptureManager 作为全局热键拦截
    auto *captureMgr = qobject_cast<CaptureManager*>(iface.captureManagerObj());
    if (captureMgr) {
        app.installNativeEventFilter(captureMgr);
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("InterfaceLayer", &iface);
    engine.rootContext()->setContextProperty("SerialManager", iface.serialManagerObj());
    engine.rootContext()->setContextProperty("CaptureManager", iface.captureManagerObj());
    engine.rootContext()->setContextProperty("KeySettingManager", iface.keySettingManagerObj());
    engine.rootContext()->setContextProperty("BleManager", iface.bleManagerObj());

    const QUrl url(QStringLiteral("qrc:/App.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}

#pragma once

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QEvent>
#include <QPoint>
#include <QSet>

class CaptureManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

    Q_PROPERTY(bool registered READ isRegistered NOTIFY registeredChanged)
    Q_PROPERTY(QString hotkey READ hotkey NOTIFY hotkeyChanged)
    Q_PROPERTY(bool appCaptureListening READ appCaptureListening NOTIFY appCaptureModeChanged)
    Q_PROPERTY(bool appCaptureExclusive READ appCaptureExclusive NOTIFY appCaptureModeChanged)
    Q_PROPERTY(bool mouseCaptureListening READ mouseCaptureListening NOTIFY appCaptureModeChanged)

public:
    // Default constructor.
    // 默认构造函数。
    explicit CaptureManager(QObject *parent = nullptr);
    // Destructor; uninstalls event filter and unregisters hotkey.
    // 析构函数；卸载事件过滤器并注销热键。
    ~CaptureManager() override;

    // Whether a global hotkey is currently registered.
    // 当前是否已注册全局热键。
    bool isRegistered() const { return m_registered; }
    // Return the current hotkey string (e.g. "Ctrl+`").
    // 返回当前热键字符串（如 "Ctrl+`"）。
    QString hotkey() const { return m_hotkey; }
    // Whether application key capture in listen-only mode is active.
    // 应用程序按键捕获（仅监听模式）是否激活。
    bool appCaptureListening() const { return m_appCaptureListening; }
    // Whether application key capture in exclusive (consume) mode is active.
    // 应用程序按键捕获（独占/吞没模式）是否激活。
    bool appCaptureExclusive() const { return m_appCaptureExclusive; }
    // Whether mouse capture (listen-only passthrough) is active.
    // 鼠标捕获（仅监听透传）是否激活。
    bool mouseCaptureListening() const { return m_mouseCaptureListening; }

    // Register a global hotkey from a human-readable string like "Ctrl+`".
    // 从人类可读的字符串（如 "Ctrl+`"）注册全局热键。
    Q_INVOKABLE void registerHotkey(const QString &keys);
    // Unregister the previously registered global hotkey.
    // 注销之前注册的全局热键。
    Q_INVOKABLE void unregisterHotkey();
    // Enable or disable application key capture in listen-only mode.
    // 启用或禁用应用程序按键捕获（仅监听模式）。
    Q_INVOKABLE void setAppCaptureListening(bool enabled);
    // Enable or disable application key capture in exclusive (consume) mode.
    // 启用或禁用应用程序按键捕获（独占/吞没模式）。
    Q_INVOKABLE void setAppCaptureExclusive(bool enabled);
    // Enable or disable mouse capture (listen-only passthrough).
    // 启用或禁用鼠标捕获（仅监听透传）。
    Q_INVOKABLE void setMouseCaptureListening(bool enabled);

    // QAbstractNativeEventFilter
    // Qt event filter for application-level keyboard and mouse events.
    // 应用程序级键盘和鼠标事件的 Qt 事件过滤器。
    bool eventFilter(QObject *watched, QEvent *event) override;
    // Platform-agnostic native event filter entry point.
    // 平台无关的原生事件过滤器入口。
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
    // Windows-specific native event filter handling WM_HOTKEY messages.
    // Windows 特定的原生事件过滤器，处理 WM_HOTKEY 消息。
    bool nativeEventFilterWin(const QByteArray &eventType, void *message, qintptr *result);
    // Non-Windows native event filter (currently a no-op).
    // 非 Windows 平台的原生事件过滤器（当前为空操作）。
    bool nativeEventFilterOther(const QByteArray &eventType, void *message, qintptr *result);

signals:
    void registeredChanged();
    void hotkeyChanged();
    void hotkeyTriggered();     // Ctrl+~ pressed
    void appCaptureModeChanged();
    void appKeyCaptured(const QString &key, const QString &text, bool pressed, bool autoRepeat, int modifiers);
    void appMouseCaptured(int buttons, int dx, int dy);
    void appMouseWheelCaptured(int delta);

private:
    // Install or remove the application event filter based on active capture modes.
    // 根据当前激活的捕获模式安装或移除应用程序事件过滤器。
    void updateApplicationEventFilter();

    bool m_registered = false;
    QString m_hotkey;           // e.g. "Ctrl+`"
    quint32 m_nativeId = 0;
    quint32 m_nativeMod = 0;
    bool m_appCaptureListening = false;
    bool m_appCaptureExclusive = false;
    bool m_mouseCaptureListening = false;
    bool m_appEventFilterInstalled = false;
    bool m_mousePositionValid = false;
    QPoint m_lastMouseGlobalPosition;
    quint8 m_mouseButtons = 0;
    int m_wheelAngleRemainder = 0;
    QSet<int> m_pressedKeys;

    // Win32: parse "Ctrl+Shift+X" into native key code + modifier flags.
    // Win32：将 "Ctrl+Shift+X" 解析为原生按键码和修饰符标志。
    bool parseToNative(const QString &keys, quint32 &outKey, quint32 &outMod);
};

#include "CaptureManager.h"
#include "DebugTrace.h"
#include <QCoreApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMouseEvent>
#include <QWheelEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
// Convert Qt mouse button flags to a bitmask matching HID mouse report format.
// 将 Qt 鼠标按钮标志转换为与 HID 鼠标报告格式匹配的位掩码。
quint8 mouseButtonsFromQt(Qt::MouseButtons buttons)
{
    quint8 result = 0;
    if (buttons.testFlag(Qt::LeftButton))
        result |= 0x01;
    if (buttons.testFlag(Qt::RightButton))
        result |= 0x02;
    if (buttons.testFlag(Qt::MiddleButton))
        result |= 0x04;
    return result;
}

// Extract the global screen position from a QMouseEvent in a Qt-version-safe way.
// 以兼容 Qt 版本的方式从 QMouseEvent 提取全局屏幕坐标。
QPoint globalMousePosition(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->globalPosition().toPoint();
#else
    return event->globalPos();
#endif
}
}

// Default constructor.
// 默认构造函数。
CaptureManager::CaptureManager(QObject *parent)
    : QObject(parent)
{
}

// Destructor; unregisters the hotkey and removes the application event filter.
// 析构函数；注销热键并移除应用程序事件过滤器。
CaptureManager::~CaptureManager()
{
    unregisterHotkey();
    if (m_appEventFilterInstalled && qApp)
        qApp->removeEventFilter(this);
}

// Register a system-wide global hotkey from a human-readable key string.
// 从人类可读的按键字符串注册系统级全局热键。
void CaptureManager::registerHotkey(const QString &keys)
{
    unregisterHotkey();
    m_hotkey = keys;
    emit hotkeyChanged();

#ifdef Q_OS_WIN
    quint32 vk = 0, mod = 0;
    if (!parseToNative(keys, vk, mod)) {
        qWarning() << "[CaptureManager] Cannot parse hotkey:" << keys;
        return;
    }
    m_nativeId = 1;
    m_nativeMod = mod;
    if (::RegisterHotKey(nullptr, m_nativeId, mod, vk)) {
        m_registered = true;
        if (DebugTrace::enabled()) qDebug() << "[CaptureManager] Registered hotkey:" << keys << "→ VK=" << vk << "MOD=" << mod;
    } else {
        qWarning() << "[CaptureManager] RegisterHotKey failed for:" << keys << "err=" << GetLastError();
    }
#else
    m_registered = false; // other platforms later
    if (DebugTrace::enabled()) qDebug() << "[CaptureManager] Hotkey registration not yet implemented on this platform";
#endif
    emit registeredChanged();
}

// Unregister the previously registered system-wide global hotkey.
// 注销之前注册的系统级全局热键。
void CaptureManager::unregisterHotkey()
{
    if (!m_registered) return;
#ifdef Q_OS_WIN
    ::UnregisterHotKey(nullptr, m_nativeId);
#endif
    m_registered = false;
    emit registeredChanged();
}

// Enable or disable application key capture in listen-only mode.
// 启用或禁用应用程序按键捕获（仅监听模式）。
void CaptureManager::setAppCaptureListening(bool enabled)
{
    if (m_appCaptureListening == enabled && !m_appCaptureExclusive)
        return;

    m_appCaptureListening = enabled;
    if (enabled)
        m_appCaptureExclusive = false;
    updateApplicationEventFilter();
    if (!m_appCaptureListening && !m_appCaptureExclusive)
        m_pressedKeys.clear();
    emit appCaptureModeChanged();
}

// Enable or disable application key capture in exclusive (consume) mode.
// 启用或禁用应用程序按键捕获（独占/吞没模式）。
void CaptureManager::setAppCaptureExclusive(bool enabled)
{
    if (m_appCaptureExclusive == enabled)
        return;

    m_appCaptureExclusive = enabled;
    if (enabled)
        m_appCaptureListening = false;
    updateApplicationEventFilter();
    if (!m_appCaptureListening && !m_appCaptureExclusive)
        m_pressedKeys.clear();
    emit appCaptureModeChanged();
}

// Enable or disable mouse capture (listen-only passthrough).
// 启用或禁用鼠标捕获（仅监听透传）。
void CaptureManager::setMouseCaptureListening(bool enabled)
{
    if (m_mouseCaptureListening == enabled)
        return;

    m_mouseCaptureListening = enabled;
    m_mousePositionValid = false;
    m_mouseButtons = 0;
    m_wheelAngleRemainder = 0;
    updateApplicationEventFilter();
    emit appCaptureModeChanged();
}

// Install or remove the application event filter based on active capture modes.
// 根据当前激活的捕获模式安装或移除应用程序事件过滤器。
void CaptureManager::updateApplicationEventFilter()
{
    if (!qApp)
        return;

    const bool shouldInstall = m_appCaptureListening || m_appCaptureExclusive || m_mouseCaptureListening;
    if (shouldInstall && !m_appEventFilterInstalled) {
        qApp->installEventFilter(this);
        m_appEventFilterInstalled = true;
    } else if (!shouldInstall && m_appEventFilterInstalled) {
        qApp->removeEventFilter(this);
        m_appEventFilterInstalled = false;
    }
}

// Qt event filter for intercepting application-level keyboard and mouse events.
// 拦截应用程序级键盘和鼠标事件的 Qt 事件过滤器。
bool CaptureManager::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if (!m_appCaptureListening && !m_appCaptureExclusive && !m_mouseCaptureListening)
        return false;

    if (m_mouseCaptureListening
        && (event->type() == QEvent::ApplicationDeactivate
            || event->type() == QEvent::WindowDeactivate
            || event->type() == QEvent::UngrabMouse)) {
        m_mousePositionValid = false;
        m_wheelAngleRemainder = 0;
        if (m_mouseButtons != 0) {
            m_mouseButtons = 0;
            emit appMouseCaptured(0, 0, 0);
        }
        return false;
    }

    if (m_mouseCaptureListening && event->type() == QEvent::Wheel) {
        auto *wheelEvent = static_cast<QWheelEvent *>(event);
        // HID wheel reports use detents, while Qt exposes angle units where a
        // normal wheel notch is 120. Preserve high-resolution wheel deltas
        // until they add up to one detent instead of dropping them.
        m_wheelAngleRemainder += wheelEvent->angleDelta().y();
        const int detents = m_wheelAngleRemainder / 120;
        m_wheelAngleRemainder -= detents * 120;
        if (detents != 0) {
            emit appMouseWheelCaptured(detents);
            if (DebugTrace::enabled())
                qDebug() << "[CaptureManager] app mouse wheel captured" << "delta=" << detents;
        }
        // Mouse passthrough is listen-only. Never consume the local event.
        return false;
    }

    if (m_mouseCaptureListening
        && (event->type() == QEvent::MouseMove
            || event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseButtonRelease
            || event->type() == QEvent::MouseButtonDblClick)) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->source() != Qt::MouseEventNotSynthesized)
            return false;
        const QPoint currentPosition = globalMousePosition(mouseEvent);
        int dx = 0;
        int dy = 0;
        if (m_mousePositionValid) {
            dx = currentPosition.x() - m_lastMouseGlobalPosition.x();
            dy = currentPosition.y() - m_lastMouseGlobalPosition.y();
        }
        m_lastMouseGlobalPosition = currentPosition;
        m_mousePositionValid = true;

        const quint8 buttons = mouseButtonsFromQt(mouseEvent->buttons());
        if (dx != 0 || dy != 0 || buttons != m_mouseButtons) {
            m_mouseButtons = buttons;
            emit appMouseCaptured(static_cast<int>(buttons), dx, dy);
            if (DebugTrace::enabled()) {
                qDebug() << "[CaptureManager] app mouse captured"
                         << "buttons=" << buttons << "dx=" << dx << "dy=" << dy;
            }
        }
        // Mouse passthrough is listen-only. Never consume the local event.
        return false;
    }

    if (!m_appCaptureListening && !m_appCaptureExclusive)
        return false;

    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
        return false;

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    const bool pressed = event->type() == QEvent::KeyPress;
    const int keyCode = keyEvent->key();

    if (!keyEvent->isAutoRepeat()) {
        if (pressed) {
            if (m_pressedKeys.contains(keyCode))
                return m_appCaptureExclusive;
            m_pressedKeys.insert(keyCode);
        } else {
            if (!m_pressedKeys.contains(keyCode))
                return m_appCaptureExclusive;
            m_pressedKeys.remove(keyCode);
        }
    }

    const QString key = QKeySequence(keyEvent->key()).toString(QKeySequence::PortableText);
    emit appKeyCaptured(key, keyEvent->text(), pressed, keyEvent->isAutoRepeat(), static_cast<int>(keyEvent->modifiers()));

    if (DebugTrace::enabled()) {
        qDebug() << "[CaptureManager] app key captured"
                 << "key=" << key
                 << "text=" << keyEvent->text()
                 << "pressed=" << pressed
                 << "exclusive=" << m_appCaptureExclusive;
    }

    return m_appCaptureExclusive;
}

// Platform-agnostic native event filter entry point; dispatches to platform-specific implementation.
// 平台无关的原生事件过滤器入口；分派到平台特定实现。
bool CaptureManager::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
#ifdef Q_OS_WIN
    return nativeEventFilterWin(eventType, message, result);
#else
    return nativeEventFilterOther(eventType, message, result);
#endif
}

// Windows-specific native event filter handling WM_HOTKEY messages.
// Windows 特定的原生事件过滤器，处理 WM_HOTKEY 消息。
bool CaptureManager::nativeEventFilterWin(const QByteArray &eventType, void *message, qintptr *result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG *msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY && msg->wParam == m_nativeId) {
            if (DebugTrace::enabled()) qDebug() << "[CaptureManager] Hotkey triggered!";
            emit hotkeyTriggered();
            *result = 0;
            return true;
        }
    }
#else
    Q_UNUSED(eventType); Q_UNUSED(message); Q_UNUSED(result);
#endif
    return false;
}

// Non-Windows native event filter (currently a no-op).
// 非 Windows 平台的原生事件过滤器（当前为空操作）。
bool CaptureManager::nativeEventFilterOther(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType); Q_UNUSED(message); Q_UNUSED(result);
    return false;
}

// Parse a human-readable key string (e.g. "Ctrl+Shift+X") into Win32 virtual key and modifier.
// 将人类可读的按键字符串（如 "Ctrl+Shift+X"）解析为 Win32 虚拟键码和修饰符。
bool CaptureManager::parseToNative(const QString &keys, quint32 &outKey, quint32 &outMod)
{
#ifdef Q_OS_WIN
    outMod = 0;
    outKey = 0;

    QStringList parts = keys.split("+");
    for (const QString &part : parts) {
        QString p = part.trimmed().toLower();
        if (p == "ctrl" || p == "control")  outMod |= MOD_CONTROL;
        else if (p == "alt")                outMod |= MOD_ALT;
        else if (p == "shift")              outMod |= MOD_SHIFT;
        else if (p == "win" || p == "meta") outMod |= MOD_WIN;
        else {
            // key char
            if (p == "`" || p == "~" || p == "backquote" || p == "tilde") {
                outKey = VK_OEM_3; // US keyboard: ` / ~ key
            } else if (p.length() == 1) {
                QChar c = p.at(0);
                outKey = VkKeyScanW(c.unicode());
                if (outKey == 0xFFFF) return false;
            } else if (p.startsWith("f") && p.length() <= 3) {
                bool ok;
                int fn = p.mid(1).toInt(&ok);
                if (ok && fn >= 1 && fn <= 24) outKey = VK_F1 + fn - 1;
                else return false;
            } else {
                return false;
            }
        }
    }
    return outKey != 0;
#else
    Q_UNUSED(keys); outKey = 0; outMod = 0;
    return false;
#endif
}

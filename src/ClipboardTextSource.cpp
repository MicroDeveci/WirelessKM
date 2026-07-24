#include "ClipboardTextSource.h"
#include <QClipboard>
#include <QGuiApplication>

// Construct and connect to the system clipboard data-changed signal.
// 构造并连接系统剪贴板数据变更信号。
ClipboardTextSource::ClipboardTextSource(QObject *parent)
    : QObject(parent)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard)
        connect(clipboard, &QClipboard::dataChanged, this, &ClipboardTextSource::onClipboardChanged);
}

// Request the current clipboard text; emits textProduced or emptyText.
// 请求当前剪贴板文本；发出 textProduced 或 emptyText。
void ClipboardTextSource::requestText()
{
    const QString text = currentClipboardText();
    if (text.isEmpty()) {
        emit emptyText();
        return;
    }

    emit textProduced(text, "clipboard");
}

// Enable or disable continuous clipboard watch mode.
// 启用或禁用持续剪贴板监视模式。
void ClipboardTextSource::setWatching(bool watching)
{
    if (m_watching == watching)
        return;

    m_watching = watching;
    m_lastText = currentClipboardText();
    emit watchingChanged();
}

// Handle clipboard data-changed signal; emit new text if watch mode is active.
// 处理剪贴板数据变更信号；若监视模式激活则发出新文本。
void ClipboardTextSource::onClipboardChanged()
{
    if (!m_watching)
        return;

    const QString text = currentClipboardText();
    if (text.isEmpty() || text == m_lastText)
        return;

    m_lastText = text;
    emit textProduced(text, "clipboard.watch");
}

// Read the current text from the system clipboard.
// 从系统剪贴板读取当前文本。
QString ClipboardTextSource::currentClipboardText()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        emit errorOccurred("Clipboard is not available");
        return QString();
    }
    return clipboard->text(QClipboard::Clipboard);
}

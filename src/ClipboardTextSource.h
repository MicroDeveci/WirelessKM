#pragma once

#include <QObject>
#include <QString>

class ClipboardTextSource : public QObject
{
    Q_OBJECT

public:
    // Construct and connect to the system clipboard data-changed signal.
    // 构造并连接系统剪贴板数据变更信号。
    explicit ClipboardTextSource(QObject *parent = nullptr);

    // Whether clipboard watch mode is currently active.
    // 剪贴板监视模式是否当前激活。
    bool watching() const { return m_watching; }
    // Request the current clipboard text and emit it via textProduced.
    // 请求当前剪贴板文本并通过 textProduced 发出。
    void requestText();
    // Enable or disable continuous clipboard watch mode.
    // 启用或禁用持续剪贴板监视模式。
    void setWatching(bool watching);

signals:
    void watchingChanged();
    void textProduced(const QString &text, const QString &source);
    void emptyText();
    void errorOccurred(const QString &error);

private slots:
    // Handle clipboard data-changed signal; emit text if watch mode is active.
    // 处理剪贴板数据变更信号；若监视模式激活则发出文本。
    void onClipboardChanged();

private:
    // Read the current text from the system clipboard.
    // 从系统剪贴板读取当前文本。
    QString currentClipboardText();

    bool m_watching = false;
    QString m_lastText;
};

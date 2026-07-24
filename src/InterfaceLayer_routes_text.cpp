#include "InterfaceLayer.h"
#include "TextInputSource.h"

// Register text input command routes (send text from QML UI)
// 注册文本输入相关的命令路由（从QML界面发送文本）
void InterfaceLayer::registerTextRoutes()
{
    m_routes["text.send"] = [this](const QVariantMap &p) {
        QString text = p["text"].toString();
        traceDataFlow("route text.send", "send text requested",
                      {{"length", text.length()}, {"preview", text.left(40)}, {"source", "qml.text"}});
        m_textInputSource->enqueueText(text, "qml.text");
        appendLog(0, "TextInput", "Text queued from UI: " + QString::number(text.length()) + " chars");
    };
}

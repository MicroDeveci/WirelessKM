import QtQuick 2.15
import QtQuick.Window 2.15
import FluentUI 1.0

FluWindow {
    width: 1280
    height: 640
    minimumWidth: 320
    minimumHeight: 320
    title: qsTr("Wireless KM")
    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"

    property string pendingPageUrl: ""

    Connections {
        target: typeof InterfaceLayer !== "undefined" ? InterfaceLayer : null
        function onDarkModeChanged() {
            if (typeof InterfaceLayer !== "undefined")
                FluTheme.darkMode = InterfaceLayer.darkMode
        }
    }
    Component.onCompleted: {
        if (typeof InterfaceLayer !== "undefined")
            FluTheme.darkMode = InterfaceLayer.darkMode
    }

    function smoothPush(url) {
        pendingPageUrl = url
        pageSwitchAnim.restart()
    }

    FluNavigationView {
        id: nav_view
        anchors.fill: parent
        pageMode: FluNavigationViewType.NoStack
        displayMode: FluNavigationViewType.Auto
        Component.onCompleted: push("qrc:/home.qml")

        SequentialAnimation {
            id: pageSwitchAnim
            running: false
            NumberAnimation { target: nav_view; property: "opacity"; from: 1; to: 0.86; duration: 90; easing.type: Easing.InOutQuad }
            ScriptAction {
                script: {
                    if (pendingPageUrl !== "") {
                        nav_view.push(pendingPageUrl)
                    }
                }
            }
            NumberAnimation { target: nav_view; property: "opacity"; from: 0.86; to: 1; duration: 150; easing.type: Easing.OutCubic }
        }

        // ====== 主导航（从上到下）======
        items: FluObject {
            FluPaneItem {
                title: english ? qsTr("Home") : qsTr("首页")
                icon: FluentIcons.Home
                onTap: { smoothPush("qrc:/home.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Clipboard") : qsTr("剪切板读取")
                icon: FluentIcons.Paste
                onTap: { smoothPush("qrc:/key_setting.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Devices") : qsTr("链接设备")
                icon: FluentIcons.Devices
                onTap: { smoothPush("qrc:/device_link.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Text input") : qsTr("文本输入")
                icon: FluentIcons.Message
                onTap: { smoothPush("qrc:/text_input.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Passthrough") : qsTr("Passthrough")
                icon: FluentIcons.KeyboardSettings
                onTap: { smoothPush("qrc:/passthrough.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Touchpad") : qsTr("触摸板")
                icon: FluentIcons.Touchpad
                onTap: { smoothPush("qrc:/touchpad.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Auto clicker") : qsTr("连点器")
                icon: FluentIcons.KeyboardSettings
                onTap: { smoothPush("qrc:/auto_clicker.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Logs") : qsTr("日志")
                icon: FluentIcons.Bug
                onTap: { smoothPush("qrc:/log.qml") }
            }

        }

        // ====== 底部导航（从下到上）======
        footerItems: FluObject {
            FluPaneItemSeparator {}
            FluPaneItem {
                title: english ? qsTr("About") : qsTr("关于")
                icon: FluentIcons.Contact
                onTap: { smoothPush("qrc:/about.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Thanks") : qsTr("赞助和鸣谢")
                icon: FluentIcons.FavoriteStar
                onTap: { smoothPush("qrc:/thanks.qml") }
            }
            FluPaneItem {
                title: english ? qsTr("Settings") : qsTr("设置")
                icon: FluentIcons.Settings
                onTap: { smoothPush("qrc:/settings.qml") }
            }
        }
    }
}

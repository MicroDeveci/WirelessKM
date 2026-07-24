import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"
    title: english ? qsTr("About") : qsTr("关于")

    function d(command, params) {
        if (typeof InterfaceLayer !== "undefined")
            InterfaceLayer.dispatch(command, params)
    }

    Loader {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        asynchronous: false
        sourceComponent: Component {
            ColumnLayout {
                width: 680
                spacing: 16

                QtObject {
                    id: colors
                    readonly property color card: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color border: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color muted: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                }

                Rectangle {
                    Layout.fillWidth: true
                    radius: 16
                    color: colors.card
                    border.color: colors.border
                    border.width: 1
                    implicitHeight: content.implicitHeight + 48

                    ColumnLayout {
                        id: content
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 12

                        RowLayout {
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.Contact; iconSize: 30; iconColor: FluTheme.primaryColor }
                            ColumnLayout {
                                spacing: 2
                                FluText { text: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.appName : qsTr("Wireless KM"); font: FluTextStyle.Subtitle }
                                FluText { text: (english ? qsTr("Version %1") : qsTr("版本 %1")).arg(typeof InterfaceLayer !== "undefined" ? InterfaceLayer.appVersion : "v1.0.0"); font: FluTextStyle.Caption; textColor: colors.muted }
                            }
                        }

                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: colors.border }
                        FluText { text: english ? qsTr("What it does") : qsTr("功能介绍"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                        FluText {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            textColor: colors.muted
                            text: english
                                ? qsTr("Wireless KM is a desktop controller for an ESP32-S3 HID bridge. It sends keyboard and mouse input from this computer to another host connected to the ESP32-S3.")
                                : qsTr("Wireless KM 是 ESP32-S3 HID 桥接器的桌面控制端，可将本机的键盘和鼠标输入发送到连接 ESP32-S3 的另一台主机。")
                        }
                        FluText { text: english ? qsTr("Connection methods") : qsTr("连接方式"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor; Layout.topMargin: 4 }
                        FluText {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            textColor: colors.muted
                            text: english
                                ? qsTr("BLE, UART, and HUDP are supported. Text input, foreground key capture, a touchpad, and the auto clicker all use the same outgoing input queue.")
                                : qsTr("支持 BLE、UART 和 HUDP。文本输入、前台按键捕获、触控板和自动点击器均通过统一的输入队列发送。")
                        }
                        FluText { text: english ? qsTr("License") : qsTr("开源协议"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor; Layout.topMargin: 4 }
                        FluText { text: qsTr("GPL-3.0"); textColor: colors.muted }

                        RowLayout {
                            Layout.topMargin: 6
                            spacing: 12
                            FluFilledButton { text: english ? qsTr("Check for updates") : qsTr("检查更新"); onClicked: d("app.checkUpdate", {}) }
                            FluButton { text: qsTr("GitHub"); onClicked: d("app.openGitHub", {}) }
                        }
                    }
                }
            }
        }
    }
}

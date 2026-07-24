import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    title: english ? qsTr("Text input") : qsTr("文本输入")

    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"

    function d(cmd, params) {
        if (typeof InterfaceLayer !== "undefined") InterfaceLayer.dispatch(cmd, params)
    }

    Loader {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        asynchronous: true
        sourceComponent: Component {
            ColumnLayout {
                width: 640
                spacing: 20

                QtObject {
                    id: c
                    readonly property color cardBg: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color cardBorder: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color mutedText: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                    readonly property color green: "#10B981"
                    readonly property color red: "#EF4444"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 24
                        anchors.rightMargin: 24
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 44
                            Layout.preferredHeight: 44
                            radius: 10
                            color: Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.12)
                            FluIcon { anchors.centerIn: parent; iconSource: FluentIcons.Message; iconSize: 22; iconColor: FluTheme.primaryColor }
                        }

                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            FluText { text: english ? qsTr("Text input") : qsTr("文本输入"); font: FluTextStyle.Title; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Send keyboard input via connected BLE or UART device") : qsTr("通过当前已连接的 BLE 或 UART 设备发送键盘输入"); font: FluTextStyle.Caption; textColor: c.mutedText }
                        }

                        Rectangle {
                            Layout.preferredWidth: 72
                            Layout.preferredHeight: 28
                            radius: 14
                            color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? Qt.rgba(0.063, 0.725, 0.506, 0.10) : Qt.rgba(0.937, 0.267, 0.267, 0.08)
                            border.color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? Qt.rgba(0.063, 0.725, 0.506, 0.30) : Qt.rgba(0.937, 0.267, 0.267, 0.24)
                            border.width: 1
                            FluText {
                                anchors.centerIn: parent
                                text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? (english ? qsTr("Connected") : qsTr("已连接")) : (english ? qsTr("Disconnected") : qsTr("未连接"))
                                font: FluTextStyle.Caption
                                textColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? c.green : c.red
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    radius: 10
                    color: Qt.rgba(0.063, 0.725, 0.506, 0.08)
                    border.color: Qt.rgba(0.063, 0.725, 0.506, 0.25)
                    border.width: 1
                    visible: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.lastSentText !== "" : false

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 10
                        FluIcon { iconSource: FluentIcons.CompletedSolid; iconSize: 18; iconColor: c.green }
                        FluText {
                            Layout.fillWidth: true
                            text: (english ? qsTr("Sent - ") : qsTr("已发送 - ")) + (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.lastSentText : "")
                            font: FluTextStyle.Caption
                            textColor: c.green
                            elide: Text.ElideRight
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 240
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 12

                        FluText { text: english ? qsTr("Input content") : qsTr("输入内容"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 8
                            color: FluTheme.dark ? "#1A1A2E" : "#F9FAFB"
                            border.color: FluTheme.dark ? "#333355" : "#E5E7EB"
                            border.width: 1

                            TextArea {
                                id: textArea
                                anchors.fill: parent
                                anchors.margins: 6
                                color: FluTheme.fontPrimaryColor
                                font.pixelSize: 14
                                placeholderText: english ? qsTr("Type text to send to target device...") : qsTr("在此输入要发送到目标设备的文本...")
                                wrapMode: TextArea.Wrap
                                background: Item {}
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Item { Layout.fillWidth: true }
                    FluText { text: textArea.length + (english ? qsTr(" chars") : qsTr(" 字符")); font: FluTextStyle.Caption; textColor: c.mutedText; Layout.alignment: Qt.AlignVCenter }
                    FluButton { text: english ? qsTr("Clear") : qsTr("清空"); visible: textArea.text !== ""; onClicked: textArea.clear() }
                    FluButton {
                        text: english ? qsTr("Send Clipboard") : qsTr("发送剪切板")
                        enabled: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) && !InterfaceLayer.sending
                        onClicked: d("clipboard.send", {})
                    }
                    FluFilledButton {
                        text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.sending) ? (english ? qsTr("Sending...") : qsTr("发送中...")) : (english ? qsTr("Send") : qsTr("发送"))
                        enabled: textArea.text !== "" && (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) && !InterfaceLayer.sending
                        onClicked: {
                            d("text.send", { text: textArea.text })
                            textArea.clear()
                        }
                    }
                }
            }
        }
    }
}

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    title: english ? qsTr("Clipboard") : qsTr("剪切板读取")

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
                width: 680
                spacing: 14

                QtObject {
                    id: c
                    readonly property color cardBg: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color cardBorder: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color mutedText: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 76
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 24
                        anchors.rightMargin: 24
                        spacing: 12
                        FluIcon { iconSource: FluentIcons.Paste; iconSize: 24; iconColor: FluTheme.primaryColor }
                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            FluText { text: english ? qsTr("Clipboard") : qsTr("剪切板读取"); font: FluTextStyle.Title; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Read the first text from clipboard after copy, validate and enqueue") : qsTr("复制文本后读取第一条文本，校验后进入统一输入队列"); font: FluTextStyle.Caption; textColor: c.mutedText }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    ColumnLayout {
                        id: clipboardContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 14

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.Paste; iconSize: 22; iconColor: FluTheme.primaryColor }
                            Column {
                                Layout.fillWidth: true
                                spacing: 2
                                FluText { text: english ? qsTr("Manual read") : qsTr("手动读取"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                                FluText { text: english ? qsTr("Read text from system clipboard and send") : qsTr("读取当前系统剪切板中的文本并发送"); font: FluTextStyle.Caption; textColor: c.mutedText }
                            }
                            FluFilledButton {
                                text: english ? qsTr("Read and send") : qsTr("读取并发送")
                                onClicked: d("clipboard.send", {})
                            }
                        }

                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: c.cardBorder }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.View; iconSize: 22; iconColor: FluTheme.fontPrimaryColor }
                            Column {
                                Layout.fillWidth: true
                                spacing: 2
                                FluText { text: english ? qsTr("Watch clipboard") : qsTr("监听剪切板"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                                FluText { text: english ? qsTr("When enabled, only newly copied non-empty text is processed; duplicates are ignored") : qsTr("开启后只处理新复制的非空文本，重复内容会被忽略"); font: FluTextStyle.Caption; textColor: c.mutedText }
                            }
                            FluToggleSwitch {
                                checked: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.clipboardWatching : false
                                onCheckedChanged: d("clipboard.watchToggled", { enabled: checked })
                            }
                        }
                    }
                    Layout.preferredHeight: clipboardContent.height + 48
                }
            }
        }
    }
}

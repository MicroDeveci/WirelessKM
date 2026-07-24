import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    title: english ? qsTr("Passthrough") : qsTr("Passthrough")

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
                spacing: 14

                QtObject {
                    id: c
                    readonly property color cardBg: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color cardBorder: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color mutedText: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                    readonly property color active: "#10B981"
                    readonly property color idle: "#6B7280"
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
                            FluIcon { anchors.centerIn: parent; iconSource: FluentIcons.KeyboardSettings; iconSize: 22; iconColor: FluTheme.primaryColor }
                        }

                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            FluText { text: english ? qsTr("Passthrough") : qsTr("Passthrough"); font: FluTextStyle.Title; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Capture foreground keyboard and mouse") : qsTr("应用前台键盘与鼠标捕获"); font: FluTextStyle.Caption; textColor: c.mutedText }
                        }

                        Rectangle {
                            Layout.preferredWidth: 104
                            Layout.preferredHeight: 28
                            radius: 14
                            color: (typeof InterfaceLayer !== "undefined" && (InterfaceLayer.passthroughListening || InterfaceLayer.passthroughExclusive || InterfaceLayer.passthroughMouseListening)) ? Qt.rgba(0.063, 0.725, 0.506, 0.10) : Qt.rgba(0.42, 0.45, 0.50, 0.10)
                            border.color: (typeof InterfaceLayer !== "undefined" && (InterfaceLayer.passthroughListening || InterfaceLayer.passthroughExclusive || InterfaceLayer.passthroughMouseListening)) ? Qt.rgba(0.063, 0.725, 0.506, 0.30) : Qt.rgba(0.42, 0.45, 0.50, 0.25)
                            border.width: 1
                            FluText {
                                anchors.centerIn: parent
                                text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.passthroughMouseListening)
                                    ? ((InterfaceLayer.passthroughListening || InterfaceLayer.passthroughExclusive) ? (english ? qsTr("Keyboard & Mouse") : qsTr("键鼠监听")) : (english ? qsTr("Mouse") : qsTr("鼠标监听")))
                                    : (typeof InterfaceLayer !== "undefined" && InterfaceLayer.passthroughExclusive)
                                        ? (english ? qsTr("Exclusive") : qsTr("独占模式"))
                                        : ((typeof InterfaceLayer !== "undefined" && InterfaceLayer.passthroughListening) ? (english ? qsTr("Listening") : qsTr("监听中")) : (english ? qsTr("Off") : qsTr("关闭")))
                                font: FluTextStyle.Caption
                                textColor: (typeof InterfaceLayer !== "undefined" && (InterfaceLayer.passthroughListening || InterfaceLayer.passthroughExclusive || InterfaceLayer.passthroughMouseListening)) ? c.active : c.idle
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: modeContent.height + 28
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1

                    ColumnLayout {
                        id: modeContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 20
                        spacing: 2

                        FluText { text: english ? qsTr("Mode") : qsTr("模式"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 56
                            radius: 8
                            color: "transparent"
                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                FluIcon { iconSource: FluentIcons.View; iconSize: 18; iconColor: FluTheme.fontPrimaryColor }
                                Column {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    FluText { text: english ? qsTr("Listen mode") : qsTr("监听模式"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                                    FluText { text: english ? qsTr("Capture app foreground keys without blocking local input") : qsTr("捕获应用前台按键，不阻断本机输入"); font: FluTextStyle.Caption; textColor: c.mutedText }
                                }
                                FluToggleSwitch {
                                    disabled: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.passthroughExclusive : false
                                    checked: typeof InterfaceLayer !== "undefined" ? (InterfaceLayer.passthroughListening && !InterfaceLayer.passthroughExclusive) : false
                                    onCheckedChanged: {
                                        if (!disabled) d("passthrough.listenToggled", { enabled: checked })
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 56
                            radius: 8
                            color: "transparent"
                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                FluIcon { iconSource: FluentIcons.Blocked2; iconSize: 18; iconColor: FluTheme.fontPrimaryColor }
                                Column {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    FluText { text: english ? qsTr("App exclusive mode") : qsTr("应用独占模式"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                                    FluText { text: english ? qsTr("Capture app foreground keys and block them inside this app") : qsTr("捕获应用前台按键并在本应用内阻断"); font: FluTextStyle.Caption; textColor: c.mutedText }
                                }
                                FluToggleSwitch {
                                    checked: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.passthroughExclusive : false
                                    onCheckedChanged: d("passthrough.exclusiveToggled", { enabled: checked })
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 64
                            radius: 8
                            color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.passthroughMouseListening)
                                ? Qt.rgba(0.063, 0.725, 0.506, 0.06) : "transparent"
                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                FluIcon {
                                    iconSource: FluentIcons.Mouse
                                    iconSize: 18
                                    iconColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.passthroughMouseListening) ? c.active : FluTheme.fontPrimaryColor
                                }
                                Column {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    FluText { text: english ? qsTr("Mouse event listening") : qsTr("鼠标事件监听"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                                    FluText { text: english ? qsTr("Capture app foreground relative movement and left/right button press/release, without blocking local input") : qsTr("捕获应用前台的相对移动及左、右键按下/释放，不阻断本机输入"); font: FluTextStyle.Caption; textColor: c.mutedText }
                                }
                                FluToggleSwitch {
                                    disabled: typeof InterfaceLayer !== "undefined" ? !InterfaceLayer.connected : true
                                    checked: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.passthroughMouseListening : false
                                    clickListener: function() {
                                        d("passthrough.mouseListenToggled", { enabled: !checked })
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: warningContent.height + 24
                    radius: 10
                    color: Qt.rgba(0.961, 0.620, 0.043, 0.08)
                    border.color: Qt.rgba(0.961, 0.620, 0.043, 0.28)
                    border.width: 1
                    RowLayout {
                        id: warningContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 10
                        FluIcon { iconSource: FluentIcons.Warning; iconSize: 18; iconColor: "#F59E0B" }
                        FluText {
                            Layout.fillWidth: true
                            text: english ? qsTr("Mouse listening controls another device. If ESP32 HID mouse connects back to this PC, captured re-injected events may form a loop. Do not enable in that topology.") : qsTr("鼠标监听用于控制另一台设备。若 ESP32 HID 鼠标回连本机，捕获到回注事件可能形成循环，请勿在该拓扑下启用。")
                            wrapMode: Text.WordWrap
                            font: FluTextStyle.Caption
                            textColor: FluTheme.dark ? "#FCD34D" : "#92400E"
                        }
                    }
                }
            }
        }
    }
}

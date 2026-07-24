import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    title: english ? qsTr("Home") : qsTr("首页")

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
                spacing: 24

                QtObject {
                    id: c
                    readonly property color cardBg: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color cardBorder: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color mutedText: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                    readonly property color green: "#10B981"
                    readonly property color red: "#EF4444"
                    readonly property color blue: "#3B82F6"
                    readonly property color purple: "#8B5CF6"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        radius: 16
                        color: c.cardBg
                        border.color: c.cardBorder
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 20
                            anchors.rightMargin: 20
                            anchors.topMargin: 20
                            anchors.bottomMargin: 20
                            spacing: 12

                            RowLayout {
                                spacing: 10
                                Rectangle {
                                    Layout.preferredWidth: 40
                                    Layout.preferredHeight: 40
                                    radius: 10
                                    color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? Qt.rgba(0.063, 0.725, 0.506, 0.12) : Qt.rgba(0.937, 0.267, 0.267, 0.12)
                                    FluIcon {
                                        anchors.centerIn: parent
                                        iconSource: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? FluentIcons.Connect : FluentIcons.DisconnectDrive
                                        iconSize: 20
                                        iconColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? c.green : c.red
                                    }
                                }
                                Column {
                                    spacing: 2
                                    FluText { text: english ? qsTr("Connection status") : qsTr("连接状态"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                                    FluText { text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? (english ? qsTr("System online") : qsTr("系统在线")) : (english ? qsTr("Waiting for connection") : qsTr("等待连接")); font: FluTextStyle.Caption; textColor: c.mutedText }
                                }
                            }

                            RowLayout {
                                Layout.topMargin: 4
                                spacing: 10

                                Rectangle {
                                    Layout.preferredWidth: 12
                                    Layout.preferredHeight: 12
                                    radius: 6
                                    color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? c.green : c.red
                                    SequentialAnimation on opacity {
                                        running: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected)
                                        loops: Animation.Infinite
                                        PropertyAnimation { from: 1.0; to: 0.3; duration: 800 }
                                        PropertyAnimation { from: 0.3; to: 1.0; duration: 800 }
                                    }
                                }
                                FluText { text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? (english ? qsTr("Connected") : qsTr("已连接")) : (english ? qsTr("Disconnected") : qsTr("未连接")); font: FluTextStyle.Subtitle; textColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? c.green : c.red }
                            }

                            Item { Layout.fillHeight: true; Layout.preferredHeight: 8 }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        radius: 16
                        color: c.cardBg
                        border.color: c.cardBorder
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 20
                            anchors.rightMargin: 20
                            anchors.topMargin: 20
                            anchors.bottomMargin: 20
                            spacing: 8

                            Rectangle {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                radius: 10
                                color: Qt.rgba(0.545, 0, 0.953, 0.12)
                                FluIcon { anchors.centerIn: parent; iconSource: FluentIcons.Bluetooth; iconSize: 20; iconColor: c.purple }
                            }
                            FluText { text: english ? qsTr("Current connection mode") : qsTr("当前连接模式"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.activeTab : "BLE"; font: FluTextStyle.Title; textColor: c.purple; Layout.topMargin: 4 }
                            Item { Layout.fillHeight: true }
                            Row {
                                spacing: 8
                                Repeater {
                                    model: ["BLE", "UART", "WiFi"]
                                    Rectangle {
                                        width: 6
                                        height: 6
                                        radius: 3
                                        color: modelData === (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.activeTab : "BLE") ? c.purple : (FluTheme.dark ? "#3F3F50" : "#D1D5DB")
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        radius: 16
                        color: c.cardBg
                        border.color: c.cardBorder
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 20
                            anchors.rightMargin: 20
                            anchors.topMargin: 20
                            anchors.bottomMargin: 20
                            spacing: 8

                            Rectangle {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                radius: 10
                                color: Qt.rgba(0.231, 0.509, 0.965, 0.12)
                                FluIcon { anchors.centerIn: parent; iconSource: FluentIcons.Devices; iconSize: 20; iconColor: c.blue }
                            }
                            FluText { text: english ? qsTr("Device info") : qsTr("设备信息"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.pairedDeviceName) ? InterfaceLayer.pairedDeviceName : "ESP32S3KB"; font: FluTextStyle.Subtitle; textColor: c.blue; Layout.topMargin: 4; elide: Text.ElideRight; Layout.fillWidth: true }
                            Item { Layout.fillHeight: true }
                            FluText { text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? qsTr("就绪") : qsTr("脱机"); font: FluTextStyle.Caption; textColor: c.mutedText }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    radius: 16
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 0

                        Repeater {
                            model: [
                                { value: "v1.0.0",  unit: english ? qsTr("Firmware version") : qsTr("固件版本"), icon: FluentIcons.Info,    clr: c.green,  bg: Qt.rgba(0.063, 0.725, 0.506, 0.1) },
                                { value: (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.connectedTimeStr : "00:00:00"), unit: english ? qsTr("Connection time") : qsTr("连接时长"), icon: FluentIcons.History, clr: c.purple, bg: Qt.rgba(0.545, 0, 0.953, 0.1) }
                            ]
                            Rectangle { visible: index > 0; Layout.preferredWidth: 1; Layout.preferredHeight: 48; color: c.cardBorder }
                            Item {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 12
                                    Rectangle {
                                        Layout.preferredWidth: 44
                                        Layout.preferredHeight: 44
                                        radius: 12
                                        color: modelData.bg
                                        FluIcon { anchors.centerIn: parent; iconSource: modelData.icon; iconSize: 22; iconColor: modelData.clr }
                                    }
                                    Column {
                                        spacing: 2
                                        FluText { text: modelData.value; font: FluTextStyle.Title; textColor: FluTheme.fontPrimaryColor }
                                        FluText { text: modelData.unit; font: FluTextStyle.Caption; textColor: c.mutedText }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

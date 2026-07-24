import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    title: english ? qsTr("Devices") : qsTr("连接设备")

    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"

    property string activeTab: "ble"
    property string hudpTarget: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.hudpTarget : ""
    property int hudpPort: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.hudpPort : 45820
    property bool hudpConnecting: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.hudpConnecting : false
    property string hudpStatus: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.hudpStatus : ""

    property bool bleScanning: typeof BleManager !== "undefined" && BleManager ? BleManager.scanning : false
    property bool bleConnecting: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.bleConnecting : false
    property bool bleConnected: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.bleConnected : false
    property string bleStatus: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.bleStatus : ""
    property var bleDeviceList: (typeof BleManager !== "undefined" && BleManager) ? BleManager.devices : []
    property string bleSelectedAddress: ""

    property string uartPort: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.uartPort : ""
    property int uartBaudrate: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.uartBaudrate : 115200
    property bool uartConnected: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.uartConnected : false
    property string uartStatus: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.uartStatus : ""
    property var uartPortList: (typeof SerialManager !== "undefined" && SerialManager) ? SerialManager.availablePorts : []

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
                width: 720
                spacing: 18

                QtObject {
                    id: c
                    readonly property color cardBg: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color cardBorder: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color mutedText: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                    readonly property color green: "#10B981"
                    readonly property color blue: "#3B82F6"
                    readonly property color purple: "#8B5CF6"
                    readonly property color red: "#EF4444"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    radius: 10
                    color: Qt.rgba(0.937, 0.267, 0.267, 0.06)
                    border.color: Qt.rgba(0.937, 0.267, 0.267, 0.2)
                    border.width: 1
                    visible: (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.connectionError : "") !== ""
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 10
                        FluIcon { iconSource: FluentIcons.StatusErrorFull; iconSize: 18; iconColor: c.red }
                        FluText { Layout.fillWidth: true; text: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.connectionError : ""; font: FluTextStyle.Caption; textColor: c.red; elide: Text.ElideRight }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    radius: 10
                    color: FluTheme.dark ? "#1A1A2E" : "#F9FAFB"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 4
                        spacing: 4

                        Repeater {
                            model: [
                                { key: "ble", label: english ? qsTr("Bluetooth BLE") : qsTr("蓝牙 BLE"), icon: FluentIcons.Bluetooth },
                                { key: "uart", label: english ? qsTr("Serial UART") : qsTr("串口 UART"), icon: FluentIcons.USB },
                                { key: "hudp", label: qsTr("HID UDP"), icon: FluentIcons.Globe }
                            ]

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 8
                                color: activeTab === modelData.key ? Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.12) : "transparent"
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: activeTab = modelData.key }
                                RowLayout {
                                    anchors.centerIn: parent
                                    spacing: 6
                                    FluIcon { iconSource: modelData.icon; iconSize: 16; iconColor: activeTab === modelData.key ? FluTheme.primaryColor : c.mutedText }
                                    FluText { text: modelData.label; font: FluTextStyle.Body; textColor: activeTab === modelData.key ? FluTheme.primaryColor : c.mutedText }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    visible: activeTab === "ble"
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1

                    ColumnLayout {
                        id: bleContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 14

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            Rectangle {
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                radius: 10
                                color: Qt.rgba(0.545, 0, 0.953, 0.12)
                                FluIcon { anchors.centerIn: parent; iconSource: FluentIcons.Bluetooth; iconSize: 20; iconColor: c.purple }
                            }
                            Column {
                                Layout.fillWidth: true
                                spacing: 2
                                FluText { text: english ? qsTr("Bluetooth BLE") : qsTr("蓝牙 BLE"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                                FluText { text: english ? qsTr("Scan BLE devices, connect to send from text input page") : qsTr("扫描 BLE 设备，连接后可直接在文本输入页发送"); font: FluTextStyle.Caption; textColor: c.mutedText }
                            }
                            FluButton {
                                text: bleScanning ? (english ? qsTr("Stop scan") : qsTr("停止扫描")) : (english ? qsTr("Scan devices") : qsTr("扫描设备"))
                                enabled: !bleConnecting && !bleConnected
                                onClicked: bleScanning ? d("ble.stopScan", {}) : d("ble.startScan", {})
                            }
                            FluButton {
                                text: english ? qsTr("Disconnect") : qsTr("断开")
                                visible: bleConnected || bleConnecting
                                onClicked: d("ble.disconnect", {})
                            }
                        }

                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: c.cardBorder }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            radius: 8
                            color: bleConnected ? Qt.rgba(0.063, 0.725, 0.506, 0.08) : (FluTheme.dark ? "#1A1A2E" : "#F9FAFB")
                            border.color: bleConnected ? Qt.rgba(0.063, 0.725, 0.506, 0.28) : c.cardBorder
                            border.width: 1
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 14
                                spacing: 10
                                Rectangle {
                                    Layout.preferredWidth: 8
                                    Layout.preferredHeight: 8
                                    radius: 4
                                    color: bleConnected ? c.green : (bleConnecting || bleScanning ? c.purple : c.mutedText)
                                }
                                FluText {
                                    Layout.fillWidth: true
                                    text: bleStatus !== "" ? bleStatus : (english ? qsTr("Disconnected") : qsTr("未连接"))
                                    font: FluTextStyle.Caption
                                    textColor: bleConnected ? c.green : c.mutedText
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 60
                            radius: 8
                            color: FluTheme.dark ? "#1A1A2E" : "#F9FAFB"
                            visible: bleScanning
                            RowLayout {
                                anchors.centerIn: parent
                                spacing: 12
                                FluProgressRing { indeterminate: true; Layout.preferredWidth: 24; Layout.preferredHeight: 24 }
                                FluText { text: english ? qsTr("Scanning for BLE devices...") : qsTr("正在扫描 BLE 设备..."); font: FluTextStyle.Body; textColor: c.mutedText }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 92
                            radius: 8
                            color: FluTheme.dark ? "#1A1A2E" : "#F9FAFB"
                            visible: !bleScanning && bleDeviceList.length === 0
                            Column {
                                anchors.centerIn: parent
                                spacing: 8
                                FluIcon { iconSource: FluentIcons.DeviceDiscovery; iconSize: 32; iconColor: c.mutedText; anchors.horizontalCenter: parent.horizontalCenter }
                                FluText { text: english ? qsTr("Click \"Scan devices\" to search for nearby BLE devices") : qsTr("点击「扫描设备」搜索附近 BLE 设备"); font: FluTextStyle.Caption; textColor: c.mutedText; anchors.horizontalCenter: parent.horizontalCenter }
                            }
                        }

                        Repeater {
                            model: bleDeviceList
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 56
                                radius: 8
                                color: bleSelectedAddress === modelData.address ? Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.08) : (FluTheme.dark ? "#1A1A2E" : "#F9FAFB")
                                border.color: bleSelectedAddress === modelData.address ? Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.32) : c.cardBorder
                                border.width: 1

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 14
                                    anchors.rightMargin: 14
                                    spacing: 12
                                    FluIcon { iconSource: FluentIcons.Devices; iconSize: 18; iconColor: c.purple }
                                    Column {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        FluText { text: modelData.name; font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor; elide: Text.ElideRight }
                                        FluText { text: modelData.address + "  RSSI " + modelData.rssi; font: FluTextStyle.Caption; textColor: c.mutedText; elide: Text.ElideRight }
                                    }
                                    FluButton {
                                        text: bleConnected && bleSelectedAddress === modelData.address ? (english ? qsTr("Connected") : qsTr("已连接")) : (bleConnecting && bleSelectedAddress === modelData.address ? (english ? qsTr("Connecting...") : qsTr("连接中...")) : (english ? qsTr("Connect") : qsTr("连接")))
                                        enabled: !bleConnecting && !(bleConnected && bleSelectedAddress === modelData.address)
                                        onClicked: {
                                            bleSelectedAddress = modelData.address
                                            d("ble.connect", { address: modelData.address })
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Layout.preferredHeight: bleContent.height + 48
                }

                Rectangle {
                    Layout.fillWidth: true
                    visible: activeTab === "uart"
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1

                    ColumnLayout {
                        id: uartContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 14

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.USB; iconSize: 22; iconColor: c.green }
                            Column {
                                Layout.fillWidth: true
                                FluText { text: english ? qsTr("Serial UART") : qsTr("串口 UART"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                                FluText { text: uartStatus !== "" ? uartStatus : (english ? qsTr("Select serial port and connect") : qsTr("选择串口并连接")); font: FluTextStyle.Caption; textColor: c.mutedText }
                            }
                            FluTextButton {
                                text: english ? qsTr("Refresh") : qsTr("刷新")
                                onClicked: {
                                    d("uart.refreshPorts", {})
                                    uartPortList = SerialManager.availablePorts
                                }
                            }
                        }

                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: c.cardBorder }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Repeater {
                                model: uartPortList
                                Rectangle {
                                    Layout.preferredWidth: 92
                                    Layout.preferredHeight: 36
                                    radius: 8
                                    color: uartPort === modelData ? Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.10) : (FluTheme.dark ? "#1A1A2E" : "#F9FAFB")
                                    border.color: uartPort === modelData ? FluTheme.primaryColor : c.cardBorder
                                    border.width: 1
                                    FluText { anchors.centerIn: parent; text: modelData; font: FluTextStyle.Body; textColor: uartPort === modelData ? FluTheme.primaryColor : FluTheme.fontPrimaryColor }
                                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: uartPort = modelData }
                                }
                            }
                            FluText { visible: uartPortList.length === 0; text: english ? qsTr("No serial ports, click refresh") : qsTr("暂无串口，点击刷新"); font: FluTextStyle.Caption; textColor: c.mutedText }
                            Item { Layout.fillWidth: true }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluText { text: english ? qsTr("Baud rate") : qsTr("波特率"); font: FluTextStyle.Caption; textColor: c.mutedText }
                            FluTextBox {
                                Layout.preferredWidth: 130
                                text: uartBaudrate.toString()
                                validator: IntValidator { bottom: 300; top: 4000000 }
                                onTextChanged: {
                                    var v = parseInt(text)
                                    if (!isNaN(v)) uartBaudrate = v
                                }
                            }
                            Item { Layout.fillWidth: true }
                            FluButton {
                                text: uartConnected ? (english ? qsTr("Connected") : qsTr("已连接")) : (english ? qsTr("Connect") : qsTr("连接"))
                                enabled: uartPort !== "" && !uartConnected
                                onClicked: d("uart.connect", { port: uartPort, baudrate: uartBaudrate })
                            }
                            FluButton {
                                text: english ? qsTr("Disconnect") : qsTr("断开")
                                visible: uartConnected || uartStatus !== ""
                                onClicked: d("uart.disconnect", {})
                            }
                        }
                    }

                    Layout.preferredHeight: uartContent.height + 48
                }

                Rectangle {
                    Layout.fillWidth: true
                    visible: activeTab === "hudp"
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1

                    ColumnLayout {
                        id: hudpContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 14

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.Globe; iconSize: 22; iconColor: c.blue }
                            Column {
                                Layout.fillWidth: true
                                FluText { text: qsTr("HID UDP"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                                FluText { text: hudpStatus !== "" ? hudpStatus : (english ? qsTr("HID UDP binary transport") : qsTr("HID UDP 二进制传输")); font: FluTextStyle.Caption; textColor: c.mutedText }
                            }
                        }

                        FluTextBox {
                            Layout.fillWidth: true
                            text: hudpTarget
                            placeholderText: english ? qsTr("Target address") : qsTr("目标地址")
                            onTextChanged: hudpTarget = text
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluTextBox {
                                Layout.preferredWidth: 120
                                text: hudpPort.toString()
                                validator: IntValidator { bottom: 1; top: 65535 }
                                onTextChanged: {
                                    var v = parseInt(text)
                                    if (!isNaN(v)) hudpPort = v
                                }
                            }
                            Item { Layout.fillWidth: true }
                            FluButton { text: hudpConnecting ? (english ? qsTr("Connecting...") : qsTr("连接中...")) : (english ? qsTr("Connect") : qsTr("连接")); enabled: hudpTarget !== "" && !hudpConnecting; onClicked: d("hudp.connect", { target: hudpTarget, port: hudpPort }) }
                            FluButton { text: english ? qsTr("Disconnect") : qsTr("断开"); visible: hudpConnecting || hudpStatus !== ""; onClicked: d("hudp.disconnect", {}) }
                        }
                    }

                    Layout.preferredHeight: hudpContent.height + 48
                }
            }
        }
    }
}

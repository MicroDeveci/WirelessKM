import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import FluentUI 1.0

Window {
    id: monitorWindow
    width: 1000
    height: 680
    minimumWidth: 560
    minimumHeight: 360
    title: english ? qsTr("ESP-IDF Serial Monitor") : qsTr("ESP-IDF 串口监视器")
    color: FluTheme.dark ? "#111122" : "#F7F8FA"
    flags: Qt.Window
    modality: Qt.NonModal

    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"
    property bool followOutput: true

    onClosing: function(close) {
        close.accepted = false
        visible = false
    }
    onVisibleChanged: {
        if (visible) {
            SerialManager.startMonitor()
            Qt.callLater(scrollToEnd)
        } else {
            SerialManager.stopMonitor()
        }
    }

    function scrollToEnd() {
        if (followOutput)
            outputScroll.ScrollBar.vertical.position = 1.0
    }

    Connections {
        target: SerialManager
        function onMonitorTextChanged() {
            Qt.callLater(monitorWindow.scrollToEnd)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            FluText {
                text: english ? qsTr("Raw serial output") : qsTr("原始串口输出")
                font: FluTextStyle.BodyStrong
                Layout.fillWidth: true
            }
            FluText {
                text: SerialManager.connected
                    ? (english ? qsTr("Connected: ") : qsTr("已连接：")) + SerialManager.currentPort
                    : (english ? qsTr("Serial not connected") : qsTr("串口未连接"))
                color: SerialManager.connected ? "#16A34A" : "#DC2626"
            }
            FluCheckBox {
                text: english ? qsTr("Auto scroll") : qsTr("自动滚动")
                checked: monitorWindow.followOutput
                onToggled: monitorWindow.followOutput = checked
            }
            FluButton {
                text: english ? qsTr("Clear") : qsTr("清空")
                onClicked: SerialManager.clearMonitor()
            }
            FluButton {
                text: english ? qsTr("Close") : qsTr("关闭")
                onClicked: monitorWindow.visible = false
            }
        }

        FluText {
            Layout.fillWidth: true
            text: english ? qsTr("Displays all raw byte streams received from existing COM connections; does not reopen the serial port or affect UART transport.") : qsTr("显示现有 COM 连接收到的全部原始字节流；不会重新打开串口或影响 UART 传输。")
            font: FluTextStyle.Caption
            color: FluTheme.dark ? "#A1A1AA" : "#6B7280"
            wrapMode: Text.WordWrap
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: FluTheme.dark ? "#080811" : "#FFFFFF"
            border.color: FluTheme.dark ? "#303044" : "#D1D5DB"
            border.width: 1
            radius: 6

            ScrollView {
                id: outputScroll
                anchors.fill: parent
                anchors.margins: 2
                clip: true

                TextArea {
                    width: outputScroll.availableWidth
                    readOnly: true
                    selectByMouse: true
                    wrapMode: TextEdit.NoWrap
                    textFormat: TextEdit.PlainText
                    text: SerialManager.monitorText
                    color: FluTheme.dark ? "#D4D4D8" : "#18181B"
                    selectionColor: FluTheme.primaryColor
                    selectedTextColor: "white"
                    font.family: "Consolas"
                    font.pixelSize: 13
                    background: Item {}
                }
            }
        }
    }
}

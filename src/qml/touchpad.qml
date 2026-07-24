import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    id: page
    title: english ? qsTr("Touchpad") : qsTr("触摸板")

    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"
    property real sensitivity: 1.0
    property real residualX: 0
    property real residualY: 0
    property string lastDeltaText: english ? qsTr("Waiting for input") : qsTr("等待输入")

    function d(cmd, params) {
        if (typeof InterfaceLayer !== "undefined")
            InterfaceLayer.dispatch(cmd, params)
    }

    function sendRelative(rawDx, rawDy) {
        var scaledX = rawDx * sensitivity + residualX
        var scaledY = rawDy * sensitivity + residualY
        var dx = scaledX < 0 ? Math.ceil(scaledX) : Math.floor(scaledX)
        var dy = scaledY < 0 ? Math.ceil(scaledY) : Math.floor(scaledY)
        residualX = scaledX - dx
        residualY = scaledY - dy
        if (dx !== 0 || dy !== 0) {
            lastDeltaText = english ? qsTr("Relative move  X %1  ·  Y %2").arg(dx).arg(dy) : qsTr("相对移动  X %1  ·  Y %2").arg(dx).arg(dy)
            d("mouse.move", { dx: dx, dy: dy })
        }
    }

    function ensureDirectTouchpadMode() {
        if (typeof InterfaceLayer !== "undefined" && InterfaceLayer.passthroughMouseListening)
            d("passthrough.mouseListenToggled", { enabled: false })
    }

    onVisibleChanged: {
        if (visible)
            ensureDirectTouchpadMode()
        else
            d("mouse.releaseAll", {})
    }
    Component.onCompleted: ensureDirectTouchpadMode()
    Component.onDestruction: d("mouse.releaseAll", {})

    Loader {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        asynchronous: true
        sourceComponent: Component {
            ColumnLayout {
                width: 720
                spacing: 14

                QtObject {
                    id: c
                    readonly property color cardBg: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color cardBorder: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color mutedText: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                    readonly property color green: "#10B981"
                    readonly property color red: "#EF4444"
                    readonly property color padBg: FluTheme.dark ? "#171725" : "#F8FAFC"
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
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        spacing: 12

                        Rectangle {
                            Layout.preferredWidth: 42
                            Layout.preferredHeight: 42
                            radius: 10
                            color: Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.12)
                            FluIcon { anchors.centerIn: parent; iconSource: FluentIcons.Touchpad; iconSize: 22; iconColor: FluTheme.primaryColor }
                        }
                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            FluText { text: english ? qsTr("Remote Touchpad") : qsTr("远程触摸板"); font: FluTextStyle.Title; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Drag to send relative coordinates, bottom buttons send press and release events") : qsTr("拖动发送相对坐标，底部按键发送按下与释放事件"); font: FluTextStyle.Caption; textColor: c.mutedText }
                        }
                        Rectangle {
                            Layout.preferredWidth: 82
                            Layout.preferredHeight: 28
                            radius: 14
                            color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected)
                                ? Qt.rgba(0.063, 0.725, 0.506, 0.10)
                                : Qt.rgba(0.937, 0.267, 0.267, 0.10)
                            border.color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected)
                                ? Qt.rgba(0.063, 0.725, 0.506, 0.30)
                                : Qt.rgba(0.937, 0.267, 0.267, 0.30)
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
                    id: touchSurface
                    Layout.fillWidth: true
                    Layout.preferredHeight: 310
                    radius: 18
                    scale: padArea.pressed ? 0.995 : 1.0
                    color: !padArea.enabled
                        ? (FluTheme.dark ? "#161620" : "#F3F4F6")
                        : padArea.pressed
                            ? Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.12)
                            : padArea.containsMouse
                                ? Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.06)
                                : c.padBg
                    border.color: padArea.pressed ? FluTheme.primaryColor : c.cardBorder
                    border.width: padArea.pressed ? 2 : 1

                    Behavior on scale { NumberAnimation { duration: 90; easing.type: Easing.OutCubic } }
                    Behavior on color { ColorAnimation { duration: 100 } }

                    Column {
                        anchors.centerIn: parent
                        spacing: 10
                        FluIcon {
                            anchors.horizontalCenter: parent.horizontalCenter
                            iconSource: padArea.pressed ? FluentIcons.TouchPointer : FluentIcons.Touchpad
                            iconSize: 42
                            iconColor: padArea.enabled ? FluTheme.primaryColor : c.mutedText
                        }
                        FluText {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: padArea.pressed ? (english ? qsTr("Moving") : qsTr("正在移动")) : (english ? qsTr("Hold and drag") : qsTr("按住并拖动"))
                            font: FluTextStyle.Subtitle
                            textColor: padArea.enabled ? FluTheme.fontPrimaryColor : c.mutedText
                        }
                        FluText {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: page.lastDeltaText
                            font: FluTextStyle.Caption
                            textColor: c.mutedText
                        }
                    }

                    MouseArea {
                        id: padArea
                        anchors.fill: parent
                        enabled: typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected
                        acceptedButtons: Qt.LeftButton
                        hoverEnabled: true
                        preventStealing: true
                        scrollGestureEnabled: false
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        property real lastX: 0
                        property real lastY: 0

                        onPressed: {
                            lastX = mouse.x
                            lastY = mouse.y
                            page.residualX = 0
                            page.residualY = 0
                            page.lastDeltaText = english ? qsTr("Touch started") : qsTr("触摸已开始")
                        }
                        onPositionChanged: {
                            if (!pressed)
                                return
                            var dx = mouse.x - lastX
                            var dy = mouse.y - lastY
                            lastX = mouse.x
                            lastY = mouse.y
                            page.sendRelative(dx, dy)
                        }
                        onReleased: {
                            page.residualX = 0
                            page.residualY = 0
                            page.lastDeltaText = english ? qsTr("Touch ended") : qsTr("触摸已结束")
                        }
                        onCanceled: {
                            page.residualX = 0
                            page.residualY = 0
                            page.lastDeltaText = english ? qsTr("Touch cancelled") : qsTr("触摸已取消")
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    FluFilledButton {
                        id: leftButton
                        Layout.fillWidth: true
                        Layout.preferredHeight: 64
                        enabled: typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected
                        text: pressed ? (english ? qsTr("Left · Pressed") : qsTr("左键 · 按下")) : (english ? qsTr("Left") : qsTr("左键"))
                        contentDescription: english ? qsTr("Remote mouse left button") : qsTr("远程鼠标左键")
                        onPressed: d("mouse.button", { button: 1, pressed: true })
                        onReleased: d("mouse.button", { button: 1, pressed: false })
                        onCanceled: d("mouse.button", { button: 1, pressed: false })
                    }

                    FluButton {
                        id: rightButton
                        Layout.fillWidth: true
                        Layout.preferredHeight: 64
                        enabled: typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected
                        text: pressed ? (english ? qsTr("Right · Pressed") : qsTr("右键 · 按下")) : (english ? qsTr("Right") : qsTr("右键"))
                        contentDescription: english ? qsTr("Remote mouse right button") : qsTr("远程鼠标右键")
                        onPressed: d("mouse.button", { button: 2, pressed: true })
                        onReleased: d("mouse.button", { button: 2, pressed: false })
                        onCanceled: d("mouse.button", { button: 2, pressed: false })
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    radius: 10
                    color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected)
                        ? Qt.rgba(0.063, 0.725, 0.506, 0.06)
                        : Qt.rgba(0.937, 0.267, 0.267, 0.06)
                    border.color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected)
                        ? Qt.rgba(0.063, 0.725, 0.506, 0.22)
                        : Qt.rgba(0.937, 0.267, 0.267, 0.22)
                    border.width: 1
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 10
                        FluIcon {
                            iconSource: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? FluentIcons.CompletedSolid : FluentIcons.StatusErrorFull
                            iconSize: 18
                            iconColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? c.green : c.red
                        }
                        FluText {
                            Layout.fillWidth: true
                            text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected)
                                ? (english ? qsTr("Mouse events will be sent via the active transport") : qsTr("鼠标事件将通过当前活动传输发送"))
                                : (english ? qsTr("Please connect first on the Devices page") : qsTr("请先在「链接设备」页面建立连接"))
                            font: FluTextStyle.Caption
                            textColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? c.green : c.red
                        }
                    }
                }
            }
        }
    }
}

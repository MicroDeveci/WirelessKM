import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    id: page
    title: english ? qsTr("Auto clicker") : qsTr("连点器")

    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"

    property var availableKeys: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.autoClickerAvailableKeys() : []
    property var keyLabels: availableKeys.map(function(k) { return k.label })

    function d(cmd, params) {
        if (typeof InterfaceLayer !== "undefined")
            InterfaceLayer.dispatch(cmd, params)
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
                    id: colors
                    readonly property color card: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color border: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color muted: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                    readonly property color green: "#10B981"
                    readonly property color red: "#EF4444"
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 76
                    radius: 12
                    color: colors.card
                    border.color: colors.border
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12
                        Rectangle {
                            Layout.preferredWidth: 42
                            Layout.preferredHeight: 42
                            radius: 10
                            color: Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.12)
                            FluIcon { anchors.centerIn: parent; iconSource: FluentIcons.KeyboardSettings; iconSize: 21; iconColor: FluTheme.primaryColor }
                        }
                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            FluText { text: english ? qsTr("Auto clicker") : qsTr("连点器"); font: FluTextStyle.Title; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Press all selected keys each cycle then release; 0 means infinite loop") : qsTr("每轮同时按下所有已选键后释放；0 次表示无限循环"); font: FluTextStyle.Caption; textColor: colors.muted }
                        }
                        Rectangle {
                            Layout.preferredWidth: 112
                            Layout.preferredHeight: 28
                            radius: 14
                            color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning) ? Qt.rgba(0.063, 0.725, 0.506, 0.10) : Qt.rgba(0.42, 0.45, 0.50, 0.10)
                            border.color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning) ? Qt.rgba(0.063, 0.725, 0.506, 0.30) : Qt.rgba(0.42, 0.45, 0.50, 0.25)
                            border.width: 1
                            FluText {
                                anchors.centerIn: parent
                                text: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.autoClickerStatus : (english ? qsTr("Not ready") : qsTr("未就绪"))
                                font: FluTextStyle.Caption
                                textColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning) ? colors.green : colors.muted
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: keyContent.height + 32
                    radius: 12
                    color: colors.card
                    border.color: colors.border
                    border.width: 1

                    ColumnLayout {
                        id: keyContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 18
                        spacing: 12

                        RowLayout {
                            Layout.fillWidth: true
                            FluText { text: english ? qsTr("Keys") : qsTr("按键"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor; Layout.fillWidth: true }
                            FluButton {
                                text: english ? qsTr("Clear") : qsTr("清空")
                                enabled: !(typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning)
                                    && (typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerKeys.length > 0)
                                onClicked: d("clicker.clearKeys", {})
                            }
                        }

                        Rectangle {
                            id: captureInput
                            Layout.fillWidth: true
                            Layout.preferredHeight: 48
                            radius: 8
                            color: captureFocus.activeFocus ? Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.12) : "transparent"
                            border.color: captureFocus.activeFocus ? FluTheme.primaryColor : colors.border
                            border.width: 1
                            FocusScope {
                                id: captureFocus
                                anchors.fill: parent
                                focus: true
                                Keys.onPressed: function(event) {
                                    event.accepted = true
                                    d("clicker.addCapturedKey", { qtKey: event.key, text: event.text })
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                                    onPressed: function(mouse) {
                                        captureFocus.forceActiveFocus()
                                        d("clicker.addMouseButton", { button: mouse.button })
                                        mouse.accepted = true
                                    }
                                }
                                FluText {
                                    anchors.centerIn: parent
                                    text: english ? "Click here, then press any key, F-key, or mouse button" : "点击此处后，直接按任意键、功能键或鼠标按钮录入"
                                    font: FluTextStyle.Caption
                                    textColor: captureFocus.activeFocus ? FluTheme.primaryColor : colors.muted
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            FluComboBox {
                                id: keyPicker
                                Layout.fillWidth: true
                                model: page.keyLabels
                                enabled: !(typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning)
                            }
                            FluFilledButton {
                                text: english ? qsTr("Add key") : qsTr("添加按键")
                                enabled: keyPicker.currentIndex >= 0 && !(typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning)
                                onClicked: d("clicker.addKey", { usage: page.availableKeys[keyPicker.currentIndex].usage })
                            }
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 8
                            visible: typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerKeys.length > 0
                            Repeater {
                                model: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.autoClickerKeys : []
                                Rectangle {
                                    width: chipText.implicitWidth + 48
                                    height: 32
                                    radius: 16
                                    color: Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.10)
                                    border.color: Qt.rgba(FluTheme.primaryColor.r, FluTheme.primaryColor.g, FluTheme.primaryColor.b, 0.26)
                                    border.width: 1
                                    Row {
                                        anchors.centerIn: parent
                                        spacing: 6
                                        FluText { id: chipText; text: modelData.label; font: FluTextStyle.Caption; textColor: FluTheme.primaryColor }
                                        FluIcon { iconSource: FluentIcons.ChromeClose; iconSize: 12; iconColor: colors.muted }
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        enabled: !(typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning)
                                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                        onClicked: d("clicker.removeKey", { usage: modelData.usage })
                                    }
                                }
                            }
                        }

                        FluText {
                            Layout.fillWidth: true
                            visible: typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerKeys.length === 0
                            text: english ? qsTr("Please select and add at least one key") : qsTr("请选择并添加至少一个按键")
                            font: FluTextStyle.Caption
                            textColor: colors.muted
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: timingContent.height + 32
                    radius: 12
                    color: colors.card
                    border.color: colors.border
                    border.width: 1

                    GridLayout {
                        id: timingContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 18
                        columns: 2
                        columnSpacing: 24
                        rowSpacing: 12

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            FluText { text: english ? qsTr("Interval (ms)") : qsTr("间隔时间（毫秒）"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                            FluSpinBox {
                                id: intervalBox
                                Layout.fillWidth: true
                                from: 10
                                to: 60000
                                stepSize: 10
                                value: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.autoClickerInterval : 100
                                editable: true
                                enabled: !(typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning)
                                onValueModified: d("clicker.setInterval", { intervalMs: value })
                            }
                            FluText { text: english ? qsTr("Minimum 10 ms") : qsTr("最小 10 ms"); font: FluTextStyle.Caption; textColor: colors.muted }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            FluText { text: english ? qsTr("Repeat count") : qsTr("循环次数"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                            FluSpinBox {
                                id: repeatBox
                                Layout.fillWidth: true
                                from: 0
                                to: 999999
                                value: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.autoClickerRepeatCount : 0
                                editable: true
                                enabled: !(typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning)
                                onValueModified: d("clicker.setRepeatCount", { repeatCount: value })
                            }
                            FluText { text: english ? qsTr("0 means infinite repeat") : qsTr("0 表示无限重复"); font: FluTextStyle.Caption; textColor: colors.muted }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    radius: 10
                    color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? Qt.rgba(0.063, 0.725, 0.506, 0.06) : Qt.rgba(0.937, 0.267, 0.267, 0.06)
                    border.color: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? Qt.rgba(0.063, 0.725, 0.506, 0.22) : Qt.rgba(0.937, 0.267, 0.267, 0.22)
                    border.width: 1
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        spacing: 10
                        FluIcon { iconSource: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? FluentIcons.CompletedSolid : FluentIcons.StatusErrorFull; iconSize: 18; iconColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? colors.green : colors.red }
                        FluText { Layout.fillWidth: true; text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? (english ? qsTr("Will send via the active transport") : qsTr("将通过当前已连接的传输通道发送")) : (english ? qsTr("Please connect first on the Devices page") : qsTr("请先在「连接设备」页面建立连接")); font: FluTextStyle.Caption; textColor: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected) ? colors.green : colors.red }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Item { Layout.fillWidth: true }
                    FluText {
                        text: typeof InterfaceLayer !== "undefined" ? qsTr("Executed %1 times").arg(InterfaceLayer.autoClickerCompletedCycles) : ""
                        font: FluTextStyle.Caption
                        textColor: colors.muted
                    }
                    FluButton {
                        text: english ? qsTr("Stop") : qsTr("停止")
                        visible: typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning
                        onClicked: d("clicker.stop", {})
                    }
                    FluFilledButton {
                        text: typeof InterfaceLayer !== "undefined" && InterfaceLayer.autoClickerRunning ? (english ? qsTr("Running") : qsTr("运行中")) : (english ? qsTr("Start") : qsTr("开始连点"))
                        enabled: typeof InterfaceLayer !== "undefined" && InterfaceLayer.connected
                            && InterfaceLayer.autoClickerKeys.length > 0 && !InterfaceLayer.autoClickerRunning
                        onClicked: d("clicker.start", {})
                    }
                }
            }
        }
    }
}

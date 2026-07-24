import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    title: english ? qsTr("Logs") : qsTr("日志")

    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"

    function d(cmd, params) {
        if (typeof InterfaceLayer !== "undefined") InterfaceLayer.dispatch(cmd, params)
    }

    readonly property var levelConfig: [
        { label: english ? qsTr("Info") : qsTr("Info"), icon: FluentIcons.InfoSolid, color: "#3B82F6" },
        { label: english ? qsTr("Warn") : qsTr("Warn"), icon: FluentIcons.Warning, color: "#F59E0B" },
        { label: english ? qsTr("Error") : qsTr("Error"), icon: FluentIcons.StatusErrorFull, color: "#EF4444" },
        { label: english ? qsTr("Debug") : qsTr("Debug"), icon: FluentIcons.Bug, color: "#8B5CF6" }
    ]

    Loader {
        id: logPageLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        asynchronous: true
        property int revision: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.logViewRevision : 0
        onRevisionChanged: {
            if (item) {
                item.opacity = 0.92
                refreshAnim.restart()
            }
        }
        NumberAnimation {
            id: refreshAnim
            target: logPageLoader.item
            property: "opacity"
            from: 0.92
            to: 1
            duration: 140
            easing.type: Easing.OutCubic
        }
        sourceComponent: Component {
            ColumnLayout {
                width: 760
                spacing: 16

                QtObject {
                    id: c
                    readonly property color cardBg: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color cardBorder: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color mutedText: FluTheme.dark ? "#9CA3AF" : "#6B7280"
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
                            FluIcon { anchors.centerIn: parent; iconSource: FluentIcons.Bug; iconSize: 22; iconColor: FluTheme.primaryColor }
                        }

                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            FluText { text: english ? qsTr("Logs") : qsTr("日志"); font: FluTextStyle.Title; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Selectable runtime text log") : qsTr("可选择的运行时文本日志"); font: FluTextStyle.Caption; textColor: c.mutedText }
                        }

                        FluIconButton { iconSource: FluentIcons.Refresh; iconSize: 16; onClicked: d("log.refresh", {}); width: 32; height: 32; radius: 8 }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: levelConfig
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            radius: 8
                            property int currentLevel: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.currentLogLevel : 0
                            property int countVal: index === 0
                                ? (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.infoCount : 0)
                                : index === 1
                                    ? (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.warningCount : 0)
                                    : index === 2
                                        ? (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.errorLogCount : 0)
                                        : (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.debugCount : 0)

                            color: currentLevel === index ? Qt.rgba(parseInt(modelData.color.slice(1,3),16)/255, parseInt(modelData.color.slice(3,5),16)/255, parseInt(modelData.color.slice(5,7),16)/255, 0.12) : (FluTheme.dark ? "#1A1A2E" : "#F9FAFB")
                            border.color: currentLevel === index ? modelData.color : c.cardBorder
                            border.width: currentLevel === index ? 1.5 : 1

                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: d("log.switchLevel", { level: index }) }

                            RowLayout {
                                anchors.centerIn: parent
                                spacing: 8
                                FluIcon { iconSource: modelData.icon; iconSize: 16; iconColor: modelData.color }
                                FluText { text: modelData.label; font: FluTextStyle.BodyStrong; textColor: currentLevel === index ? modelData.color : c.mutedText }
                                Rectangle {
                                    visible: countVal > 0
                                    width: Math.max(24, countText.width + 12)
                                    height: 18
                                    radius: 9
                                    color: modelData.color
                                    FluText { id: countText; anchors.centerIn: parent; text: countVal; font: FluTextStyle.Caption; color: "#FFFFFF" }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 520
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            FluText { text: english ? qsTr("Log text") : qsTr("日志文本"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor; Layout.fillWidth: true }
                            FluTextButton { text: english ? qsTr("Clear") : qsTr("清空"); onClicked: d("log.clear", { level: (typeof InterfaceLayer !== "undefined" ? InterfaceLayer.currentLogLevel : 0) }) }
                            FluTextButton { text: english ? qsTr("Export JSON") : qsTr("导出 JSON"); onClicked: d("log.exportLogs", {}) }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 8
                            color: FluTheme.dark ? "#111122" : "#F9FAFB"
                            border.color: c.cardBorder
                            border.width: 1

                            ScrollView {
                                anchors.fill: parent
                                anchors.margins: 2
                                clip: true

                                TextArea {
                                    width: parent.width
                                    readOnly: true
                                    selectByMouse: true
                                    wrapMode: TextArea.Wrap
                                    textFormat: TextEdit.PlainText
                                    text: (typeof InterfaceLayer !== "undefined" && InterfaceLayer.logText !== "")
                                        ? InterfaceLayer.logText
                                        : qsTr("No logs")
                                    color: FluTheme.fontPrimaryColor
                                    selectedTextColor: "#FFFFFF"
                                    selectionColor: FluTheme.primaryColor
                                    font.family: "Consolas"
                                    font.pixelSize: 13
                                    background: Item {}
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

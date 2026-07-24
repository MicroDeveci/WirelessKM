import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"
    title: english ? qsTr("Thanks") : qsTr("鸣谢")

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
                            FluIcon { iconSource: FluentIcons.FavoriteStar; iconSize: 30; iconColor: "#EF4444" }
                            FluText { text: english ? qsTr("Acknowledgements") : qsTr("鸣谢"); font: FluTextStyle.Subtitle }
                        }
                        FluText {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            textColor: colors.muted
                            text: english
                                ? qsTr("This application is built with Qt Quick and the FluentUI control library.")
                                : qsTr("本应用基于 Qt Quick 与 FluentUI 控件库构建。")
                        }
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: colors.border }
                        FluText { text: english ? qsTr("Built with") : qsTr("技术栈"); font: FluTextStyle.Body; Layout.topMargin: 4 }
                        FluText { text: qsTr("Qt Quick — cross-platform C++ and QML application framework"); textColor: colors.muted; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                        FluButton { text: qsTr("Visit qt.io"); onClicked: Qt.openUrlExternally("https://www.qt.io/") }
                        FluText { text: qsTr("FluentUI — Fluent Design controls for Qt Quick"); textColor: colors.muted; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                        FluButton { text: qsTr("Visit zhuzichu520/FluentUI"); onClicked: Qt.openUrlExternally("https://github.com/zhuzichu520/FluentUI") }
                        RowLayout {
                            Layout.topMargin: 6
                            spacing: 12
                            FluButton { text: qsTr("GitHub"); onClicked: d("app.openGitHub", {}) }
                        }
                    }
                }
            }
        }
    }
}

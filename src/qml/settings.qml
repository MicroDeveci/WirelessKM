import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import FluentUI 1.0

FluScrollablePage {
    title: english ? "Settings" : qsTr("设置")

    function d(cmd, params) {
        if (typeof InterfaceLayer !== "undefined") InterfaceLayer.dispatch(cmd, params)
    }

    property var serialMonitorWindow: null
    property bool english: typeof InterfaceLayer !== "undefined" && InterfaceLayer.uiLanguage === "en-US"

    function openSerialMonitorWindow() {
        if (!serialMonitorWindow) {
            const component = Qt.createComponent("qrc:/SerialMonitorWindow.qml")
            if (component.status !== Component.Ready) {
                console.warn("Unable to create serial monitor:", component.errorString())
                return
            }
            // A null QObject parent makes this a real independent native Window,
            // not a child popup of the Settings page.
            serialMonitorWindow = component.createObject(null)
        }

        SerialManager.startMonitor()
        serialMonitorWindow.show()
        serialMonitorWindow.raise()
    }

    Loader {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        asynchronous: true
        sourceComponent: Component {
            ColumnLayout {
                width: 720
                spacing: 14

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 68
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12
                        FluIcon { iconSource: FluentIcons.Settings; iconSize: 20; iconColor: FluTheme.primaryColor }
                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            FluText { text: english ? qsTr("Language") : "语言"; font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Choose the desktop display language.") : "选择桌面显示语言。"; font: FluTextStyle.Caption; textColor: c.mutedText }
                        }
                        FluComboBox {
                            id: languageSelector
                            Layout.preferredWidth: 150
                            model: ["简体中文", "English"]
                            currentIndex: english ? 1 : 0
                            onActivated: d("settings.uiLanguageChanged", { language: currentIndex === 1 ? "en-US" : "zh-CN" })
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 68
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12
                        FluIcon { iconSource: FluentIcons.Brightness; iconSize: 20; iconColor: FluTheme.primaryColor }
                        Column {
                            Layout.fillWidth: true
                            spacing: 2
                            FluText { text: english ? qsTr("Theme") : "主题"; font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Switch between light, dark, and system theme.") : "切换浅色、深色和跟随系统主题。"; font: FluTextStyle.Caption; textColor: c.mutedText }
                        }
                        FluComboBox {
                            id: themeSelector
                            Layout.preferredWidth: 150
                            model: [english ? "Light" : "白天", english ? "Dark" : "暗夜", english ? "System" : "自动"]
                            currentIndex: FluTheme.darkMode === 2 ? 1 : FluTheme.darkMode === 1 ? 0 : 2
                            onActivated: {
                                var modes = [1, 2, 0]
                                FluTheme.darkMode = modes[currentIndex]
                                d("settings.themeChanged", { mode: modes[currentIndex] })
                            }
                        }
                    }
                }

                QtObject {
                    id: c
                    readonly property color cardBg: FluTheme.dark ? "#1E1E2E" : "#FFFFFF"
                    readonly property color cardBorder: FluTheme.dark ? "#2D2D3F" : "#E5E7EB"
                    readonly property color mutedText: FluTheme.dark ? "#9CA3AF" : "#6B7280"
                }

                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    ColumnLayout {
                        id: serialMonitorContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 12

                        FluText { text: english ? qsTr("Serial log") : qsTr("串口日志"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.Bug; iconSize: 18; iconColor: FluTheme.fontPrimaryColor }
                            Column {
                                Layout.fillWidth: true
                                spacing: 2
                                FluText { text: english ? qsTr("ESP-IDF serial log window") : qsTr("ESP-IDF 串口日志窗口"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                                FluText { text: english ? qsTr("View boot logs, errors, and panic output from connected ESP.") : qsTr("查看已连接 ESP 的启动日志、错误和 panic 输出。"); font: FluTextStyle.Caption; textColor: c.mutedText }
                            }
                            FluButton {
                                text: english ? qsTr("Open window") : qsTr("启动窗口")
                                onClicked: openSerialMonitorWindow()
                            }
                        }
                    }
                    Layout.preferredHeight: serialMonitorContent.height + 48
                }

                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    ColumnLayout {
                        id: connectionContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 12

                        FluText { text: english ? qsTr("Connection") : qsTr("连接"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 54
                            radius: 8
                            color: "transparent"
                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                FluIcon { iconSource: FluentIcons.Connect; iconSize: 18; iconColor: FluTheme.fontPrimaryColor }
                                Column {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    FluText { text: english ? qsTr("Remember last connection") : qsTr("记住上次连接状态"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                                    FluText { text: english ? qsTr("Auto-restore last UART/HUDP on startup; BLE scans first then matches address") : qsTr("启动时自动恢复上次未断开的 UART/HUDP，BLE 会先扫描再匹配地址"); font: FluTextStyle.Caption; textColor: c.mutedText }
                                }
                                FluToggleSwitch {
                                    checked: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.rememberLastConnection : false
                                    onCheckedChanged: d("settings.rememberLastConnectionToggled", { enabled: checked })
                                }
                            }
                        }
                    }
                    Layout.preferredHeight: connectionContent.height + 48
                }

                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    ColumnLayout {
                        id: systemContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 12

                        FluText { text: english ? qsTr("System") : qsTr("系统"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 54
                            radius: 8
                            color: "transparent"
                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                FluIcon { iconSource: FluentIcons.PowerButton; iconSize: 18; iconColor: FluTheme.fontPrimaryColor }
                                Column {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    FluText { text: english ? qsTr("Auto start") : qsTr("开机自启"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                                    FluText { text: english ? qsTr("Write to the system autostart location for current user") : qsTr("写入当前用户的系统自启动位置"); font: FluTextStyle.Caption; textColor: c.mutedText }
                                }
                                FluToggleSwitch {
                                    checked: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.autoStart : false
                                    onCheckedChanged: d("settings.autoStartToggled", { enabled: checked })
                                }
                            }
                        }
                    }
                    Layout.preferredHeight: systemContent.height + 48
                }

                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    ColumnLayout {
                        id: captureContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 12

                        FluText { text: english ? qsTr("Text capture") : qsTr("捕获文字"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.SelectAll; iconSize: 18; iconColor: FluTheme.fontPrimaryColor }
                            Column {
                                Layout.fillWidth: true
                                spacing: 2
                                FluText { text: english ? qsTr("Enable text capture") : qsTr("启用捕获文字"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                                FluText { text: english ? qsTr("Keep the original text capture toggle; configuration managed here") : qsTr("保留原捕获文字开关，配置从这里统一管理"); font: FluTextStyle.Caption; textColor: c.mutedText }
                            }
                            FluToggleSwitch {
                                checked: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.captureEnabled : true
                                onCheckedChanged: d("key.captureToggled", { enabled: checked })
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.KeyboardSettings; iconSize: 18; iconColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Capture hotkey") : qsTr("捕获热键"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                            Item { Layout.fillWidth: true }
                            FluTextBox {
                                Layout.preferredWidth: 180
                                selectByMouse: true
                                text: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.captureHotkey : "Ctrl+`"
                                onEditingFinished: d("key.captureHotkeyChanged", { newHotkey: text })
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12
                            FluIcon { iconSource: FluentIcons.Paste; iconSize: 18; iconColor: FluTheme.fontPrimaryColor }
                            FluText { text: english ? qsTr("Paste hotkey") : qsTr("粘贴热键"); font: FluTextStyle.Body; textColor: FluTheme.fontPrimaryColor }
                            Item { Layout.fillWidth: true }
                            FluTextBox {
                                Layout.preferredWidth: 180
                                selectByMouse: true
                                text: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.capturePasteHotkey : "Ctrl+V"
                                onEditingFinished: d("key.capturePasteHotkeyChanged", { newHotkey: text })
                            }
                        }
                    }
                    Layout.preferredHeight: captureContent.height + 48
                }

                Rectangle {
                    Layout.fillWidth: true
                    radius: 12
                    color: c.cardBg
                    border.color: c.cardBorder
                    border.width: 1
                    ColumnLayout {
                        id: configContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 24
                        spacing: 12

                        FluText { text: english ? qsTr("Config file") : qsTr("配置文件"); font: FluTextStyle.BodyStrong; textColor: FluTheme.fontPrimaryColor }

                        FluTextBox {
                            Layout.fillWidth: true
                            readOnly: true
                            selectByMouse: true
                            text: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.configFilePath : ""
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            FluTextBox {
                                id: importPath
                                Layout.fillWidth: true
                                selectByMouse: true
                                placeholderText: english ? qsTr("Enter JSON config file path to import") : qsTr("输入要导入的 JSON 配置文件路径")
                            }
                            FluButton {
                                text: english ? qsTr("Import config") : qsTr("导入配置")
                                enabled: importPath.text.length > 0
                                onClicked: d("settings.importConfig", { path: importPath.text })
                            }
                        }

                        FluText {
                            Layout.fillWidth: true
                            text: typeof InterfaceLayer !== "undefined" ? InterfaceLayer.settingsStatus : ""
                            font: FluTextStyle.Caption
                            textColor: c.mutedText
                            wrapMode: Text.WordWrap
                        }
                    }
                    Layout.preferredHeight: configContent.height + 48
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    Item { Layout.fillWidth: true }
                    FluFilledButton {
                        text: english ? qsTr("Reset to defaults") : qsTr("恢复默认配置")
                        onClicked: d("settings.resetToDefaults", {})
                    }
                }
            }
        }
    }
}

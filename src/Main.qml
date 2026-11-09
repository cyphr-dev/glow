import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 980
    height: 700
    minimumWidth: 840
    minimumHeight: 600
    visible: true
    title: qsTr("Glow — RGB Controller")
    color: "#0e0e10"

    Material.theme: Material.Dark
    Material.accent: theme.accent
    Material.roundedScale: theme.radius

    property bool helpVisible: false

    // Keyboard shortcuts
    Shortcut { sequence: "?"; context: Qt.ApplicationShortcut; onActivated: win.helpVisible = !win.helpVisible }
    Shortcut { sequence: "Escape"; context: Qt.ApplicationShortcut; onActivated: if (win.helpVisible) win.helpVisible = false }
    Shortcut { sequence: "Q"; context: Qt.ApplicationShortcut; onActivated: Qt.quit() }
    Shortcut { sequence: "Space"; context: Qt.ApplicationShortcut; onActivated: backend.primaryAction() }
    Shortcut { sequence: "S"; context: Qt.ApplicationShortcut; onActivated: backend.applyThemeAccent(theme.accent) }
    Shortcut { sequence: "R"; context: Qt.ApplicationShortcut; onActivated: backend.rescan() }
    Shortcut { sequence: "Tab"; context: Qt.ApplicationShortcut; onActivated: backend.nextDevice() }
    Shortcut { sequence: "J"; context: Qt.ApplicationShortcut; onActivated: backend.nextDevice() }
    Shortcut { sequence: "K"; context: Qt.ApplicationShortcut; onActivated: backend.prevDevice() }
    Shortcut { sequence: "1"; context: Qt.ApplicationShortcut; onActivated: backend.applyPreset(0) }
    Shortcut { sequence: "2"; context: Qt.ApplicationShortcut; onActivated: backend.applyPreset(1) }
    Shortcut { sequence: "3"; context: Qt.ApplicationShortcut; onActivated: backend.applyPreset(2) }
    Shortcut { sequence: "4"; context: Qt.ApplicationShortcut; onActivated: backend.applyPreset(3) }
    Shortcut { sequence: "5"; context: Qt.ApplicationShortcut; onActivated: backend.applyPreset(4) }

    Item {
        anchors.fill: parent
        anchors.margins: 24
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 20

            // Header bar
            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                Rectangle {
                    width: 44
                    height: 44
                    radius: Math.min(10, theme.radius)
                    color: backend.lightsOn ? backend.color : "#222228"
                    border.color: Qt.rgba(1, 1, 1, 0.15)
                    border.width: 1

                    Behavior on color {
                        ColorAnimation { duration: 250 }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "✦"
                        font.pixelSize: 22
                        font.bold: true
                        color: backend.lightsOn ? (theme.accentForeground) : "#666"
                    }
                }

                ColumnLayout {
                    spacing: 2

                    RowLayout {
                        spacing: 8
                        Text {
                            text: qsTr("Glow")
                            font.pixelSize: 22
                            font.bold: true
                            color: "white"
                        }

                        Rectangle {
                            height: 20
                            width: devCountText.implicitWidth + 14
                            radius: Math.min(4, theme.radius)
                            color: Qt.rgba(1, 1, 1, 0.08)

                            Text {
                                id: devCountText
                                anchors.centerIn: parent
                                text: qsTr("%n device(s)", "", backend.deviceCount)
                                font.pixelSize: 11
                                color: "#aaa"
                            }
                        }
                    }

                    Text {
                        text: qsTr("Omarchy RGB lighting & hardware controller")
                        font.pixelSize: 12
                        color: "#888892"
                    }
                }

                Item { Layout.fillWidth: true }

                // Theme sync button
                Button {
                    text: backend.syncWithTheme ? qsTr("Theme Synced (S)") : qsTr("Sync Theme (S)")
                    highlighted: backend.syncWithTheme
                    Material.roundedScale: theme.radius
                    onClicked: backend.applyThemeAccent(theme.accent)
                }

                // Power toggle button
                Button {
                    text: backend.lightsOn ? qsTr("Power Off (Space)") : qsTr("Power On (Space)")
                    Material.roundedScale: theme.radius
                    onClicked: backend.togglePower()
                }

                // Help button
                ToolButton {
                    text: "?"
                    font.bold: true
                    onClicked: win.helpVisible = !win.helpVisible
                }
            }

            // Driver status banner (if driver needs attention)
            Rectangle {
                Layout.fillWidth: true
                height: 40
                radius: Math.min(6, theme.radius)
                color: backend.openRgbInstalled ? "#14251a" : "#2a1c0d"
                border.color: backend.openRgbInstalled ? "#2ecc71" : "#e67e22"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 12

                    Text {
                        text: backend.openRgbInstalled ? "✓" : "⚠"
                        font.pixelSize: 14
                        font.bold: true
                        color: backend.openRgbInstalled ? "#2ecc71" : "#e67e22"
                    }

                    Text {
                        Layout.fillWidth: true
                        text: backend.driverStatus
                        font.pixelSize: 12
                        color: backend.openRgbInstalled ? "#a8e6cf" : "#ffd3b6"
                    }

                    Button {
                        text: qsTr("Rescan (R)")
                        flat: true
                        Material.roundedScale: theme.radius
                        onClicked: backend.rescan()
                    }
                }
            }

            // Central Area
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 20

                // Left column: Device Selector
                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 320
                    radius: theme.radius
                    color: "#16161a"
                    border.color: Qt.rgba(1, 1, 1, 0.08)
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 12

                        Text {
                            text: qsTr("DEVICES (Tab / J / K)")
                            font.pixelSize: 11
                            font.bold: true
                            font.letterSpacing: 1.2
                            color: "#888892"
                        }

                        ListView {
                            id: devListView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: backend.devices
                            spacing: 8

                            delegate: Rectangle {
                                width: devListView.width
                                height: 68
                                radius: Math.min(8, theme.radius)
                                color: (index === backend.selectedDeviceIndex)
                                    ? Qt.rgba(255, 255, 255, 0.08)
                                    : (devMouse.containsMouse ? Qt.rgba(255, 255, 255, 0.03) : "transparent")
                                border.color: (index === backend.selectedDeviceIndex)
                                    ? theme.accent
                                    : Qt.rgba(1, 1, 1, 0.05)
                                border.width: (index === backend.selectedDeviceIndex) ? 2 : 1

                                MouseArea {
                                    id: devMouse
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: backend.selectDevice(index)
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 12

                                    Rectangle {
                                        width: 14
                                        height: 14
                                        radius: 7
                                        color: backend.lightsOn ? backend.color : "#444"
                                        border.color: Qt.rgba(1, 1, 1, 0.2)
                                        border.width: 1
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Text {
                                            text: modelData.name
                                            font.pixelSize: 13
                                            font.bold: true
                                            color: "white"
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }

                                        Text {
                                            text: modelData.summary
                                            font.pixelSize: 10
                                            color: "#888892"
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Right column: Lighting controls & preview
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: theme.radius
                    color: "#16161a"
                    border.color: Qt.rgba(1, 1, 1, 0.08)
                    border.width: 1

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 20
                        contentWidth: availableWidth

                        ColumnLayout {
                            width: parent.width
                            spacing: 20

                            // Visualizer glow card
                            Rectangle {
                                Layout.fillWidth: true
                                height: 130
                                radius: Math.min(10, theme.radius)
                                color: "#0d0d10"
                                border.color: Qt.rgba(1, 1, 1, 0.06)
                                border.width: 1

                                Item {
                                    anchors.centerIn: parent
                                    width: 180
                                    height: 80

                                    // Outer soft glow
                                    Rectangle {
                                        id: glowHalo
                                        anchors.centerIn: parent
                                        width: parent.width + 30
                                        height: parent.height + 30
                                        radius: theme.radius + 15
                                        color: backend.lightsOn ? backend.color : "transparent"
                                        opacity: backend.lightsOn ? 0.35 : 0.0

                                        SequentialAnimation on opacity {
                                            running: backend.lightsOn && backend.mode === "breathing"
                                            loops: Animation.Infinite
                                            NumberAnimation { to: 0.1; duration: 1500; easing.type: Easing.InOutQuad }
                                            NumberAnimation { to: 0.6; duration: 1500; easing.type: Easing.InOutQuad }
                                        }
                                    }

                                    // Inner chassis preview
                                    Rectangle {
                                        anchors.fill: parent
                                        radius: theme.radius
                                        color: "#18181f"
                                        border.color: backend.lightsOn ? backend.color : "#33333d"
                                        border.width: 2

                                        ColumnLayout {
                                            anchors.centerIn: parent
                                            spacing: 4

                                            Text {
                                                Layout.alignment: Qt.AlignHCenter
                                                text: backend.lightsOn ? qsTr("ILLUMINATED") : qsTr("OFF")
                                                font.pixelSize: 10
                                                font.bold: true
                                                font.letterSpacing: 1.5
                                                color: backend.lightsOn ? backend.color : "#666"
                                            }

                                            Text {
                                                Layout.alignment: Qt.AlignHCenter
                                                text: backend.lightsOn ? backend.color : "—"
                                                font.pixelSize: 14
                                                font.bold: true
                                                font.family: "monospace"
                                                color: "white"
                                            }
                                        }
                                    }
                                }
                            }

                            // Mode Selector
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Text {
                                    text: qsTr("LIGHTING MODE")
                                    font.pixelSize: 11
                                    font.bold: true
                                    font.letterSpacing: 1.2
                                    color: "#888892"
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Repeater {
                                        model: [
                                            { name: qsTr("Static"), id: "static" },
                                            { name: qsTr("Breathing"), id: "breathing" },
                                            { name: qsTr("Spectrum"), id: "spectrum" },
                                            { name: qsTr("Off"), id: "off" }
                                        ]

                                        delegate: Button {
                                            Layout.fillWidth: true
                                            text: modelData.name
                                            highlighted: (backend.mode === modelData.id) && (modelData.id !== "off" ? backend.lightsOn : !backend.lightsOn)
                                            Material.roundedScale: theme.radius
                                            onClicked: backend.setMode(modelData.id)
                                        }
                                    }
                                }
                            }

                            // Presets
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Text {
                                    text: qsTr("COLOR PRESETS (1 - 5)")
                                    font.pixelSize: 11
                                    font.bold: true
                                    font.letterSpacing: 1.2
                                    color: "#888892"
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    Repeater {
                                        model: [
                                            { hex: "#FFD60A", name: "Gold (1)" },
                                            { hex: "#FF9500", name: "Amber (2)" },
                                            { hex: "#00C7BE", name: "Cyan (3)" },
                                            { hex: "#AF52DE", name: "Purple (4)" },
                                            { hex: "#FF2D55", name: "Rose (5)" }
                                        ]

                                        delegate: Rectangle {
                                            Layout.fillWidth: true
                                            height: 42
                                            radius: Math.min(8, theme.radius)
                                            color: modelData.hex
                                            border.color: (backend.color.toUpperCase() === modelData.hex.toUpperCase()) ? "white" : Qt.rgba(0, 0, 0, 0.4)
                                            border.width: (backend.color.toUpperCase() === modelData.hex.toUpperCase()) ? 2 : 1

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: backend.setColor(modelData.hex)
                                            }

                                            Text {
                                                anchors.centerIn: parent
                                                text: modelData.name
                                                font.pixelSize: 11
                                                font.bold: true
                                                color: (index === 0 || index === 1) ? "black" : "white"
                                            }
                                        }
                                    }
                                }
                            }

                            // Hex input & Omarchy Accent
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 16

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    TextField {
                                        id: hexField
                                        Layout.fillWidth: true
                                        placeholderText: qsTr("Hex Color Code")
                                        text: backend.color
                                        Material.roundedScale: theme.radius
                                        onEditingFinished: {
                                            if (text.startsWith("#") && (text.length === 7)) {
                                                backend.setColor(text);
                                            }
                                        }
                                    }
                                }

                                Button {
                                    text: qsTr("Omarchy Accent (%1)").arg(theme.accent)
                                    Material.roundedScale: theme.radius
                                    onClicked: backend.applyThemeAccent(theme.accent)
                                }
                            }

                            // Brightness slider
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: qsTr("BRIGHTNESS")
                                        font.pixelSize: 11
                                        font.bold: true
                                        font.letterSpacing: 1.2
                                        color: "#888892"
                                    }
                                    Item { Layout.fillWidth: true }
                                    Text {
                                        text: backend.brightness + "%"
                                        font.pixelSize: 12
                                        font.bold: true
                                        color: "white"
                                    }
                                }

                                Slider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 100
                                    stepSize: 1
                                    value: backend.brightness
                                    Material.roundedScale: theme.radius
                                    onMoved: backend.setBrightness(value)
                                }
                            }
                        }
                    }
                }
            }
        }

        // Shortcut cheat-sheet modal
        Rectangle {
            anchors.fill: parent
            visible: win.helpVisible
            color: Qt.rgba(0, 0, 0, 0.78)
            z: 99

            MouseArea {
                anchors.fill: parent
                onClicked: win.helpVisible = false
            }

            Rectangle {
                anchors.centerIn: parent
                width: 480
                height: 380
                radius: theme.radius
                color: "#1c1c22"
                border.color: theme.accent
                border.width: 1

                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: qsTr("Keyboard Shortcuts")
                            font.pixelSize: 18
                            font.bold: true
                            color: "white"
                        }
                        Item { Layout.fillWidth: true }
                        ToolButton {
                            text: "✕"
                            onClicked: win.helpVisible = false
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Qt.rgba(1, 1, 1, 0.1)
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        rowSpacing: 10
                        columnSpacing: 20

                        Text { text: "Space"; font.bold: true; font.family: "monospace"; color: theme.accent }
                        Text { text: qsTr("Toggle lights On / Off"); color: "#eee" }

                        Text { text: "S"; font.bold: true; font.family: "monospace"; color: theme.accent }
                        Text { text: qsTr("Sync with Omarchy desktop accent"); color: "#eee" }

                        Text { text: "1 - 5"; font.bold: true; font.family: "monospace"; color: theme.accent }
                        Text { text: qsTr("Apply color presets"); color: "#eee" }

                        Text { text: "Tab / J / K"; font.bold: true; font.family: "monospace"; color: theme.accent }
                        Text { text: qsTr("Navigate detected devices"); color: "#eee" }

                        Text { text: "R"; font.bold: true; font.family: "monospace"; color: theme.accent }
                        Text { text: qsTr("Rescan hardware and drivers"); color: "#eee" }

                        Text { text: "?"; font.bold: true; font.family: "monospace"; color: theme.accent }
                        Text { text: qsTr("Toggle this cheat-sheet"); color: "#eee" }

                        Text { text: "Q"; font.bold: true; font.family: "monospace"; color: theme.accent }
                        Text { text: qsTr("Quit Glow"); color: "#eee" }
                    }

                    Item { Layout.fillHeight: true }

                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Got it")
                        Material.roundedScale: theme.radius
                        onClicked: win.helpVisible = false
                    }
                }
            }
        }
    }
}

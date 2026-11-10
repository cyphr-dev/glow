import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 1020
    height: 760
    minimumWidth: 880
    minimumHeight: 640
    visible: true
    title: qsTr("Glow — RGB Controller & Thermaltake Monitor")
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
    Shortcut { sequence: "T"; context: Qt.ApplicationShortcut; onActivated: backend.toggleThermalReactive() }
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
            spacing: 18

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

                        // Thermal quick-badge
                        Rectangle {
                            height: 20
                            width: tempHeadText.implicitWidth + 14
                            radius: Math.min(4, theme.radius)
                            color: Qt.rgba(255, 255, 255, 0.06)
                            border.color: backend.fanThermalStatusColor
                            border.width: 1

                            Text {
                                id: tempHeadText
                                anchors.centerIn: parent
                                text: qsTr("❋ %1 (%2)").arg(backend.fanTemperatureString, backend.fanThermalStatus)
                                font.pixelSize: 10
                                font.bold: true
                                color: backend.fanThermalStatusColor
                            }
                        }
                    }

                    Text {
                        text: qsTr("Omarchy RGB lighting & Thermaltake fan hardware controller")
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
                height: 38
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
                spacing: 18

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
                                    ? (modelData.type === "fans" ? backend.fanThermalStatusColor : theme.accent)
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
                                        color: modelData.type === "fans"
                                            ? backend.fanThermalStatusColor
                                            : (backend.lightsOn ? backend.color : "#444")
                                        border.color: Qt.rgba(1, 1, 1, 0.2)
                                        border.width: 1
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 6

                                            Text {
                                                text: modelData.name
                                                font.pixelSize: 13
                                                font.bold: true
                                                color: "white"
                                                elide: Text.ElideRight
                                                Layout.fillWidth: true
                                            }

                                            // Temperature pill if available
                                            Rectangle {
                                                visible: !!modelData.temp
                                                height: 18
                                                width: devTempText.implicitWidth + 10
                                                radius: 9
                                                color: Qt.rgba(0, 0, 0, 0.45)
                                                border.color: backend.fanThermalStatusColor
                                                border.width: 1

                                                Text {
                                                    id: devTempText
                                                    anchors.centerIn: parent
                                                    text: modelData.temp || ""
                                                    font.pixelSize: 10
                                                    font.bold: true
                                                    color: backend.fanThermalStatusColor
                                                }
                                            }
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

                // Right column: Lighting controls & preview & Thermaltake Monitor
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: theme.radius
                    color: "#16161a"
                    border.color: Qt.rgba(1, 1, 1, 0.08)
                    border.width: 1

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 18
                        contentWidth: availableWidth

                        ColumnLayout {
                            width: parent.width
                            spacing: 18

                            // Visualizer glow card
                            Rectangle {
                                Layout.fillWidth: true
                                height: 120
                                radius: Math.min(10, theme.radius)
                                color: "#0d0d10"
                                border.color: Qt.rgba(1, 1, 1, 0.06)
                                border.width: 1

                                Item {
                                    anchors.centerIn: parent
                                    width: 200
                                    height: 74

                                    // Outer soft glow
                                    Rectangle {
                                        id: glowHalo
                                        anchors.centerIn: parent
                                        width: parent.width + 28
                                        height: parent.height + 28
                                        radius: theme.radius + 14
                                        color: backend.lightsOn ? backend.color : "transparent"
                                        opacity: backend.lightsOn ? 0.35 : 0.0

                                        SequentialAnimation on opacity {
                                            running: backend.lightsOn && (backend.mode === "breathing" || backend.mode === "thermal")
                                            loops: Animation.Infinite
                                            NumberAnimation { to: 0.15; duration: 1500; easing.type: Easing.InOutQuad }
                                            NumberAnimation { to: 0.65; duration: 1500; easing.type: Easing.InOutQuad }
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
                                            spacing: 3

                                            Text {
                                                Layout.alignment: Qt.AlignHCenter
                                                text: !backend.lightsOn
                                                    ? qsTr("OFF")
                                                    : (backend.mode === "thermal" ? qsTr("THERMAL REACTIVE") : qsTr("ILLUMINATED"))
                                                font.pixelSize: 10
                                                font.bold: true
                                                font.letterSpacing: 1.5
                                                color: backend.lightsOn ? backend.color : "#666"
                                            }

                                            Text {
                                                Layout.alignment: Qt.AlignHCenter
                                                text: !backend.lightsOn
                                                    ? "—"
                                                    : (backend.mode === "thermal" ? backend.fanTemperatureString : backend.color)
                                                font.pixelSize: 14
                                                font.bold: true
                                                font.family: "monospace"
                                                color: "white"
                                            }
                                        }
                                    }
                                }
                            }

                            // Thermaltake Fans & Thermal Telemetry Card
                            Rectangle {
                                Layout.fillWidth: true
                                radius: Math.min(10, theme.radius)
                                color: "#111116"
                                border.color: (backend.mode === "thermal") ? backend.fanThermalStatusColor : Qt.rgba(1, 1, 1, 0.08)
                                border.width: (backend.mode === "thermal") ? 2 : 1

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 14

                                    // Card Header
                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 10

                                        Rectangle {
                                            width: 28
                                            height: 28
                                            radius: 14
                                            color: Qt.rgba(255, 255, 255, 0.07)
                                            Text {
                                                anchors.centerIn: parent
                                                text: "❋"
                                                font.pixelSize: 16
                                                color: backend.fanThermalStatusColor
                                                RotationAnimation on rotation {
                                                    running: backend.fanSpeedEstimate > 0
                                                    loops: Animation.Infinite
                                                    from: 0
                                                    to: 360
                                                    duration: Math.max(350, 2400 - (backend.fanSpeedEstimate * 19))
                                                }
                                            }
                                        }

                                        ColumnLayout {
                                            spacing: 1
                                            Text {
                                                text: qsTr("THERMALTAKE FANS & TEMPERATURES")
                                                font.pixelSize: 11
                                                font.bold: true
                                                font.letterSpacing: 1.2
                                                color: "white"
                                            }
                                            Text {
                                                text: backend.primarySensorName.length > 0
                                                    ? qsTr("Primary Sensor: %1").arg(backend.primarySensorName)
                                                    : qsTr("Hardware Thermal Sensors")
                                                font.pixelSize: 10
                                                color: "#888892"
                                            }
                                        }

                                        Item { Layout.fillWidth: true }

                                        // Status badge
                                        Rectangle {
                                            height: 24
                                            width: statPillText.implicitWidth + 20
                                            radius: 12
                                            color: Qt.rgba(0, 0, 0, 0.5)
                                            border.color: backend.fanThermalStatusColor
                                            border.width: 1

                                            RowLayout {
                                                anchors.centerIn: parent
                                                spacing: 6
                                                Rectangle {
                                                    width: 6
                                                    height: 6
                                                    radius: 3
                                                    color: backend.fanThermalStatusColor
                                                }
                                                Text {
                                                    id: statPillText
                                                    text: qsTr("%1 • %2").arg(backend.fanThermalStatus.toUpperCase(), backend.fanTemperatureString)
                                                    font.pixelSize: 10
                                                    font.bold: true
                                                    color: backend.fanThermalStatusColor
                                                }
                                            }
                                        }

                                        ToolButton {
                                            text: "↻"
                                            font.bold: true
                                            onClicked: backend.refreshThermals()
                                        }
                                    }

                                    // Gauge & Metrics Split Row
                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 14

                                        // Temperature Metric Box
                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 88
                                            radius: Math.min(8, theme.radius)
                                            color: "#181820"
                                            border.color: Qt.rgba(1, 1, 1, 0.05)
                                            border.width: 1

                                            ColumnLayout {
                                                anchors.fill: parent
                                                anchors.margins: 12
                                                spacing: 6

                                                RowLayout {
                                                    Layout.fillWidth: true
                                                    Text {
                                                        text: qsTr("FAN TEMPERATURE")
                                                        font.pixelSize: 10
                                                        font.bold: true
                                                        color: "#888892"
                                                    }
                                                    Item { Layout.fillWidth: true }
                                                    Text {
                                                        text: backend.fanThermalStatus
                                                        font.pixelSize: 10
                                                        font.bold: true
                                                        color: backend.fanThermalStatusColor
                                                    }
                                                }

                                                Text {
                                                    text: backend.fanTemperatureString
                                                    font.pixelSize: 22
                                                    font.bold: true
                                                    color: "white"
                                                }

                                                Rectangle {
                                                    Layout.fillWidth: true
                                                    height: 6
                                                    radius: 3
                                                    color: "#282834"

                                                    Rectangle {
                                                        height: parent.height
                                                        radius: 3
                                                        width: parent.width * Math.max(0.06, Math.min(1.0, (backend.fanTemperature - 25.0) / 65.0))
                                                        color: backend.fanThermalStatusColor

                                                        Behavior on width {
                                                            NumberAnimation { duration: 300 }
                                                        }
                                                        Behavior on color {
                                                            ColorAnimation { duration: 300 }
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        // Fan Speed Box
                                        Rectangle {
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 88
                                            radius: Math.min(8, theme.radius)
                                            color: "#181820"
                                            border.color: Qt.rgba(1, 1, 1, 0.05)
                                            border.width: 1

                                            ColumnLayout {
                                                anchors.fill: parent
                                                anchors.margins: 12
                                                spacing: 6

                                                RowLayout {
                                                    Layout.fillWidth: true
                                                    Text {
                                                        text: qsTr("FAN SPEED")
                                                        font.pixelSize: 10
                                                        font.bold: true
                                                        color: "#888892"
                                                    }
                                                    Item { Layout.fillWidth: true }
                                                    Text {
                                                        text: backend.fanRpm > 0 ? qsTr("%1 RPM").arg(backend.fanRpm) : qsTr("ESTIMATE")
                                                        font.pixelSize: 10
                                                        font.bold: true
                                                        color: theme.accent
                                                    }
                                                }

                                                Text {
                                                    text: backend.fanSpeedEstimate + "%"
                                                    font.pixelSize: 22
                                                    font.bold: true
                                                    color: "white"
                                                }

                                                Rectangle {
                                                    Layout.fillWidth: true
                                                    height: 6
                                                    radius: 3
                                                    color: "#282834"

                                                    Rectangle {
                                                        height: parent.height
                                                        radius: 3
                                                        width: parent.width * (backend.fanSpeedEstimate / 100.0)
                                                        color: theme.accent

                                                        Behavior on width {
                                                            NumberAnimation { duration: 300 }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    // Fan Profiles & Thermal Reactive toggle
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 6

                                        RowLayout {
                                            Layout.fillWidth: true
                                            Text {
                                                text: qsTr("THERMALTAKE FAN PROFILE")
                                                font.pixelSize: 10
                                                font.bold: true
                                                font.letterSpacing: 1.1
                                                color: "#888892"
                                            }
                                            Item { Layout.fillWidth: true }
                                            Button {
                                                text: backend.thermalReactive ? qsTr("Thermal Reactive RGB Active (T)") : qsTr("Sync RGB to Thermals (T)")
                                                flat: true
                                                highlighted: backend.thermalReactive
                                                font.pixelSize: 11
                                                Material.roundedScale: theme.radius
                                                onClicked: backend.toggleThermalReactive()
                                            }
                                        }

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 8

                                            Repeater {
                                                model: [
                                                    { name: qsTr("Silent"), id: "silent" },
                                                    { name: qsTr("Standard"), id: "standard" },
                                                    { name: qsTr("Performance"), id: "performance" },
                                                    { name: qsTr("Full Speed"), id: "full_speed" }
                                                ]

                                                delegate: Button {
                                                    Layout.fillWidth: true
                                                    text: modelData.name
                                                    highlighted: backend.fanSpeedProfile === modelData.id
                                                    Material.roundedScale: theme.radius
                                                    onClicked: backend.setFanSpeedProfile(modelData.id)
                                                }
                                            }
                                        }
                                    }

                                    // Additional Sensors mini chips
                                    Flow {
                                        Layout.fillWidth: true
                                        spacing: 8
                                        visible: backend.thermalSensors.length > 0

                                        Repeater {
                                            model: backend.thermalSensors

                                            delegate: Rectangle {
                                                height: 22
                                                width: sensorChipText.implicitWidth + 14
                                                radius: 11
                                                color: Qt.rgba(255, 255, 255, 0.05)
                                                border.color: Qt.rgba(255, 255, 255, 0.08)
                                                border.width: 1

                                                Text {
                                                    id: sensorChipText
                                                    anchors.centerIn: parent
                                                    text: modelData.name + ": " + modelData.tempStr
                                                    font.pixelSize: 10
                                                    color: "#aaa"
                                                }
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
                                            { name: qsTr("Thermal"), id: "thermal" },
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
                width: 500
                height: 420
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

                        Text { text: "T"; font.bold: true; font.family: "monospace"; color: theme.accent }
                        Text { text: qsTr("Toggle Thermal Reactive lighting mode"); color: "#eee" }

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

#include "backend.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTcpSocket>

Backend::Backend(QObject *parent) : QObject(parent) {
    checkDriverStatus();
    readThermalSensors();
    detectHardware();

    m_thermalTimer = new QTimer(this);
    connect(m_thermalTimer, &QTimer::timeout, this, &Backend::readThermalSensors);
    m_thermalTimer->start(2000);
}

void Backend::checkDriverStatus() {
    const QString openRgbPath = QStandardPaths::findExecutable("openrgb");
    m_openRgbInstalled = !openRgbPath.isEmpty();

    // Check if OpenRGB SDK server is listening on port 6742
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 6742);
    m_openRgbRunning = socket.waitForConnected(150);
    if (m_openRgbRunning)
        socket.disconnectFromHost();

    if (m_openRgbInstalled) {
        m_driverStatus = m_openRgbRunning
            ? tr("OpenRGB daemon active (port 6742)")
            : tr("OpenRGB CLI ready");
    } else {
        m_driverStatus = tr("Driver required: install with 'sudo pacman -S openrgb'");
    }

    emit driverStatusChanged();
}

void Backend::detectHardware() {
    m_devices.clear();
    m_thermaltakeHardwareFound = false;

    // Always include a global "All Devices" option
    QVariantMap allDevices;
    allDevices["id"] = -1;
    allDevices["name"] = tr("All Synced Devices");
    allDevices["type"] = "global";
    allDevices["summary"] = tr("Broadcasts lighting to all connected hardware");
    allDevices["icon"] = "flare";
    allDevices["active"] = true;
    m_devices.append(allDevices);

    bool foundHardware = false;

    // Check if openrgb can list devices
    if (m_openRgbInstalled) {
        QProcess proc;
        proc.start("openrgb", {"--list-devices"});
        if (proc.waitForFinished(1500) && proc.exitCode() == 0) {
            const QString output = QString::fromUtf8(proc.readAllStandardOutput());
            const QStringList lines = output.split('\n');
            QRegularExpression devRegex(R"re(^(\d+):\s*(.+)$)re");
            for (const QString &line : lines) {
                const auto match = devRegex.match(line.trimmed());
                if (match.hasMatch()) {
                    foundHardware = true;
                    const QString devName = match.captured(2).trimmed();
                    if (devName.contains("Thermaltake", Qt::CaseInsensitive) || devName.contains("Riing", Qt::CaseInsensitive)) {
                        m_thermaltakeHardwareFound = true;
                    }
                    QVariantMap dev;
                    dev["id"] = match.captured(1).toInt();
                    dev["name"] = devName;
                    dev["type"] = "hardware";
                    dev["summary"] = tr("OpenRGB Device #%1").arg(match.captured(1));
                    dev["icon"] = "devices";
                    dev["active"] = true;
                    m_devices.append(dev);
                }
            }
        }
    }

    // Direct hardware detection from USB sysfs
    QFile usbDevices("/sys/kernel/debug/usb/devices");
    QString usbInfo;
    if (usbDevices.open(QIODevice::ReadOnly)) {
        usbInfo = QString::fromUtf8(usbDevices.readAll());
    } else {
        QDir sysUsb("/sys/bus/usb/devices");
        const auto entries = sysUsb.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const auto &entry : entries) {
            QFile vendorFile(sysUsb.filePath(entry) + "/idVendor");
            QFile productFile(sysUsb.filePath(entry) + "/idProduct");
            QFile nameFile(sysUsb.filePath(entry) + "/product");
            if (vendorFile.open(QIODevice::ReadOnly) && productFile.open(QIODevice::ReadOnly)) {
                usbInfo += QString::fromUtf8(vendorFile.readAll()).trimmed() + ":"
                         + QString::fromUtf8(productFile.readAll()).trimmed() + " ";
                if (nameFile.open(QIODevice::ReadOnly)) {
                    usbInfo += QString::fromUtf8(nameFile.readAll()).trimmed();
                }
                usbInfo += "\n";
            }
        }
    }

    // Check if Thermaltake USB controller is present (VID 264a or Riing/Thermaltake descriptors)
    if (usbInfo.contains("264a:", Qt::CaseInsensitive) ||
        usbInfo.contains("Thermaltake", Qt::CaseInsensitive) ||
        usbInfo.contains("Riing", Qt::CaseInsensitive)) {
        m_thermaltakeHardwareFound = true;
    }

    if (!foundHardware) {
        if (usbInfo.contains("1462:7d73", Qt::CaseInsensitive) || usbInfo.contains("1462", Qt::CaseInsensitive)) {
            QVariantMap msi;
            msi["id"] = 0;
            msi["name"] = tr("MSI Mystic Light");
            msi["type"] = "motherboard";
            msi["summary"] = tr("Motherboard & ARGB Headers (USB 1462:7d73)");
            msi["icon"] = "memory";
            msi["active"] = true;
            m_devices.append(msi);
        }

        if (usbInfo.contains("2516:0145", Qt::CaseInsensitive) || usbInfo.contains("2516", Qt::CaseInsensitive)) {
            QVariantMap cm;
            cm["id"] = 1;
            cm["name"] = tr("Cooler Master CK550 V2");
            cm["type"] = "keyboard";
            cm["summary"] = tr("RGB Mechanical Keyboard (USB 2516:0145)");
            cm["icon"] = "keyboard";
            cm["active"] = true;
            m_devices.append(cm);
        }

        if (usbInfo.contains("3633:0002", Qt::CaseInsensitive) || usbInfo.contains("3633", Qt::CaseInsensitive)) {
            QVariantMap dc;
            dc["id"] = 2;
            dc["name"] = tr("DeepCool AK620 Digital");
            dc["type"] = "display";
            dc["summary"] = tr("CPU Cooler Display Unit (USB 3633:0002)");
            dc["icon"] = "speed";
            dc["active"] = true;
            m_devices.append(dc);
        }
    }

    // Always register Thermaltake Fans & Thermals in device list
    QVariantMap tt;
    tt["id"] = 99;
    tt["name"] = m_thermaltakeHardwareFound ? tr("Thermaltake Riing / Fans") : tr("Thermaltake Fans & Thermals");
    tt["type"] = "fans";
    tt["summary"] = tr("Active thermal monitoring • %1 (%2)").arg(fanTemperatureString(), m_fanSpeedProfile.toUpper());
    tt["icon"] = "toys";
    tt["temp"] = fanTemperatureString();
    tt["status"] = m_fanThermalStatus;
    tt["active"] = true;
    m_devices.append(tt);

    emit devicesChanged();
}

QString Backend::fanTemperatureString() const {
    return QString::asprintf("%.1f°C", m_fanTemperature);
}

void Backend::readThermalSensors() {
    m_thermalSensors.clear();
    double detectedCpuTemp = -1.0;
    int detectedFanRpm = 0;

    QDir hwmonDir("/sys/class/hwmon");
    if (hwmonDir.exists()) {
        const auto hwmonEntries = hwmonDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString &entry : hwmonEntries) {
            const QString basePath = hwmonDir.filePath(entry);
            QString driverName;
            QFile nameFile(basePath + "/name");
            if (nameFile.open(QIODevice::ReadOnly)) {
                driverName = QString::fromUtf8(nameFile.readAll()).trimmed();
            }

            // Look for temperature inputs
            QDir entryDir(basePath);
            const auto tempInputs = entryDir.entryList({"temp*_input"}, QDir::Files, QDir::Name);
            for (const QString &tempInput : tempInputs) {
                const QString inputPath = entryDir.filePath(tempInput);
                const QString prefix = tempInput.left(tempInput.length() - 6);
                QString label = prefix;
                QFile labelFile(entryDir.filePath(prefix + "_label"));
                if (labelFile.open(QIODevice::ReadOnly)) {
                    label = QString::fromUtf8(labelFile.readAll()).trimmed();
                }

                QFile valFile(inputPath);
                if (valFile.open(QIODevice::ReadOnly)) {
                    bool ok = false;
                    qint64 milliDeg = QString::fromUtf8(valFile.readAll()).trimmed().toLongLong(&ok);
                    if (ok && milliDeg > 0 && milliDeg < 150000) {
                        double deg = milliDeg / 1000.0;
                        QString friendlyCategory;
                        if (driverName == "k10temp" || driverName == "coretemp" || driverName == "zenpower") {
                            friendlyCategory = tr("CPU (%1)").arg(label);
                            if (detectedCpuTemp < 0.0 || label.contains("Tctl", Qt::CaseInsensitive) || label.contains("Package", Qt::CaseInsensitive)) {
                                detectedCpuTemp = deg;
                                m_primarySensorName = QString("%1: %2").arg(driverName, label);
                            }
                        } else if (driverName == "spd5118") {
                            friendlyCategory = tr("DDR5 Memory");
                        } else if (driverName == "nvme") {
                            friendlyCategory = tr("NVMe Storage (%1)").arg(label);
                        } else {
                            friendlyCategory = QString("%1 (%2)").arg(driverName, label);
                        }

                        QVariantMap sensorItem;
                        sensorItem["driver"] = driverName;
                        sensorItem["label"] = label;
                        sensorItem["name"] = friendlyCategory;
                        sensorItem["temp"] = deg;
                        sensorItem["tempStr"] = QString::asprintf("%.1f°C", deg);
                        m_thermalSensors.append(sensorItem);
                    }
                }
            }

            // Look for fan speed inputs
            const auto fanInputs = entryDir.entryList({"fan*_input"}, QDir::Files, QDir::Name);
            for (const QString &fanInput : fanInputs) {
                QFile fanFile(entryDir.filePath(fanInput));
                if (fanFile.open(QIODevice::ReadOnly)) {
                    bool ok = false;
                    int rpm = QString::fromUtf8(fanFile.readAll()).trimmed().toInt(&ok);
                    if (ok && rpm > 0 && detectedFanRpm == 0) {
                        detectedFanRpm = rpm;
                    }
                }
            }
        }
    }

    if (detectedCpuTemp > 0.0) {
        m_fanTemperature = detectedCpuTemp;
    }
    if (detectedFanRpm > 0) {
        m_fanRpm = detectedFanRpm;
    }

    updateThermalMetrics();
}

void Backend::updateThermalMetrics() {
    // Fan profile calculations
    if (m_fanSpeedProfile == "silent") {
        if (m_fanTemperature <= 40.0) {
            m_fanSpeedEstimate = 30;
        } else if (m_fanTemperature >= 75.0) {
            m_fanSpeedEstimate = 100;
        } else {
            m_fanSpeedEstimate = 30 + static_cast<int>((m_fanTemperature - 40.0) * (70.0 / 35.0));
        }
    } else if (m_fanSpeedProfile == "performance") {
        if (m_fanTemperature <= 40.0) {
            m_fanSpeedEstimate = 55;
        } else if (m_fanTemperature >= 65.0) {
            m_fanSpeedEstimate = 100;
        } else {
            m_fanSpeedEstimate = 55 + static_cast<int>((m_fanTemperature - 40.0) * (45.0 / 25.0));
        }
    } else if (m_fanSpeedProfile == "full_speed") {
        m_fanSpeedEstimate = 100;
    } else { // "standard"
        if (m_fanTemperature <= 40.0) {
            m_fanSpeedEstimate = 40;
        } else if (m_fanTemperature >= 70.0) {
            m_fanSpeedEstimate = 100;
        } else {
            m_fanSpeedEstimate = 40 + static_cast<int>((m_fanTemperature - 40.0) * (60.0 / 30.0));
        }
    }

    m_fanSpeedEstimate = std::clamp(m_fanSpeedEstimate, 0, 100);

    // Thermal Status
    if (m_fanTemperature < 45.0) {
        m_fanThermalStatus = tr("Cool");
        m_fanThermalStatusColor = "#00C7BE";
    } else if (m_fanTemperature < 65.0) {
        m_fanThermalStatus = tr("Normal");
        m_fanThermalStatusColor = "#30D158";
    } else if (m_fanTemperature < 80.0) {
        m_fanThermalStatus = tr("Warm");
        m_fanThermalStatusColor = "#FF9500";
    } else {
        m_fanThermalStatus = tr("Hot");
        m_fanThermalStatusColor = "#FF453A";
    }

    // If thermal reactive lighting is active, adjust color
    if (m_mode == "thermal") {
        QString newColor = calculateThermalColor(m_fanTemperature);
        if (m_color != newColor) {
            m_color = newColor;
            emit colorChanged();
            applyHardwareSettings();
        }
    }

    // Keep the Thermaltake entry in device list up to date
    for (int i = 0; i < m_devices.size(); ++i) {
        QVariantMap dev = m_devices[i].toMap();
        if (dev.value("type") == "fans") {
            dev["summary"] = tr("Active thermal monitoring • %1 (%2)").arg(fanTemperatureString(), m_fanSpeedProfile.toUpper());
            dev["temp"] = fanTemperatureString();
            dev["status"] = m_fanThermalStatus;
            m_devices[i] = dev;
            break;
        }
    }

    emit thermalUpdated();
}

QString Backend::calculateThermalColor(double temp) const {
    if (temp < 40.0) {
        return "#00C7BE"; // Electric Cyan
    } else if (temp < 60.0) {
        return "#30D158"; // Fresh Green
    } else if (temp < 75.0) {
        return "#FFD60A"; // Omarchy Gold
    } else if (temp < 85.0) {
        return "#FF9500"; // Warm Amber
    } else {
        return "#FF2D55"; // Hot Crimson
    }
}

void Backend::setFanSpeedProfile(const QString &profile) {
    if (m_fanSpeedProfile == profile)
        return;
    m_fanSpeedProfile = profile;
    emit fanSpeedProfileChanged();
    updateThermalMetrics();
}

void Backend::toggleThermalReactive() {
    if (m_mode == "thermal") {
        setMode("static");
    } else {
        setMode("thermal");
    }
}

void Backend::refreshThermals() {
    readThermalSensors();
}

void Backend::primaryAction() {
    togglePower();
}

void Backend::togglePower() {
    m_lightsOn = !m_lightsOn;
    emit lightsOnChanged();
    applyHardwareSettings();
}

void Backend::setMode(const QString &mode) {
    if (m_mode == mode)
        return;
    m_mode = mode;
    if (m_mode == "off") {
        m_lightsOn = false;
        emit lightsOnChanged();
    } else if (!m_lightsOn) {
        m_lightsOn = true;
        emit lightsOnChanged();
    }

    if (m_mode == "thermal") {
        m_color = calculateThermalColor(m_fanTemperature);
        emit colorChanged();
        emit thermalReactiveChanged();
    } else {
        emit thermalReactiveChanged();
    }

    emit modeChanged();
    applyHardwareSettings();
}

void Backend::setColor(const QString &color) {
    if (m_color == color)
        return;
    m_color = color;
    m_syncWithTheme = false;
    if (m_mode == "thermal") {
        m_mode = "static";
        emit modeChanged();
        emit thermalReactiveChanged();
    }
    emit syncWithThemeChanged();
    emit colorChanged();
    if (!m_lightsOn) {
        m_lightsOn = true;
        emit lightsOnChanged();
    }
    applyHardwareSettings();
}

void Backend::setBrightness(int brightness) {
    int clamped = std::clamp(brightness, 0, 100);
    if (m_brightness == clamped)
        return;
    m_brightness = clamped;
    emit brightnessChanged();
    applyHardwareSettings();
}

void Backend::applyThemeAccent(const QString &accentHex) {
    if (accentHex.isEmpty())
        return;
    m_color = accentHex;
    m_syncWithTheme = true;
    m_lightsOn = true;
    m_mode = "static";
    emit colorChanged();
    emit syncWithThemeChanged();
    emit lightsOnChanged();
    emit modeChanged();
    emit thermalReactiveChanged();
    applyHardwareSettings();
}

void Backend::setSyncWithTheme(bool sync) {
    if (m_syncWithTheme == sync)
        return;
    m_syncWithTheme = sync;
    emit syncWithThemeChanged();
}

void Backend::selectDevice(int index) {
    if (index >= 0 && index < m_devices.size() && m_selectedDeviceIndex != index) {
        m_selectedDeviceIndex = index;
        emit selectedDeviceIndexChanged();
    }
}

void Backend::nextDevice() {
    if (m_devices.isEmpty())
        return;
    selectDevice((m_selectedDeviceIndex + 1) % m_devices.size());
}

void Backend::prevDevice() {
    if (m_devices.isEmpty())
        return;
    selectDevice((m_selectedDeviceIndex - 1 + m_devices.size()) % m_devices.size());
}

void Backend::applyPreset(int presetIndex) {
    static const QStringList presets = {
        "#FFD60A", // Omarchy Gold
        "#FF9500", // Amber
        "#00C7BE", // Electric Cyan
        "#AF52DE", // Vaporwave Purple
        "#FF2D55"  // Crimson Rose
    };

    if (presetIndex >= 0 && presetIndex < presets.size()) {
        setColor(presets[presetIndex]);
    }
}

void Backend::rescan() {
    checkDriverStatus();
    readThermalSensors();
    detectHardware();
    applyHardwareSettings();
}

void Backend::applyHardwareSettings() {
    if (!m_openRgbInstalled)
        return;

    QString cleanColor = m_color;
    if (cleanColor.startsWith('#'))
        cleanColor.remove(0, 1);

    QStringList args;

    if (!m_lightsOn || m_mode == "off") {
        args << "--mode" << "off" << "--color" << "000000";
    } else {
        if (m_mode == "static" || m_mode == "thermal") {
            args << "--mode" << "static" << "--color" << cleanColor;
        } else if (m_mode == "breathing") {
            args << "--mode" << "breathing" << "--color" << cleanColor;
        } else if (m_mode == "spectrum") {
            args << "--mode" << "spectrum";
        }

        if (m_brightness > 0 && m_brightness <= 100) {
            args << "--brightness" << QString::number(m_brightness);
        }
    }

    if (m_selectedDeviceIndex > 0 && m_selectedDeviceIndex < m_devices.size()) {
        const QVariantMap dev = m_devices[m_selectedDeviceIndex].toMap();
        int devId = dev.value("id", -1).toInt();
        if (devId >= 0 && dev.value("type") != "fans") {
            args.prepend(QString::number(devId));
            args.prepend("--device");
        }
    }

    QProcess::startDetached("openrgb", args);
}

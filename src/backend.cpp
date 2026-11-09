#include "backend.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTcpSocket>

Backend::Backend(QObject *parent) : QObject(parent) {
    checkDriverStatus();
    detectHardware();
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
                    QVariantMap dev;
                    dev["id"] = match.captured(1).toInt();
                    dev["name"] = match.captured(2).trimmed();
                    dev["type"] = "hardware";
                    dev["summary"] = tr("OpenRGB Device #%1").arg(match.captured(1));
                    dev["icon"] = "devices";
                    dev["active"] = true;
                    m_devices.append(dev);
                }
            }
        }
    }

    // Fallback: direct hardware detection from USB sysfs
    if (!foundHardware) {
        QFile usbDevices("/sys/kernel/debug/usb/devices");
        QString usbInfo;
        if (usbDevices.open(QIODevice::ReadOnly)) {
            usbInfo = QString::fromUtf8(usbDevices.readAll());
        } else {
            // Check /sys/bus/usb/devices
            QDir sysUsb("/sys/bus/usb/devices");
            const auto entries = sysUsb.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const auto &entry : entries) {
                QFile vendorFile(sysUsb.filePath(entry) + "/idVendor");
                QFile productFile(sysUsb.filePath(entry) + "/idProduct");
                if (vendorFile.open(QIODevice::ReadOnly) && productFile.open(QIODevice::ReadOnly)) {
                    usbInfo += QString::fromUtf8(vendorFile.readAll()).trimmed() + ":"
                             + QString::fromUtf8(productFile.readAll()).trimmed() + "\n";
                }
            }
        }

        // Check for known user devices
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

    emit devicesChanged();
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
    emit modeChanged();
    applyHardwareSettings();
}

void Backend::setColor(const QString &color) {
    if (m_color == color)
        return;
    m_color = color;
    m_syncWithTheme = false;
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
        if (m_mode == "static") {
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
        if (devId >= 0) {
            args.prepend(QString::number(devId));
            args.prepend("--device");
        }
    }

    QProcess::startDetached("openrgb", args);
}

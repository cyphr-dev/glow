#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool lightsOn READ lightsOn NOTIFY lightsOnChanged)
    Q_PROPERTY(QString mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QString color READ color NOTIFY colorChanged)
    Q_PROPERTY(int brightness READ brightness NOTIFY brightnessChanged)
    Q_PROPERTY(bool syncWithTheme READ syncWithTheme NOTIFY syncWithThemeChanged)
    Q_PROPERTY(bool openRgbInstalled READ openRgbInstalled NOTIFY driverStatusChanged)
    Q_PROPERTY(bool openRgbRunning READ openRgbRunning NOTIFY driverStatusChanged)
    Q_PROPERTY(QString driverStatus READ driverStatus NOTIFY driverStatusChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(int selectedDeviceIndex READ selectedDeviceIndex NOTIFY selectedDeviceIndexChanged)
    Q_PROPERTY(int deviceCount READ deviceCount NOTIFY devicesChanged)

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend() override = default;

    bool lightsOn() const { return m_lightsOn; }
    QString mode() const { return m_mode; }
    QString color() const { return m_color; }
    int brightness() const { return m_brightness; }
    bool syncWithTheme() const { return m_syncWithTheme; }
    bool openRgbInstalled() const { return m_openRgbInstalled; }
    bool openRgbRunning() const { return m_openRgbRunning; }
    QString driverStatus() const { return m_driverStatus; }
    QVariantList devices() const { return m_devices; }
    int selectedDeviceIndex() const { return m_selectedDeviceIndex; }
    int deviceCount() const { return m_devices.size(); }

    Q_INVOKABLE void primaryAction();
    Q_INVOKABLE void togglePower();
    Q_INVOKABLE void setMode(const QString &mode);
    Q_INVOKABLE void setColor(const QString &color);
    Q_INVOKABLE void setBrightness(int brightness);
    Q_INVOKABLE void applyThemeAccent(const QString &accentHex);
    Q_INVOKABLE void setSyncWithTheme(bool sync);
    Q_INVOKABLE void selectDevice(int index);
    Q_INVOKABLE void nextDevice();
    Q_INVOKABLE void prevDevice();
    Q_INVOKABLE void applyPreset(int presetIndex);
    Q_INVOKABLE void rescan();

signals:
    void lightsOnChanged();
    void modeChanged();
    void colorChanged();
    void brightnessChanged();
    void syncWithThemeChanged();
    void driverStatusChanged();
    void devicesChanged();
    void selectedDeviceIndexChanged();

private:
    void detectHardware();
    void applyHardwareSettings();
    void checkDriverStatus();

    bool m_lightsOn = true;
    QString m_mode = "static";
    QString m_color = "#FFD60A";
    int m_brightness = 100;
    bool m_syncWithTheme = true;
    bool m_openRgbInstalled = false;
    bool m_openRgbRunning = false;
    QString m_driverStatus;
    QVariantList m_devices;
    int m_selectedDeviceIndex = 0;
};

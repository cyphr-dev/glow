#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
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

    // Thermaltake & Thermal Telemetry
    Q_PROPERTY(double fanTemperature READ fanTemperature NOTIFY thermalUpdated)
    Q_PROPERTY(QString fanTemperatureString READ fanTemperatureString NOTIFY thermalUpdated)
    Q_PROPERTY(QString fanThermalStatus READ fanThermalStatus NOTIFY thermalUpdated)
    Q_PROPERTY(QString fanThermalStatusColor READ fanThermalStatusColor NOTIFY thermalUpdated)
    Q_PROPERTY(int fanSpeedEstimate READ fanSpeedEstimate NOTIFY thermalUpdated)
    Q_PROPERTY(int fanRpm READ fanRpm NOTIFY thermalUpdated)
    Q_PROPERTY(QString fanSpeedProfile READ fanSpeedProfile NOTIFY fanSpeedProfileChanged)
    Q_PROPERTY(bool thermalReactive READ thermalReactive NOTIFY thermalReactiveChanged)
    Q_PROPERTY(QVariantList thermalSensors READ thermalSensors NOTIFY thermalUpdated)
    Q_PROPERTY(QString primarySensorName READ primarySensorName NOTIFY thermalUpdated)
    Q_PROPERTY(bool thermaltakeHardwareFound READ thermaltakeHardwareFound NOTIFY devicesChanged)

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

    double fanTemperature() const { return m_fanTemperature; }
    QString fanTemperatureString() const;
    QString fanThermalStatus() const { return m_fanThermalStatus; }
    QString fanThermalStatusColor() const { return m_fanThermalStatusColor; }
    int fanSpeedEstimate() const { return m_fanSpeedEstimate; }
    int fanRpm() const { return m_fanRpm; }
    QString fanSpeedProfile() const { return m_fanSpeedProfile; }
    bool thermalReactive() const { return m_mode == "thermal"; }
    QVariantList thermalSensors() const { return m_thermalSensors; }
    QString primarySensorName() const { return m_primarySensorName; }
    bool thermaltakeHardwareFound() const { return m_thermaltakeHardwareFound; }

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

    // Thermal actions
    Q_INVOKABLE void setFanSpeedProfile(const QString &profile);
    Q_INVOKABLE void toggleThermalReactive();
    Q_INVOKABLE void refreshThermals();

signals:
    void lightsOnChanged();
    void modeChanged();
    void colorChanged();
    void brightnessChanged();
    void syncWithThemeChanged();
    void driverStatusChanged();
    void devicesChanged();
    void selectedDeviceIndexChanged();
    void thermalUpdated();
    void fanSpeedProfileChanged();
    void thermalReactiveChanged();

private:
    void detectHardware();
    void applyHardwareSettings();
    void checkDriverStatus();
    void readThermalSensors();
    void updateThermalMetrics();
    QString calculateThermalColor(double temp) const;

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

    // Thermal & Fan fields
    double m_fanTemperature = 48.0;
    QString m_fanThermalStatus = "Normal";
    QString m_fanThermalStatusColor = "#30D158";
    int m_fanSpeedEstimate = 45;
    int m_fanRpm = 0;
    QString m_fanSpeedProfile = "standard";
    QVariantList m_thermalSensors;
    QString m_primarySensorName = "CPU Tctl";
    bool m_thermaltakeHardwareFound = false;
    QTimer *m_thermalTimer = nullptr;
};

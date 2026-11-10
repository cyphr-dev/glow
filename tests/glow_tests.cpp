#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

#include "backend.h"
#include "theme.h"
#include "translator.h"

class GlowTests : public QObject {
    Q_OBJECT

private slots:
    void primaryActionTogglesPower()
    {
        Backend backend;
        const bool initial = backend.lightsOn();
        backend.primaryAction();
        QCOMPARE(backend.lightsOn(), !initial);
        backend.primaryAction();
        QCOMPARE(backend.lightsOn(), initial);
    }

    void setColorAndModeUpdates()
    {
        Backend backend;
        backend.setColor("#00C7BE");
        QCOMPARE(backend.color(), QString("#00C7BE"));
        QCOMPARE(backend.syncWithTheme(), false);
        QCOMPARE(backend.lightsOn(), true);

        backend.setMode("breathing");
        QCOMPARE(backend.mode(), QString("breathing"));

        backend.setMode("off");
        QCOMPARE(backend.mode(), QString("off"));
        QCOMPARE(backend.lightsOn(), false);
    }

    void applyThemeAccentSyncs()
    {
        Backend backend;
        backend.applyThemeAccent("#FFD60A");
        QCOMPARE(backend.color(), QString("#FFD60A"));
        QCOMPARE(backend.syncWithTheme(), true);
        QCOMPARE(backend.lightsOn(), true);
        QCOMPARE(backend.mode(), QString("static"));
    }

    void deviceNavigation()
    {
        Backend backend;
        QVERIFY(backend.deviceCount() > 0);
        QCOMPARE(backend.selectedDeviceIndex(), 0);
        backend.nextDevice();
        if (backend.deviceCount() > 1) {
            QCOMPARE(backend.selectedDeviceIndex(), 1);
            backend.prevDevice();
            QCOMPARE(backend.selectedDeviceIndex(), 0);
        }
    }

    void applyPresets()
    {
        Backend backend;
        backend.applyPreset(1); // Amber
        QCOMPARE(backend.color(), QString("#FF9500"));
        backend.applyPreset(2); // Cyan
        QCOMPARE(backend.color(), QString("#00C7BE"));
    }

    void thermalReadingsWork()
    {
        Backend backend;
        QVERIFY(backend.fanTemperature() > 0.0);
        QVERIFY(backend.fanTemperatureString().contains("°C"));
        QVERIFY(!backend.fanThermalStatus().isEmpty());
        QVERIFY(!backend.fanThermalStatusColor().isEmpty());
        QVERIFY(backend.fanSpeedEstimate() >= 0 && backend.fanSpeedEstimate() <= 100);
    }

    void fanProfileChanges()
    {
        Backend backend;
        backend.setFanSpeedProfile("silent");
        QCOMPARE(backend.fanSpeedProfile(), QString("silent"));

        backend.setFanSpeedProfile("performance");
        QCOMPARE(backend.fanSpeedProfile(), QString("performance"));

        backend.setFanSpeedProfile("full_speed");
        QCOMPARE(backend.fanSpeedProfile(), QString("full_speed"));
        QCOMPARE(backend.fanSpeedEstimate(), 100);

        backend.setFanSpeedProfile("standard");
        QCOMPARE(backend.fanSpeedProfile(), QString("standard"));
    }

    void thermalReactiveMode()
    {
        Backend backend;
        backend.setMode("thermal");
        QCOMPARE(backend.mode(), QString("thermal"));
        QCOMPARE(backend.thermalReactive(), true);
        QVERIFY(backend.color().startsWith("#"));

        backend.toggleThermalReactive();
        QCOMPARE(backend.mode(), QString("static"));
        QCOMPARE(backend.thermalReactive(), false);

        backend.toggleThermalReactive();
        QCOMPARE(backend.mode(), QString("thermal"));
        QCOMPARE(backend.thermalReactive(), true);
    }

    void thermaltakeDeviceIncluded()
    {
        Backend backend;
        bool foundTt = false;
        const auto devs = backend.devices();
        for (const auto &val : devs) {
            const QVariantMap dev = val.toMap();
            if (dev.value("type") == "fans" && dev.value("name").toString().contains("Thermaltake")) {
                foundTt = true;
                QVERIFY(dev.contains("temp"));
                QVERIFY(dev.contains("status"));
                break;
            }
        }
        QVERIFY(foundTt);
    }

    void themeFallsBack()
    {
        QTemporaryDir dir;
        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QString("#FFD60A"));
        QCOMPARE(theme.accentForeground(), QString("black"));
    }

    void themeReadsAccent()
    {
        QTemporaryDir dir;
        QDir(dir.path()).mkpath("theme");
        QFile colors(dir.path() + "/theme/colors.toml");
        QVERIFY(colors.open(QIODevice::WriteOnly));
        colors.write("accent = \"#112233\"\n");
        colors.close();

        Theme theme(dir.path());
        QCOMPARE(theme.accent(), QString("#112233"));
        QCOMPARE(theme.accentForeground(), QString("white"));
    }

    void themeFollowsHyprlandRounding()
    {
        QTemporaryDir dir;
        const QString response = dir.filePath("response.json"), command = dir.filePath("hyprctl");
        auto write = [](const QString &path, const QByteArray &data) {
            QFile file(path);
            return file.open(QIODevice::WriteOnly) && file.write(data) == data.size();
        };
        QVERIFY(write(command, "#!/bin/sh\ncat '" + response.toUtf8() + "'\n"));
        QVERIFY(QFile::setPermissions(command, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        QVERIFY(write(response, "{\"int\":0}"));

        Theme theme(dir.path(), nullptr, command);
        QCOMPARE(theme.radius(), 0);
        QVERIFY(write(response, "{\"int\":6}"));
        QTRY_COMPARE(theme.radius(), 6);
    }

    void translatorFallsBackToEnglish()
    {
        QTemporaryDir dir;
        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.language(), QString("en"));
        QVERIFY(translator.translate("Main", "Glow").isNull());
    }

    void translatorFollowsTheLanguageFile()
    {
        QTemporaryDir dir;
        write(dir.path() + "/language", "pt-BR\n");
        write(dir.path() + "/pt.json", R"({"Glow": "Brilho", "Rescan": "Reescanear"})");
        write(dir.path() + "/pt-BR.json", R"({"Glow": "Glow"})");

        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.language(), QString("pt-BR"));
        QCOMPARE(translator.translate("Main", "Glow"), QString("Glow"));
        QCOMPARE(translator.translate("Main", "Rescan"), QString("Reescanear"));
        QVERIFY(translator.translate("Main", "Quit").isNull());
    }

    void translatorPicksPluralForms()
    {
        QTemporaryDir dir;
        write(dir.path() + "/language", "ru\n");
        write(dir.path() + "/en.json", R"json({"%n device(s)": {"one": "%n device", "other": "%n devices"}})json");
        write(dir.path() + "/ru.json", R"json({"%n device(s)": {"one": "%n устройство", "few": "%n устройства", "many": "%n устройств", "other": "%n устройства"}})json");

        Translator translator(dir.path() + "/language", dir.path());
        QCOMPARE(translator.translate("Main", "%n device(s)", nullptr, 21), QString("%n устройство"));
        QCOMPARE(translator.translate("Main", "%n device(s)", nullptr, 22), QString("%n устройства"));
        QCOMPARE(translator.translate("Main", "%n device(s)", nullptr, 25), QString("%n устройств"));
        QCOMPARE(Translator::pluralCategory("ar", 11), QString("many"));
        QCOMPARE(Translator::pluralCategory("ja", 1), QString("other"));
    }

private:
    static void write(const QString &path, const QByteArray &contents)
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(contents);
    }
};

QTEST_MAIN(GlowTests)
#include "glow_tests.moc"

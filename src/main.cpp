// glow — RGB lighting controller for Omarchy.

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include "backend.h"
#include "theme.h"
#include "translator.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("glow");
    app.setApplicationVersion("0.1.0");

    // Matches the window to glow.desktop, so the compositor (Wayland app_id)
    // and the launcher pick up the installed icon.
    app.setDesktopFileName("glow");
    app.setWindowIcon(QIcon::fromTheme("glow"));

    QQuickStyle::setStyle("Material");

    // Installed before anything is translated, and every qsTr() binding is
    // re-evaluated when the Omarchy language changes.
    Translator translator;
    app.installTranslator(&translator);

    Theme theme;
    Backend backend;

    // Connect live theme accent updates to backend
    QObject::connect(&theme, &Theme::changed, &backend, [&theme, &backend]() {
        if (backend.syncWithTheme()) {
            backend.applyThemeAccent(theme.accent());
        }
    });

    if (backend.syncWithTheme()) {
        backend.applyThemeAccent(theme.accent());
    }

    QQmlApplicationEngine engine;
    QObject::connect(&translator, &Translator::changed, &engine, &QQmlEngine::retranslate);
    engine.rootContext()->setContextProperty("translator", &translator);
    engine.rootContext()->setContextProperty("theme", &theme);
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.load(QUrl("qrc:/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}

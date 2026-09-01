// Entry point.
//
// Order matters here: the Chromium switches have to be in the environment
// before Qt WebEngine initialises, and Qt WebEngine has to initialise before
// the GUI application exists.

#include "app/application.h"
#include "browser/browser_controller.h"
#include "utils/logging.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QIcon>
#include <QtQml/QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>
#include <QtWebEngineQuick/QtWebEngineQuick>

int main(int argc, char *argv[])
{
    QGuiApplication::setApplicationName(QStringLiteral("PrivacyBrowser"));
    QGuiApplication::setApplicationVersion(QStringLiteral(PB_VERSION));
    // No organization domain and no installation identifier (PLAN.md §49).

    pb::app::BrowserApplication::prepareEnvironment();
    QtWebEngineQuick::initialize();

    // The Basic style has no platform theming of its own, which is what the
    // glass components need: every control here is drawn by this project.
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/qt/qml/PrivacyBrowser/Ui/assets/icon.png")));
    pb::log::write(pb::log::Level::Info, "PrivacyBrowser " PB_VERSION " starting");

    pb::app::BrowserApplication browser;

    QQmlApplicationEngine engine;
    browser.registerWithEngine(engine);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() {
            pb::log::write(pb::log::Level::Error, "failed to create the root QML object");
            QCoreApplication::exit(1);
        },
        Qt::QueuedConnection);
    engine.loadFromModule("PrivacyBrowser.Ui", "Main");

    // Closing the last window ends the session, which is what triggers
    // cleanup. Quitting is explicit rather than relying on the default
    // quit-on-last-window-closed, because the root object is not a window.
    QObject::connect(browser.browser(), &pb::browser::BrowserController::allWindowsClosed,
                     &app, &QCoreApplication::quit);

    const int status = app.exec();

    // Explicit, before static destruction, so a cleanup failure can still be
    // logged and reported through the exit status.
    const bool cleaned = browser.shutdown();
    pb::log::write(pb::log::Level::Info, "PrivacyBrowser exiting");
    return (status == 0 && !cleaned) ? 2 : status;
}

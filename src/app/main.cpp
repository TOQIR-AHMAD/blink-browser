// Entry point.
//
// Order matters here: the Chromium switches have to be in the environment
// before Qt WebEngine initialises, and Qt WebEngine has to initialise before
// the GUI application exists.

#include "app/application.h"
#include "utils/logging.h"

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtWebEngineQuick/QtWebEngineQuick>

int main(int argc, char *argv[])
{
    QGuiApplication::setApplicationName(QStringLiteral("PrivacyBrowser"));
    QGuiApplication::setApplicationVersion(QStringLiteral(PB_VERSION));
    // No organization domain and no installation identifier (PLAN.md §49).

    pb::app::BrowserApplication::prepareEnvironment();
    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);
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

    const int status = app.exec();

    // Explicit, before static destruction, so a cleanup failure can still be
    // logged and reported through the exit status.
    const bool cleaned = browser.shutdown();
    pb::log::write(pb::log::Level::Info, "PrivacyBrowser exiting");
    return (status == 0 && !cleaned) ? 2 : status;
}

// Phase 0 entry point: brings up the Qt/QML shell and shows an empty window.
// Web content, tabs and the privacy subsystem arrive in later phases.

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "utils/logging.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Application name and version only. No organization domain and no
    // installation identifier: PLAN.md section 49 rules out a per-install ID,
    // and nothing here writes to QSettings or QStandardPaths.
    QGuiApplication::setApplicationName(QStringLiteral("PrivacyBrowser"));
    QGuiApplication::setApplicationVersion(QStringLiteral(PB_VERSION));

    pb::log::write(pb::log::Level::Info, "PrivacyBrowser " PB_VERSION " starting");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() {
            pb::log::write(pb::log::Level::Error, "failed to create the root QML object");
            QCoreApplication::exit(1);
        },
        Qt::QueuedConnection);
    engine.loadFromModule("PrivacyBrowser.Ui", "Main");

    const int status = app.exec();
    pb::log::write(pb::log::Level::Info, "PrivacyBrowser exiting");
    return status;
}

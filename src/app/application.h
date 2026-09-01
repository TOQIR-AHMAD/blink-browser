// Owns everything with a lifetime equal to the running browser.
//
// The object graph is built here once, in one place, so that the shutdown order
// is explicit: windows and tabs first, then the profile, then the session
// directory (PLAN.md §14).

#ifndef PB_APP_APPLICATION_H
#define PB_APP_APPLICATION_H

#include "privacy/core/session_paths.h"

#include <QtCore/QObject>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE
class QQmlApplicationEngine;
class QWebEngineProfile;
QT_END_NAMESPACE

namespace pb::chromium {
class WebProfile;
}

namespace pb::app {

class BrowserApplication : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QWebEngineProfile *profile READ webEngineProfile CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)

public:
    // Must run before QtWebEngineQuick::initialize() and before QGuiApplication
    // exists: it is what puts the privacy switches on Chromium's command line.
    static void prepareEnvironment();

    explicit BrowserApplication(QObject *parent = nullptr);
    ~BrowserApplication() override;

    void registerWithEngine(QQmlApplicationEngine &engine);

    // Idempotent. Runs the shutdown sequence and reports whether the session
    // directory is gone.
    bool shutdown();

    QWebEngineProfile *webEngineProfile() const;
    QString version() const;

private:
    pb::privacy::SessionPaths m_sessionPaths;
    pb::chromium::WebProfile *m_webProfile = nullptr;
    bool m_shutdownDone = false;
};

} // namespace pb::app

#endif // PB_APP_APPLICATION_H

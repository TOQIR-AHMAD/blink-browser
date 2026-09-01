// Runtime privacy verification (PLAN.md §42).
//
// Boots the real application object, exercises it, shuts it down, and then
// checks the disk. The checks are deliberately about observable state - does
// this directory exist, is this file there - rather than about how the code is
// written, which is what tests/privacy/audit_sources.py covers.

#include "app/application.h"
#include "chromium/web_profile.h"
#include "privacy/core/session_paths.h"
#include "privacy/privacy_manager.h"
#include "settings/settings_controller.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryDir>
#include <QtGui/QGuiApplication>
#include <QtTest/QTest>
#include <QtWebEngineCore/QWebEngineProfile>
#include <QtWebEngineQuick/QtWebEngineQuick>

class PersistenceTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void profileIsOffTheRecord();
    void sessionDirectoryIsCreatedInTemp();
    void shutdownRemovesTheSessionDirectory();
    void shutdownIsIdempotent();
    void noSettingsFileUnlessAskedFor();
    void noBrowsingDataInTheConfigLocation();
};

void PersistenceTest::profileIsOffTheRecord()
{
    pb::app::BrowserApplication application;
    QWebEngineProfile *profile = application.webEngineProfile();

    QVERIFY(profile != nullptr);
    QVERIFY2(profile->isOffTheRecord(), "the browsing profile must be off-the-record");
    QVERIFY(profile != QWebEngineProfile::defaultProfile());
    QCOMPARE(profile->httpCacheType(), QWebEngineProfile::MemoryHttpCache);
    QCOMPARE(profile->persistentCookiesPolicy(), QWebEngineProfile::NoPersistentCookies);
    QVERIFY(!profile->isSpellCheckEnabled());
    QVERIFY2(!profile->httpUserAgent().contains(QLatin1String("QtWebEngine")),
             "the QtWebEngine token is a fingerprint and must be gone");
}

void PersistenceTest::sessionDirectoryIsCreatedInTemp()
{
    pb::app::BrowserApplication application;
    const QString session = application.privacy()->sessionDirectory();

    QVERIFY(!session.isEmpty());
    QVERIFY(QFileInfo::exists(session));
    QVERIFY2(session.startsWith(QDir::tempPath()),
             "the session directory must live under the system temporary directory");
    QVERIFY2(!session.contains(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                               + QStringLiteral("/.privacy-browser")),
             "nothing may be written to a fixed location in the user's home");
}

void PersistenceTest::shutdownRemovesTheSessionDirectory()
{
    QString session;
    {
        pb::app::BrowserApplication application;
        session = application.privacy()->sessionDirectory();
        QVERIFY(QFileInfo::exists(session));

        // Leave something behind, the way Chromium does.
        QDir().mkpath(session + QStringLiteral("/cache"));
        QFile file(session + QStringLiteral("/cache/entry.bin"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("cached bytes");
        file.close();

        QVERIFY(application.shutdown());
    }
    QVERIFY2(!QFileInfo::exists(session),
             "the session directory and everything in it must be gone after shutdown");
}

void PersistenceTest::shutdownIsIdempotent()
{
    pb::app::BrowserApplication application;
    QVERIFY(application.shutdown());
    QVERIFY2(application.shutdown(), "a second shutdown must succeed and change nothing");
}

void PersistenceTest::noSettingsFileUnlessAskedFor()
{
    const QString path
        = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
        + QStringLiteral("/settings.json");
    QFile::remove(path);

    {
        pb::app::BrowserApplication application;
        application.settings()->setTrackerBlocking(false);
        QVERIFY2(!QFileInfo::exists(path),
                 "changing a setting must not write a file while 'remember' is off");

        application.settings()->setRememberSettings(true);
        QVERIFY2(QFileInfo::exists(path), "opting in must write the settings file");

        application.settings()->setRememberSettings(false);
        QVERIFY2(!QFileInfo::exists(path), "opting out must delete the settings file");
    }
}

void PersistenceTest::noBrowsingDataInTheConfigLocation()
{
    const QString configDirectory
        = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    pb::app::BrowserApplication application;
    application.settings()->setRememberSettings(true);

    const QStringList entries
        = QDir(configDirectory).entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        QVERIFY2(entry == QLatin1String("settings.json"),
                 qPrintable(QStringLiteral("unexpected file in the config directory: %1")
                                .arg(entry)));
    }

    application.settings()->setRememberSettings(false);
}

int main(int argc, char *argv[])
{
    // Offscreen so the test can run on a build machine, and test mode so
    // QStandardPaths points at a throwaway location instead of the developer's
    // real configuration.
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QStandardPaths::setTestModeEnabled(true);

    pb::app::BrowserApplication::prepareEnvironment();
    QtWebEngineQuick::initialize();

    QGuiApplication app(argc, argv);
    PersistenceTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_persistence.moc"

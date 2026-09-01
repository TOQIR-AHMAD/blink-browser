// Tabs, windows and the session history working together (PLAN.md §41).
//
// No web engine is started here: these are the browser's own data structures,
// which is exactly where a privacy mistake (a private window leaking into the
// history, a closed tab being remembered too long) would live.

#include "browser/browser_controller.h"
#include "browser/window_controller.h"
#include "settings/settings_controller.h"
#include "tabs/tab.h"
#include "tabs/tab_model.h"

#include <QtCore/QStandardPaths>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

using pb::browser::BrowserController;
using pb::browser::WindowController;
using pb::settings::SettingsController;
using pb::tabs::Tab;
using pb::tabs::TabModel;

class BrowserTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void tabLifecycle();
    void closingTheLastTabIsReported();
    void reorderKeepsTheSelection();
    void closedTabsCanBeReopened();
    void windowsAreIndependent();
    void privateWindowsRecordNothing();
    void addressBarResolvesInput();
};

void BrowserTest::tabLifecycle()
{
    TabModel tabs(false);
    QCOMPARE(tabs.count(), 0);
    QCOMPARE(tabs.currentIndex(), -1);

    Tab *first = tabs.addTab(QUrl(QStringLiteral("https://example.com/")));
    QVERIFY(first != nullptr);
    QCOMPARE(tabs.count(), 1);
    QCOMPARE(tabs.currentIndex(), 0);
    QCOMPARE(tabs.currentTab(), first);
    QCOMPARE(first->url(), QUrl(QStringLiteral("https://example.com/")));

    Tab *background = tabs.addTab(QUrl(QStringLiteral("https://other.example/")), true);
    QCOMPARE(tabs.count(), 2);
    QCOMPARE(tabs.currentIndex(), 0);
    QCOMPARE(tabs.tabAt(1), background);

    tabs.duplicateTab(0);
    QCOMPARE(tabs.count(), 3);
    QCOMPARE(tabs.currentTab()->url(), first->url());
    QVERIFY2(tabs.currentTab() != first, "a duplicate is a new tab, not the same one");

    // A tab command asks the view to act; it does not change state itself.
    QSignalSpy navigations(first, &Tab::navigationRequested);
    first->navigate(QUrl(QStringLiteral("https://elsewhere.example/")));
    QCOMPARE(navigations.count(), 1);
    QCOMPARE(first->url(), QUrl(QStringLiteral("https://example.com/")));
}

void BrowserTest::closingTheLastTabIsReported()
{
    TabModel tabs(false);
    tabs.addTab(QUrl(QStringLiteral("https://example.com/")));

    QSignalSpy lastClosed(&tabs, &TabModel::lastTabClosed);
    tabs.closeCurrentTab();

    QCOMPARE(tabs.count(), 0);
    QCOMPARE(tabs.currentIndex(), -1);
    QCOMPARE(lastClosed.count(), 1);
}

void BrowserTest::reorderKeepsTheSelection()
{
    TabModel tabs(false);
    Tab *a = tabs.addTab(QUrl(QStringLiteral("https://a.example/")));
    Tab *b = tabs.addTab(QUrl(QStringLiteral("https://b.example/")));
    tabs.addTab(QUrl(QStringLiteral("https://c.example/")));

    tabs.setCurrentIndex(0);
    tabs.moveTab(0, 2);

    QCOMPARE(tabs.tabAt(2), a);
    QCOMPARE(tabs.tabAt(0), b);
    QCOMPARE(tabs.currentTab(), a);
    QCOMPARE(tabs.currentIndex(), 2);
}

void BrowserTest::closedTabsCanBeReopened()
{
    TabModel tabs(false);
    tabs.addTab(QUrl(QStringLiteral("https://a.example/")));
    tabs.addTab(QUrl(QStringLiteral("https://b.example/")));

    QVERIFY(!tabs.canReopenClosedTab());
    tabs.closeTab(1);
    QVERIFY(tabs.canReopenClosedTab());

    tabs.reopenClosedTab();
    QCOMPARE(tabs.count(), 2);
    QCOMPARE(tabs.currentTab()->url(), QUrl(QStringLiteral("https://b.example/")));
    QVERIFY(!tabs.canReopenClosedTab());

    // Closing every tab clears the reopen list: nothing about browsing outlives
    // the window.
    tabs.closeTab(1);
    QVERIFY(tabs.canReopenClosedTab());
    tabs.closeAll();
    QVERIFY(!tabs.canReopenClosedTab());
    QCOMPARE(tabs.count(), 0);
}

void BrowserTest::windowsAreIndependent()
{
    SettingsController settings;
    BrowserController browser(&settings);

    QCOMPARE(browser.windowCount(), 0);
    WindowController *first = browser.createWindow(false);
    WindowController *second = browser.createWindow(false);
    QCOMPARE(browser.windowCount(), 2);

    first->tabs()->addTab(QUrl(QStringLiteral("https://a.example/")));
    QCOMPARE(first->tabs()->count(), 1);
    QCOMPARE(second->tabs()->count(), 0);

    QSignalSpy allClosed(&browser, &BrowserController::allWindowsClosed);
    browser.closeWindow(first);
    QCOMPARE(browser.windowCount(), 1);
    QCOMPARE(allClosed.count(), 0);
    browser.closeWindow(second);
    QCOMPARE(allClosed.count(), 1);
}

void BrowserTest::privateWindowsRecordNothing()
{
    SettingsController settings;
    BrowserController browser(&settings);

    WindowController *normal = browser.createWindow(false);
    WindowController *incognito = browser.createWindow(true);

    normal->recordVisit(QUrl(QStringLiteral("https://remembered.example/page")),
                        QStringLiteral("Remembered"));
    incognito->recordVisit(QUrl(QStringLiteral("https://secret.example/page")),
                           QStringLiteral("Secret"));

    QCOMPARE(static_cast<int>(browser.history().size()), 1);

    const QVariantList completions
        = normal->completions(QStringLiteral("secret"), 5);
    QVERIFY2(completions.isEmpty(),
             "a private window's pages must not appear in another window's completions");

    QVERIFY2(incognito->completions(QStringLiteral("remembered"), 5).isEmpty(),
             "a private window offers no completions at all");
    QVERIFY2(incognito->topSites(5).isEmpty(), "nor any shortcuts");

    QVERIFY(!normal->completions(QStringLiteral("remembered"), 5).isEmpty());

    browser.shutdown();
    QCOMPARE(static_cast<int>(browser.history().size()), 0);
}

void BrowserTest::addressBarResolvesInput()
{
    SettingsController settings;
    BrowserController browser(&settings);
    WindowController *window = browser.createWindow(false);

    QCOMPARE(window->resolveInput(QStringLiteral("https://example.com/a")),
             QUrl(QStringLiteral("https://example.com/a")));
    QCOMPARE(window->resolveInput(QStringLiteral("example.com")),
             QUrl(QStringLiteral("https://example.com")));

    const QUrl search = window->resolveInput(QStringLiteral("privacy browser"));
    QVERIFY(search.isValid());
    QVERIFY2(search.host().contains(QLatin1String("duckduckgo")),
             "a search goes to the configured provider");
    QVERIFY(search.toString().contains(QLatin1String("privacy%20browser")));

    const QUrl script = window->resolveInput(QStringLiteral("javascript:alert(1)"));
    QVERIFY2(script.scheme() != QLatin1String("javascript"),
             "a javascript: URL must never become a navigation");

    QVERIFY(!window->resolveInput(QStringLiteral("   ")).isValid());
}

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QStandardPaths::setTestModeEnabled(true);

    QCoreApplication app(argc, argv);
    BrowserTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_browser.moc"

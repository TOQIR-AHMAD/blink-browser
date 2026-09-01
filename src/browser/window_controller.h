// One browser window: its tabs, and the logic the window's chrome needs.
//
// A private window differs in exactly one way that matters here: nothing it
// visits is recorded in the session history, so it leaves no trace even in the
// address bar's completions of the current session (PLAN.md §27).

#ifndef PB_BROWSER_WINDOW_CONTROLLER_H
#define PB_BROWSER_WINDOW_CONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QVariantList>
#include <QtQml/qqmlregistration.h>

namespace pb::tabs {
class TabModel;
}

namespace pb::browser {

class BrowserController;

class WindowController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Windows are created by the browser controller")

    Q_PROPERTY(pb::tabs::TabModel *tabs READ tabs CONSTANT)
    Q_PROPERTY(bool privateMode READ privateMode CONSTANT)
    Q_PROPERTY(pb::browser::BrowserController *browser READ browser CONSTANT)

public:
    WindowController(BrowserController *browser, bool privateMode, QObject *parent = nullptr);

    pb::tabs::TabModel *tabs() const { return m_tabs; }
    bool privateMode() const { return m_privateMode; }
    BrowserController *browser() const { return m_browser; }

    // Turns what the user typed into something to load: an address, or a
    // search on the configured provider. Returns an invalid QUrl for input
    // that should do nothing.
    Q_INVOKABLE QUrl resolveInput(const QString &text) const;

    // Completions drawn from this session's history only - never a network
    // request (PLAN.md §10). Each entry is {url, title, host}.
    Q_INVOKABLE QVariantList completions(const QString &query, int limit = 6) const;

    // Called by the QML view when a page finishes loading. Ignored for private
    // windows.
    Q_INVOKABLE void recordVisit(const QUrl &url, const QString &title);

    Q_INVOKABLE void requestClose();
    Q_INVOKABLE void openInNewWindow(const QUrl &url, bool privateMode = false);

    // Drops the tabs and their closed-tab list.
    void shutdown();

Q_SIGNALS:
    void closeRequested();

private:
    BrowserController *m_browser = nullptr;
    pb::tabs::TabModel *m_tabs = nullptr;
    bool m_privateMode = false;
};

} // namespace pb::browser

#endif // PB_BROWSER_WINDOW_CONTROLLER_H

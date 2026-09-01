// One browser tab.
//
// The tab owns the *state* of a page (title, url, progress, security level)
// while the QML WebEngineView owns the page itself. State flows up from the
// view by assignment, commands flow down as signals. Keeping it this way means
// the tab model, and everything that reasons about tabs, stays testable and
// free of Qt WebEngine.

#ifndef PB_TABS_TAB_H
#define PB_TABS_TAB_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtQml/qqmlregistration.h>

namespace pb::tabs {

class Tab : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Tabs are created by the tab model")

    Q_PROPERTY(int id READ id CONSTANT)
    Q_PROPERTY(bool privateMode READ privateMode CONSTANT)
    Q_PROPERTY(QUrl initialUrl READ initialUrl CONSTANT)

    Q_PROPERTY(QString title MEMBER m_title NOTIFY titleChanged)
    Q_PROPERTY(QUrl url MEMBER m_url NOTIFY urlChanged)
    Q_PROPERTY(QUrl iconUrl MEMBER m_iconUrl NOTIFY iconUrlChanged)
    Q_PROPERTY(bool loading MEMBER m_loading NOTIFY loadingChanged)
    Q_PROPERTY(int loadProgress MEMBER m_loadProgress NOTIFY loadProgressChanged)
    Q_PROPERTY(bool canGoBack MEMBER m_canGoBack NOTIFY canGoBackChanged)
    Q_PROPERTY(bool canGoForward MEMBER m_canGoForward NOTIFY canGoForwardChanged)
    Q_PROPERTY(bool audible MEMBER m_audible NOTIFY audibleChanged)
    Q_PROPERTY(bool muted MEMBER m_muted NOTIFY mutedChanged)
    Q_PROPERTY(bool crashed MEMBER m_crashed NOTIFY crashedChanged)
    Q_PROPERTY(SecurityLevel securityLevel MEMBER m_securityLevel NOTIFY securityLevelChanged)
    Q_PROPERTY(QString certificateIssue MEMBER m_certificateIssue NOTIFY certificateIssueChanged)

    // Derived, for the address bar and the tab strip.
    Q_PROPERTY(QString displayHost READ displayHost NOTIFY urlChanged)
    Q_PROPERTY(QString displayTitle READ displayTitle NOTIFY titleChanged)

public:
    enum SecurityLevel {
        Unknown,     // nothing loaded yet
        Internal,    // a page the browser itself provides
        Insecure,    // plain http
        Secure,      // https with a valid certificate
        Compromised, // https with a certificate problem the user accepted
    };
    Q_ENUM(SecurityLevel)

    Tab(int id, const QUrl &initialUrl, bool privateMode, QObject *parent = nullptr);

    int id() const { return m_id; }
    bool privateMode() const { return m_privateMode; }
    QUrl initialUrl() const { return m_initialUrl; }
    QString displayHost() const;
    QString displayTitle() const;

    // Read accessors for C++ callers; QML uses the properties above.
    QUrl url() const { return m_url; }
    QString title() const { return m_title; }
    bool loading() const { return m_loading; }
    bool crashed() const { return m_crashed; }
    SecurityLevel securityLevel() const { return m_securityLevel; }

public Q_SLOTS:
    // Commands. Each one asks the view to act; nothing here changes state
    // directly, because only the view knows whether the action succeeded.
    void navigate(const QUrl &url);
    void reload();
    void reloadBypassingCache();
    void stop();
    void goBack();
    void goForward();
    void setMuted(bool muted);
    void toggleMuted();

Q_SIGNALS:
    void titleChanged();
    void urlChanged();
    void iconUrlChanged();
    void loadingChanged();
    void loadProgressChanged();
    void canGoBackChanged();
    void canGoForwardChanged();
    void audibleChanged();
    void mutedChanged();
    void crashedChanged();
    void securityLevelChanged();
    void certificateIssueChanged();

    // Consumed by the WebEngineView bound to this tab.
    void navigationRequested(const QUrl &url);
    void reloadRequested(bool bypassCache);
    void stopRequested();
    void backRequested();
    void forwardRequested();

private:
    const int m_id;
    const bool m_privateMode;
    const QUrl m_initialUrl;

    QString m_title;
    QUrl m_url;
    QUrl m_iconUrl;
    bool m_loading = false;
    int m_loadProgress = 0;
    bool m_canGoBack = false;
    bool m_canGoForward = false;
    bool m_audible = false;
    bool m_muted = false;
    bool m_crashed = false;
    SecurityLevel m_securityLevel = Unknown;
    QString m_certificateIssue;
};

} // namespace pb::tabs

#endif // PB_TABS_TAB_H

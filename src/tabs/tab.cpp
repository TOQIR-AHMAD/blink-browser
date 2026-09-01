#include "tabs/tab.h"

namespace pb::tabs {

Tab::Tab(int id, const QUrl &initialUrl, bool privateMode, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_privateMode(privateMode)
    , m_initialUrl(initialUrl)
    , m_url(initialUrl)
{
}

QString Tab::displayHost() const
{
    if (m_url.scheme() == QLatin1String("about") || m_url.isEmpty())
        return QString();
    const QString host = m_url.host();
    if (host.startsWith(QLatin1String("www.")))
        return host.mid(4);
    return host;
}

QString Tab::displayTitle() const
{
    if (!m_title.isEmpty())
        return m_title;
    const QString host = displayHost();
    return host.isEmpty() ? tr("New tab") : host;
}

void Tab::navigate(const QUrl &url)
{
    if (!url.isValid() || url.isEmpty())
        return;
    Q_EMIT navigationRequested(url);
}

void Tab::reload()
{
    Q_EMIT reloadRequested(false);
}

void Tab::reloadBypassingCache()
{
    Q_EMIT reloadRequested(true);
}

void Tab::stop()
{
    Q_EMIT stopRequested();
}

void Tab::goBack()
{
    Q_EMIT backRequested();
}

void Tab::goForward()
{
    Q_EMIT forwardRequested();
}

void Tab::setMuted(bool muted)
{
    if (m_muted == muted)
        return;
    m_muted = muted;
    Q_EMIT mutedChanged();
}

void Tab::toggleMuted()
{
    setMuted(!m_muted);
}

} // namespace pb::tabs

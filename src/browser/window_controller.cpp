#include "browser/window_controller.h"

#include "browser/browser_controller.h"
#include "browser/core/session_history.h"
#include "settings/core/omnibox_input.h"
#include "settings/core/search_engines.h"
#include "settings/settings_controller.h"
#include "tabs/tab_model.h"

#include <QtCore/QDateTime>
#include <algorithm>
#include <QtCore/QVariantMap>

namespace pb::browser {

WindowController::WindowController(BrowserController *browser, bool privateMode, QObject *parent)
    : QObject(parent)
    , m_browser(browser)
    , m_tabs(new pb::tabs::TabModel(privateMode, this))
    , m_privateMode(privateMode)
{
}

QUrl WindowController::resolveInput(const QString &text) const
{
    const pb::settings::OmniboxResult result
        = pb::settings::classifyInput(text.toStdString());

    switch (result.action) {
    case pb::settings::OmniboxAction::Nothing:
        return {};
    case pb::settings::OmniboxAction::Navigate:
        return QUrl::fromUserInput(QString::fromStdString(result.value));
    case pb::settings::OmniboxAction::Search:
        break;
    }

    const std::string searchTemplate = m_browser && m_browser->settings()
        ? m_browser->settings()->settings().searchTemplate()
        : std::string();
    const std::string url = pb::settings::buildSearchUrl(searchTemplate, result.value);
    if (url.empty())
        return {};
    return QUrl(QString::fromStdString(url));
}

QVariantList WindowController::completions(const QString &query, int limit) const
{
    QVariantList list;
    if (m_privateMode || !m_browser || limit <= 0)
        return list;

    const auto suggestions = m_browser->history().suggestions(query.toStdString(),
                                                              static_cast<std::size_t>(limit));
    for (const HistoryEntry &entry : suggestions) {
        QVariantMap item;
        const QUrl url(QString::fromStdString(entry.url));
        item[QStringLiteral("url")] = url;
        item[QStringLiteral("title")] = QString::fromStdString(entry.title);
        item[QStringLiteral("host")] = url.host();
        list.append(item);
    }
    return list;
}

QVariantList WindowController::topSites(int limit) const
{
    QVariantList list;
    if (m_privateMode || !m_browser || limit <= 0)
        return list;

    std::vector<HistoryEntry> entries = m_browser->history().entries();
    std::stable_sort(entries.begin(), entries.end(),
                     [](const HistoryEntry &a, const HistoryEntry &b) {
                         return a.visitCount > b.visitCount;
                     });

    for (const HistoryEntry &entry : entries) {
        if (list.size() >= limit)
            break;
        const QUrl url(QString::fromStdString(entry.url));
        QVariantMap item;
        item[QStringLiteral("url")] = url;
        item[QStringLiteral("title")] = QString::fromStdString(entry.title);
        item[QStringLiteral("host")] = url.host().startsWith(QLatin1String("www."))
                ? url.host().mid(4) : url.host();
        list.append(item);
    }
    return list;
}

void WindowController::recordVisit(const QUrl &url, const QString &title)
{
    if (m_privateMode || !m_browser)
        return;
    m_browser->history().recordVisit(url.toString().toStdString(), title.toStdString(),
                                     QDateTime::currentMSecsSinceEpoch());
}

void WindowController::requestClose()
{
    Q_EMIT closeRequested();
    if (m_browser)
        m_browser->closeWindow(this);
}

void WindowController::openInNewWindow(const QUrl &url, bool privateMode)
{
    if (!m_browser)
        return;
    WindowController *window = m_browser->createWindow(privateMode);
    if (window && url.isValid())
        window->tabs()->addTab(url);
}

void WindowController::shutdown()
{
    m_tabs->closeAll();
}

} // namespace pb::browser

#include "browser/core/session_history.h"

#include "network/core/url_info.h"
#include "utils/text.h"

#include <algorithm>

namespace pb::browser {
namespace {

// Ranking inputs: a host prefix match is what the user almost always means.
constexpr int kHostPrefixScore = 1000;
constexpr int kUrlPrefixScore = 600;
constexpr int kHostContainsScore = 200;
constexpr int kUrlContainsScore = 100;

int matchScore(const HistoryEntry &entry, const std::string &query)
{
    const std::string url = pb::text::toLower(entry.url);
    const pb::net::UrlInfo info = pb::net::UrlInfo::parse(url);
    const std::string host = info.host;

    if (!host.empty()) {
        if (pb::text::startsWith(host, query))
            return kHostPrefixScore;
        if (pb::text::startsWith(host, "www.") && pb::text::startsWith(host.substr(4), query))
            return kHostPrefixScore;
    }
    if (pb::text::startsWith(url, query))
        return kUrlPrefixScore;
    if (!host.empty() && host.find(query) != std::string::npos)
        return kHostContainsScore;
    if (url.find(query) != std::string::npos)
        return kUrlContainsScore;
    if (pb::text::toLower(entry.title).find(query) != std::string::npos)
        return kUrlContainsScore / 2;
    return 0;
}

bool isRecordable(const pb::net::UrlInfo &info)
{
    // about:, data: and view-source: entries are noise, and file: URLs would
    // put local paths into the suggestion list.
    return info.valid && (info.scheme == "http" || info.scheme == "https") && !info.host.empty();
}

} // namespace

SessionHistory::SessionHistory(std::size_t maxEntries)
    : m_maxEntries(maxEntries == 0 ? 1 : maxEntries)
{
}

void SessionHistory::recordVisit(std::string_view url, std::string_view title, std::int64_t nowMs)
{
    const pb::net::UrlInfo info = pb::net::UrlInfo::parse(url);
    if (!isRecordable(info))
        return;

    const std::string key(url);
    const auto existing = std::find_if(m_entries.begin(), m_entries.end(),
                                       [&](const HistoryEntry &e) { return e.url == key; });
    if (existing != m_entries.end()) {
        existing->lastVisitMs = nowMs;
        existing->visitCount += 1;
        if (!title.empty())
            existing->title = std::string(title);
        HistoryEntry moved = *existing;
        m_entries.erase(existing);
        m_entries.push_back(std::move(moved));
        return;
    }

    HistoryEntry entry;
    entry.url = key;
    entry.title = std::string(title);
    entry.lastVisitMs = nowMs;
    entry.visitCount = 1;
    m_entries.push_back(std::move(entry));

    while (m_entries.size() > m_maxEntries)
        m_entries.erase(m_entries.begin());
}

void SessionHistory::updateTitle(std::string_view url, std::string_view title)
{
    if (title.empty())
        return;
    const std::string key(url);
    for (auto &entry : m_entries) {
        if (entry.url == key) {
            entry.title = std::string(title);
            return;
        }
    }
}

std::vector<HistoryEntry> SessionHistory::entries() const
{
    std::vector<HistoryEntry> result(m_entries.rbegin(), m_entries.rend());
    return result;
}

std::vector<HistoryEntry> SessionHistory::suggestions(std::string_view query,
                                                      std::size_t limit) const
{
    std::vector<HistoryEntry> result;
    const std::string needle = pb::text::toLower(pb::text::trim(query));
    if (needle.empty() || limit == 0)
        return result;

    struct Scored {
        const HistoryEntry *entry;
        int score;
    };
    std::vector<Scored> scored;
    scored.reserve(m_entries.size());
    for (const HistoryEntry &entry : m_entries) {
        const int score = matchScore(entry, needle);
        if (score > 0)
            scored.push_back({ &entry, score });
    }

    std::stable_sort(scored.begin(), scored.end(), [](const Scored &a, const Scored &b) {
        if (a.score != b.score)
            return a.score > b.score;
        if (a.entry->visitCount != b.entry->visitCount)
            return a.entry->visitCount > b.entry->visitCount;
        return a.entry->lastVisitMs > b.entry->lastVisitMs;
    });

    for (const Scored &item : scored) {
        if (result.size() >= limit)
            break;
        result.push_back(*item.entry);
    }
    return result;
}

void SessionHistory::clear()
{
    m_entries.clear();
    m_entries.shrink_to_fit();
}

} // namespace pb::browser

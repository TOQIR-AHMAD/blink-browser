// History for the current session only (PLAN.md §18).
//
// It lives in this process and nowhere else: no file, no database, no remote
// store, and it is destroyed when the browser exits. It exists so that the
// address bar can complete what the user typed *from what the user already
// visited*, which is the privacy-preserving alternative to sending every
// keystroke to a suggestion service (PLAN.md §10).

#ifndef PB_BROWSER_CORE_SESSION_HISTORY_H
#define PB_BROWSER_CORE_SESSION_HISTORY_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace pb::browser {

struct HistoryEntry {
    std::string url;
    std::string title;
    std::int64_t lastVisitMs = 0;
    unsigned visitCount = 0;
};

class SessionHistory
{
public:
    explicit SessionHistory(std::size_t maxEntries = 500);

    // Private-window navigations are never passed here by the callers; see
    // WindowController.
    void recordVisit(std::string_view url, std::string_view title, std::int64_t nowMs);
    void updateTitle(std::string_view url, std::string_view title);

    // Most recently visited first.
    std::vector<HistoryEntry> entries() const;

    // Ranked completions for what the user has typed: prefix matches on the
    // host first, then any substring match, ordered by visit count and
    // recency. An empty query returns nothing rather than the whole history.
    std::vector<HistoryEntry> suggestions(std::string_view query, std::size_t limit) const;

    void clear();
    std::size_t size() const { return m_entries.size(); }
    std::size_t maxEntries() const { return m_maxEntries; }

private:
    std::vector<HistoryEntry> m_entries; // newest last
    std::size_t m_maxEntries;
};

} // namespace pb::browser

#endif // PB_BROWSER_CORE_SESSION_HISTORY_H

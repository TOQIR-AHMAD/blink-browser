#include "browser/core/session_history.h"
#include "check.h"

using pb::browser::HistoryEntry;
using pb::browser::SessionHistory;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

void recordsAndRanks()
{
    SessionHistory history;
    history.recordVisit("https://example.com/", "Example", 1000);
    history.recordVisit("https://example.com/docs", "Example docs", 2000);
    history.recordVisit("https://elsewhere.org/example", "Elsewhere", 3000);
    history.recordVisit("https://example.com/", "Example", 4000); // second visit

    checkEqual(static_cast<long long>(history.size()), 3, "repeat visits do not duplicate");

    const auto recent = history.entries();
    checkEqual(recent.front().url, "https://example.com/", "most recent first");
    checkEqual(static_cast<long long>(recent.front().visitCount), 2, "visit count increments");

    const auto matches = history.suggestions("exam", 5);
    checkTrue(matches.size() >= 2, "prefix matches found");
    checkEqual(matches.front().url, "https://example.com/",
               "the most visited host prefix match ranks first");

    const auto hostOnly = history.suggestions("elsewhere", 5);
    checkEqual(static_cast<long long>(hostOnly.size()), 1, "host match");
    checkEqual(hostOnly.front().url, "https://elsewhere.org/example", "host match content");

    checkEqual(static_cast<long long>(history.suggestions("", 5).size()), 0,
               "an empty query suggests nothing");
    checkEqual(static_cast<long long>(history.suggestions("exam", 0).size()), 0, "limit honoured");
    checkEqual(static_cast<long long>(history.suggestions("nothing-here", 5).size()), 0,
               "no false matches");
}

void ignoresWhatMustNotBeRemembered()
{
    SessionHistory history;
    history.recordVisit("about:blank", "New tab", 1);
    history.recordVisit("file:///C:/Users/me/secret.txt", "secret", 2);
    history.recordVisit("data:text/html,<b>x</b>", "inline", 3);
    history.recordVisit("view-source:https://example.com", "source", 4);
    checkEqual(static_cast<long long>(history.size()), 0,
               "internal, local-file and inline documents are not history");
}

void isBoundedAndClearable()
{
    SessionHistory history(3);
    history.recordVisit("https://a.example/", "a", 1);
    history.recordVisit("https://b.example/", "b", 2);
    history.recordVisit("https://c.example/", "c", 3);
    history.recordVisit("https://d.example/", "d", 4);

    checkEqual(static_cast<long long>(history.size()), 3, "capped at the maximum");
    checkEqual(static_cast<long long>(history.suggestions("a.example", 5).size()), 0,
               "the oldest entry was dropped");

    history.clear();
    checkEqual(static_cast<long long>(history.size()), 0, "clear empties the history");
    checkEqual(static_cast<long long>(history.entries().size()), 0, "nothing survives clear");
}

void titlesCanArriveLate()
{
    SessionHistory history;
    history.recordVisit("https://example.com/", "", 1);
    history.updateTitle("https://example.com/", "Example");
    checkEqual(history.entries().front().title, "Example", "title filled in after load");

    history.updateTitle("https://example.com/", "");
    checkEqual(history.entries().front().title, "Example", "an empty title does not erase one");
}

} // namespace

int main()
{
    recordsAndRanks();
    ignoresWhatMustNotBeRemembered();
    isBoundedAndClearable();
    titlesCanArriveLate();
    return pbtest::finish();
}

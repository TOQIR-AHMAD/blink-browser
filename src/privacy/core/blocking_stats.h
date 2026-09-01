// RAM-only counters behind the privacy dashboard (PLAN.md §33).
//
// Counters only: no URL, no host, no per-site breakdown, nothing that could
// reconstruct where the user has been. They are never written to disk and never
// leave the process.

#ifndef PB_PRIVACY_CORE_BLOCKING_STATS_H
#define PB_PRIVACY_CORE_BLOCKING_STATS_H

#include <atomic>
#include <cstdint>

namespace pb::privacy {

struct StatsSnapshot {
    std::uint64_t trackersBlocked = 0;
    std::uint64_t adsBlocked = 0;
    std::uint64_t thirdPartyRequestsBlocked = 0;
    std::uint64_t cookiesBlocked = 0;
    std::uint64_t permissionsGranted = 0;
    std::uint64_t permissionsDenied = 0;
    std::uint64_t httpsUpgrades = 0;

    std::uint64_t total() const
    {
        return trackersBlocked + adsBlocked + thirdPartyRequestsBlocked + cookiesBlocked;
    }
};

// The counters are incremented from the Chromium network thread and read from
// the UI thread, hence the atomics.
class BlockingStats
{
public:
    void recordTrackerBlocked();
    void recordAdBlocked();
    void recordThirdPartyBlocked();
    void recordCookieBlocked();
    void recordPermissionGranted();
    void recordPermissionDenied();
    void recordHttpsUpgrade();

    StatsSnapshot snapshot() const;
    void reset();

private:
    std::atomic<std::uint64_t> m_trackers{0};
    std::atomic<std::uint64_t> m_ads{0};
    std::atomic<std::uint64_t> m_thirdParty{0};
    std::atomic<std::uint64_t> m_cookies{0};
    std::atomic<std::uint64_t> m_permissionsGranted{0};
    std::atomic<std::uint64_t> m_permissionsDenied{0};
    std::atomic<std::uint64_t> m_httpsUpgrades{0};
};

} // namespace pb::privacy

#endif // PB_PRIVACY_CORE_BLOCKING_STATS_H

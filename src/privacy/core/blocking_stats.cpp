#include "privacy/core/blocking_stats.h"

namespace pb::privacy {
namespace {
constexpr auto kOrder = std::memory_order_relaxed;
}

void BlockingStats::recordTrackerBlocked()
{
    m_trackers.fetch_add(1, kOrder);
}

void BlockingStats::recordAdBlocked()
{
    m_ads.fetch_add(1, kOrder);
}

void BlockingStats::recordThirdPartyBlocked()
{
    m_thirdParty.fetch_add(1, kOrder);
}

void BlockingStats::recordCookieBlocked()
{
    m_cookies.fetch_add(1, kOrder);
}

void BlockingStats::recordPermissionGranted()
{
    m_permissionsGranted.fetch_add(1, kOrder);
}

void BlockingStats::recordPermissionDenied()
{
    m_permissionsDenied.fetch_add(1, kOrder);
}

void BlockingStats::recordHttpsUpgrade()
{
    m_httpsUpgrades.fetch_add(1, kOrder);
}

StatsSnapshot BlockingStats::snapshot() const
{
    StatsSnapshot snapshot;
    snapshot.trackersBlocked = m_trackers.load(kOrder);
    snapshot.adsBlocked = m_ads.load(kOrder);
    snapshot.thirdPartyRequestsBlocked = m_thirdParty.load(kOrder);
    snapshot.cookiesBlocked = m_cookies.load(kOrder);
    snapshot.permissionsGranted = m_permissionsGranted.load(kOrder);
    snapshot.permissionsDenied = m_permissionsDenied.load(kOrder);
    snapshot.httpsUpgrades = m_httpsUpgrades.load(kOrder);
    return snapshot;
}

void BlockingStats::reset()
{
    m_trackers.store(0, kOrder);
    m_ads.store(0, kOrder);
    m_thirdParty.store(0, kOrder);
    m_cookies.store(0, kOrder);
    m_permissionsGranted.store(0, kOrder);
    m_permissionsDenied.store(0, kOrder);
    m_httpsUpgrades.store(0, kOrder);
}

} // namespace pb::privacy

#include "check.h"
#include "privacy/core/blocking_stats.h"

#include <thread>
#include <vector>

using pb::privacy::BlockingStats;
using pb::privacy::StatsSnapshot;
using pbtest::checkEqual;

int main()
{
    BlockingStats stats;

    stats.recordTrackerBlocked();
    stats.recordTrackerBlocked();
    stats.recordAdBlocked();
    stats.recordThirdPartyBlocked();
    stats.recordCookieBlocked();
    stats.recordPermissionGranted();
    stats.recordPermissionDenied();
    stats.recordHttpsUpgrade();

    StatsSnapshot snapshot = stats.snapshot();
    checkEqual(static_cast<long long>(snapshot.trackersBlocked), 2, "trackers");
    checkEqual(static_cast<long long>(snapshot.adsBlocked), 1, "ads");
    checkEqual(static_cast<long long>(snapshot.thirdPartyRequestsBlocked), 1, "third party");
    checkEqual(static_cast<long long>(snapshot.cookiesBlocked), 1, "cookies");
    checkEqual(static_cast<long long>(snapshot.permissionsGranted), 1, "permissions granted");
    checkEqual(static_cast<long long>(snapshot.permissionsDenied), 1, "permissions denied");
    checkEqual(static_cast<long long>(snapshot.httpsUpgrades), 1, "https upgrades");
    checkEqual(static_cast<long long>(snapshot.total()), 5, "dashboard total");

    stats.reset();
    checkEqual(static_cast<long long>(stats.snapshot().total()), 0, "reset clears the counters");

    // The interceptor runs on Chromium's network thread while the dashboard
    // reads from the UI thread.
    constexpr int kThreads = 4;
    constexpr int kIterations = 1000;
    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < kIterations; ++j)
                stats.recordTrackerBlocked();
        });
    }
    for (auto &thread : threads)
        thread.join();
    checkEqual(static_cast<long long>(stats.snapshot().trackersBlocked), kThreads * kIterations,
               "concurrent increments are not lost");

    return pbtest::finish();
}

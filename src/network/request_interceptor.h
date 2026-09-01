// Sits between Chromium and the network for every single request.
//
// Three things happen here, in this order:
//   1. HTTPS-first: a top-level http:// navigation is redirected to https://
//      unless the host is local or already known to have no https endpoint.
//   2. Filtering: the request is matched against the filter engine and
//      blocked outright if it matches. Blocking here means the connection is
//      never made - not hidden after the fact.
//   3. Counting: a blocked request bumps an in-memory counter and nothing
//      else. No URL, no host and no timestamp is stored (PLAN.md §33).
//
// This runs on Chromium's network thread, so it touches only thread-safe
// state: FilterService takes a read lock, BlockingStats is atomic.

#ifndef PB_NETWORK_REQUEST_INTERCEPTOR_H
#define PB_NETWORK_REQUEST_INTERCEPTOR_H

#include "network/core/https_upgrade.h"
#include "network/core/network_audit.h"

#include <QtCore/QMutex>
#include <QtCore/QVariantList>
#include <QtWebEngineCore/QWebEngineUrlRequestInterceptor>

namespace pb::privacy {
class BlockingStats;
}

namespace pb::network {

class FilterService;

class RequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    RequestInterceptor(FilterService *filters, pb::privacy::BlockingStats *stats,
                       QObject *parent = nullptr);

    void interceptRequest(QWebEngineUrlRequestInfo &info) override;

    void setBlockingEnabled(bool trackers, bool ads);
    void setHttpsFirstEnabled(bool enabled);

    // Called from the UI thread when an https attempt failed, so the host is
    // not upgraded again this session.
    void recordHttpsFailure(const QString &host);

    // Development-only audit (PLAN.md §43). Off unless the build defines
    // PB_NETWORK_AUDIT and the user turns it on.
    void setAuditEnabled(bool enabled);
    bool auditEnabled() const;
    QVariantList auditRows() const;
    void clearAudit();

Q_SIGNALS:
    // Carries only the category, never the request.
    void requestBlocked(int category);

private:
    FilterService *m_filters = nullptr;
    pb::privacy::BlockingStats *m_stats = nullptr;

    mutable QMutex m_mutex; // guards the upgrade tracker and the audit
    pb::net::HttpsUpgrade m_httpsUpgrade;
    pb::net::NetworkAudit m_audit;

    bool m_trackersEnabled = true;
    bool m_adsEnabled = true;
    bool m_httpsFirstEnabled = true;
    bool m_auditEnabled = false;
};

} // namespace pb::network

#endif // PB_NETWORK_REQUEST_INTERCEPTOR_H

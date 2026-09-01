// Owns the one directory the browser is allowed to write to while running.
//
// Chromium writes some things to disk no matter how the profile is configured:
// the GPU shader cache, a crash database, temporary files for downloads and the
// PDF viewer. Rather than pretend otherwise (PLAN.md §13), the browser points
// all of it at a directory created under the system temp directory at start-up
// and removes that directory on exit (PLAN.md §14).
//
// The directory name contains a random suffix, regenerated every launch. It is
// deliberately not derived from the user, the machine or anything stable, so it
// cannot act as an installation identifier (PLAN.md §49).

#ifndef PB_PRIVACY_CORE_SESSION_PATHS_H
#define PB_PRIVACY_CORE_SESSION_PATHS_H

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

namespace pb::privacy {

struct CleanupResult {
    bool completed = false;
    // Number of entries that could not be removed. Never a file name: a
    // leftover path can reveal what was downloaded (PLAN.md §31).
    std::size_t remaining = 0;
    // Technical error text (an error code or category), never a path.
    std::string error;
};

class SessionPaths
{
public:
    SessionPaths() = default;
    explicit SessionPaths(std::filesystem::path root);
    ~SessionPaths();

    SessionPaths(const SessionPaths &) = delete;
    SessionPaths &operator=(const SessionPaths &) = delete;
    SessionPaths(SessionPaths &&other) noexcept;
    SessionPaths &operator=(SessionPaths &&other) noexcept;

    // Creates <temp>/<prefix><random> and returns it. Throws
    // std::filesystem::filesystem_error if the directory cannot be created;
    // the caller must treat that as fatal rather than falling back to a
    // persistent location.
    static SessionPaths createTemporary(const std::filesystem::path &baseTempDir,
                                        std::string_view prefix = "privacy-browser-");

    const std::filesystem::path &root() const { return m_root; }
    bool isValid() const { return !m_root.empty(); }
    bool exists() const;

    // Creates the named subdirectory if needed and returns its path.
    std::filesystem::path subdirectory(std::string_view name) const;

    // Removes the whole directory tree. Idempotent: calling it on an already
    // cleaned (or never created) session reports completed = true.
    CleanupResult cleanup();

private:
    std::filesystem::path m_root;
};

// Generates a random directory name component. Exposed for tests.
std::string randomSessionSuffix();

} // namespace pb::privacy

#endif // PB_PRIVACY_CORE_SESSION_PATHS_H

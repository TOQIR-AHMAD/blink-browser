#include "privacy/core/session_paths.h"

#include "utils/logging.h"

#include <random>
#include <system_error>
#include <utility>

namespace pb::privacy {
namespace {

constexpr char kHexDigits[] = "0123456789abcdef";

std::size_t countEntries(const std::filesystem::path &path)
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec))
        return 0;

    std::size_t count = 0;
    for (auto it = std::filesystem::recursive_directory_iterator(
             path, std::filesystem::directory_options::skip_permission_denied, ec);
         it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
        if (ec)
            break;
        ++count;
    }
    return count;
}

} // namespace

std::string randomSessionSuffix()
{
    std::random_device device;
    std::uniform_int_distribution<int> distribution(0, 15);
    std::string suffix;
    suffix.reserve(16);
    for (int i = 0; i < 16; ++i)
        suffix.push_back(kHexDigits[distribution(device)]);
    return suffix;
}

SessionPaths::SessionPaths(std::filesystem::path root)
    : m_root(std::move(root))
{
}

SessionPaths::SessionPaths(SessionPaths &&other) noexcept
    : m_root(std::move(other.m_root))
{
    other.m_root.clear();
}

SessionPaths &SessionPaths::operator=(SessionPaths &&other) noexcept
{
    if (this != &other) {
        cleanup();
        m_root = std::move(other.m_root);
        other.m_root.clear();
    }
    return *this;
}

SessionPaths::~SessionPaths()
{
    cleanup();
}

SessionPaths SessionPaths::createTemporary(const std::filesystem::path &baseTempDir,
                                           std::string_view prefix)
{
    std::filesystem::create_directories(baseTempDir);

    for (int attempt = 0; attempt < 8; ++attempt) {
        const std::filesystem::path candidate
            = baseTempDir / (std::string(prefix) + randomSessionSuffix());
        std::error_code ec;
        if (std::filesystem::create_directory(candidate, ec))
            return SessionPaths(candidate);
        if (ec && ec != std::errc::file_exists) {
            throw std::filesystem::filesystem_error("cannot create session directory", ec);
        }
    }
    throw std::filesystem::filesystem_error(
        "cannot create session directory",
        std::make_error_code(std::errc::file_exists));
}

bool SessionPaths::exists() const
{
    if (m_root.empty())
        return false;
    std::error_code ec;
    return std::filesystem::exists(m_root, ec);
}

std::filesystem::path SessionPaths::subdirectory(std::string_view name) const
{
    const std::filesystem::path path = m_root / std::string(name);
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    return path;
}

CleanupResult SessionPaths::cleanup()
{
    CleanupResult result;
    if (m_root.empty()) {
        result.completed = true;
        return result;
    }

    std::error_code ec;
    if (!std::filesystem::exists(m_root, ec)) {
        result.completed = true;
        m_root.clear();
        return result;
    }

    std::filesystem::remove_all(m_root, ec);
    if (ec) {
        result.error = ec.message();
        result.remaining = countEntries(m_root);
        result.completed = false;
        // Technical detail only: no path, no file name.
        pb::log::write(pb::log::Level::Error,
                       "session directory cleanup failed: " + result.error);
        return result;
    }

    result.completed = true;
    m_root.clear();
    return result;
}

} // namespace pb::privacy

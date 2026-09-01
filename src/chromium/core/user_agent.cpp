#include "chromium/core/user_agent.h"

namespace pb::chromium {
namespace {

constexpr std::string_view kChromeToken = "Chrome/";

std::string_view platformToken(UserAgentPlatform platform)
{
    switch (platform) {
    case UserAgentPlatform::Windows:
        return "Windows NT 10.0; Win64; x64";
    case UserAgentPlatform::MacOs:
        return "Macintosh; Intel Mac OS X 10_15_7";
    case UserAgentPlatform::Linux:
        return "X11; Linux x86_64";
    }
    return "Windows NT 10.0; Win64; x64";
}

} // namespace

std::string chromiumMajorVersion(std::string_view defaultUserAgent)
{
    const std::size_t at = defaultUserAgent.find(kChromeToken);
    if (at == std::string_view::npos)
        return {};

    std::string version;
    for (std::size_t i = at + kChromeToken.size(); i < defaultUserAgent.size(); ++i) {
        const char c = defaultUserAgent[i];
        if (c < '0' || c > '9')
            break;
        version.push_back(c);
    }
    return version;
}

std::string reducedUserAgent(std::string_view defaultUserAgent, UserAgentPlatform platform)
{
    const std::string major = chromiumMajorVersion(defaultUserAgent);
    if (major.empty())
        return {};

    std::string agent = "Mozilla/5.0 (";
    agent += platformToken(platform);
    agent += ") AppleWebKit/537.36 (KHTML, like Gecko) Chrome/";
    agent += major;
    agent += ".0.0.0 Safari/537.36";
    return agent;
}

} // namespace pb::chromium

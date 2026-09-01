#include "check.h"
#include "chromium/core/user_agent.h"

using namespace pb::chromium;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

constexpr char kQtDefault[] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                              "(KHTML, like Gecko) QtWebEngine/6.8.1 Chrome/122.0.6261.171 "
                              "Safari/537.36";

} // namespace

int main()
{
    checkEqual(chromiumMajorVersion(kQtDefault), "122", "major version extracted");
    checkEqual(chromiumMajorVersion("no chrome token here"), "", "missing token");
    checkEqual(chromiumMajorVersion("Chrome/"), "", "token without digits");

    const std::string windows = reducedUserAgent(kQtDefault, UserAgentPlatform::Windows);
    checkEqual(windows,
               "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
               "Chrome/122.0.0.0 Safari/537.36",
               "reduced Windows user agent");
    checkTrue(windows.find("QtWebEngine") == std::string::npos,
              "the QtWebEngine token is what makes the default rare, and must be gone");
    checkTrue(windows.find("6261") == std::string::npos,
              "the build number is entropy and must be gone");

    checkTrue(reducedUserAgent(kQtDefault, UserAgentPlatform::MacOs).find("Mac OS X")
                  != std::string::npos,
              "macOS platform token");
    checkTrue(reducedUserAgent(kQtDefault, UserAgentPlatform::Linux).find("X11; Linux")
                  != std::string::npos,
              "Linux platform token");

    checkEqual(reducedUserAgent("something unrecognisable", UserAgentPlatform::Windows), "",
               "no invented version when the default cannot be parsed");

    return pbtest::finish();
}

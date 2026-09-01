// Session cleanup is the promise in PLAN.md §14, and it must be idempotent.

#include "check.h"
#include "privacy/core/session_paths.h"

#include <fstream>

namespace fs = std::filesystem;
using pb::privacy::CleanupResult;
using pb::privacy::randomSessionSuffix;
using pb::privacy::SessionPaths;
using pbtest::checkEqual;
using pbtest::checkTrue;

namespace {

void writeFile(const fs::path &path, const char *content)
{
    std::ofstream out(path);
    out << content;
}

fs::path testBase()
{
    return fs::temp_directory_path() / "pb-session-tests";
}

void createsAndRemovesTheTree()
{
    fs::path root;
    {
        SessionPaths session = SessionPaths::createTemporary(testBase(), "unit-test-");
        root = session.root();

        checkTrue(session.isValid(), "session is valid after creation");
        checkTrue(fs::exists(root), "session directory exists");
        checkTrue(root.filename().string().rfind("unit-test-", 0) == 0, "prefix used");

        const fs::path cache = session.subdirectory("cache");
        writeFile(cache / "entry.bin", "cached bytes");
        writeFile(session.subdirectory("downloads") / "temp.part", "partial download");
        checkTrue(fs::exists(cache / "entry.bin"), "file written inside the session");

        const CleanupResult result = session.cleanup();
        checkTrue(result.completed, "cleanup completes");
        checkEqual(static_cast<long long>(result.remaining), 0, "nothing left behind");
        checkTrue(!fs::exists(root), "session directory removed");

        const CleanupResult again = session.cleanup();
        checkTrue(again.completed, "cleanup is idempotent");
        checkTrue(!session.isValid(), "session invalid after cleanup");
    }
    checkTrue(!fs::exists(root), "destructor did not resurrect anything");
}

void destructorCleansUp()
{
    fs::path root;
    {
        SessionPaths session = SessionPaths::createTemporary(testBase(), "unit-test-");
        root = session.root();
        writeFile(session.subdirectory("storage") / "file.txt", "data");
    }
    checkTrue(!fs::exists(root), "destructor removes the session directory");
}

void moveTransfersOwnership()
{
    fs::path root;
    {
        SessionPaths first = SessionPaths::createTemporary(testBase(), "unit-test-");
        root = first.root();
        SessionPaths second = std::move(first);
        checkTrue(!first.isValid(), "moved-from session owns nothing");
        checkTrue(second.root() == root, "moved-to session owns the directory");
        checkTrue(fs::exists(root), "move did not delete the directory");
    }
    checkTrue(!fs::exists(root), "the surviving owner cleaned up");
}

void suffixIsRandomAndNotAnIdentifier()
{
    // PLAN.md §49: nothing stable per installation.
    const std::string first = randomSessionSuffix();
    const std::string second = randomSessionSuffix();
    checkEqual(static_cast<long long>(first.size()), 16, "suffix length");
    checkTrue(first != second, "suffix differs between sessions");
}

void invalidSessionIsHarmless()
{
    SessionPaths empty;
    checkTrue(!empty.isValid(), "default session is invalid");
    checkTrue(!empty.exists(), "default session does not exist");
    checkTrue(empty.cleanup().completed, "cleaning an empty session succeeds");
}

} // namespace

int main()
{
    createsAndRemovesTheTree();
    destructorCleansUp();
    moveTransfersOwnership();
    suffixIsRandomAndNotAnIdentifier();
    invalidSessionIsHarmless();

    std::error_code ec;
    fs::remove_all(testBase(), ec);
    return pbtest::finish();
}

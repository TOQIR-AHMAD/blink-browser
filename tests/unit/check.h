// Minimal check helpers for the core unit tests.
//
// The core is deliberately free of Qt, so its tests are plain executables run
// by CTest and there is no test-framework dependency to justify (PLAN.md §45).

#ifndef PB_TESTS_CHECK_H
#define PB_TESTS_CHECK_H

#include <cstdio>
#include <string>

namespace pbtest {

inline int failureCount = 0;

inline void checkTrue(bool condition, const char *what)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL %s\n", what);
        ++failureCount;
    }
}

inline void checkEqual(const std::string &actual, const std::string &expected, const char *what)
{
    if (actual != expected) {
        std::fprintf(stderr, "FAIL %s\n  expected: %s\n  actual:   %s\n", what, expected.c_str(),
                     actual.c_str());
        ++failureCount;
    }
}

inline void checkEqual(long long actual, long long expected, const char *what)
{
    if (actual != expected) {
        std::fprintf(stderr, "FAIL %s\n  expected: %lld\n  actual:   %lld\n", what, expected,
                     actual);
        ++failureCount;
    }
}

inline int finish()
{
    if (failureCount > 0) {
        std::fprintf(stderr, "%d check(s) failed\n", failureCount);
        return 1;
    }
    std::fprintf(stderr, "all checks passed\n");
    return 0;
}

} // namespace pbtest

#endif // PB_TESTS_CHECK_H

#include "check.h"
#include "privacy/core/cleanup_sequence.h"

using namespace pb::privacy;
using pbtest::checkEqual;
using pbtest::checkTrue;

int main()
{
    int firstRuns = 0;
    int secondRuns = 0;
    int failingRuns = 0;

    CleanupSequence sequence;
    sequence.addStep("close tabs", [&](std::string *) {
        ++firstRuns;
        return true;
    });
    sequence.addStep("destroy profile", [&](std::string *detail) {
        ++failingRuns;
        // Fails the first time, succeeds when retried.
        if (failingRuns == 1) {
            *detail = "profile still in use";
            return false;
        }
        return true;
    });
    sequence.addStep("delete session directory", [&](std::string *) {
        ++secondRuns;
        return true;
    });

    checkEqual(static_cast<long long>(sequence.stepCount()), 3, "three steps registered");
    checkTrue(!sequence.hasRun(), "nothing has run yet");

    const CleanupOutcome first = sequence.run();
    checkTrue(sequence.hasRun(), "the sequence ran");
    checkEqual(static_cast<long long>(first.steps.size()), 3, "every step reported");
    checkTrue(!first.completed(), "a failing step makes the run incomplete");
    checkEqual(static_cast<long long>(first.failedCount()), 1, "one failure");
    checkEqual(first.steps[1].detail, "profile still in use", "failure detail passed through");
    checkEqual(static_cast<long long>(secondRuns), 1,
               "a failure does not stop the steps after it");

    const CleanupOutcome second = sequence.run();
    checkTrue(second.completed(), "the retry completes the sequence");
    checkEqual(static_cast<long long>(firstRuns), 1, "a step that succeeded is not run again");
    checkEqual(static_cast<long long>(secondRuns), 1, "nor is the one after it");
    checkEqual(static_cast<long long>(failingRuns), 2, "the failed step was retried");

    const CleanupOutcome third = sequence.run();
    checkTrue(third.completed(), "running again is harmless");
    checkEqual(static_cast<long long>(failingRuns), 2, "and does nothing");

    CleanupSequence empty;
    checkTrue(empty.run().completed(), "an empty sequence completes");

    empty.addStep("ignored", CleanupSequence::Step());
    checkEqual(static_cast<long long>(empty.stepCount()), 0, "a null step is not registered");

    return pbtest::finish();
}

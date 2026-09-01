// The shutdown sequence from PLAN.md §14, as data rather than as a function
// body.
//
// Each step is named, runs at most once per sequence, and reports success or
// failure without stopping the ones after it: a profile that refuses to close
// must not prevent the temporary directory from being deleted. Running the
// whole sequence twice is safe, which is the "cleanup must be idempotent"
// requirement expressed in code.

#ifndef PB_PRIVACY_CORE_CLEANUP_SEQUENCE_H
#define PB_PRIVACY_CORE_CLEANUP_SEQUENCE_H

#include <functional>
#include <string>
#include <vector>

namespace pb::privacy {

struct CleanupStepResult {
    std::string name;
    bool succeeded = false;
    // Technical detail only - never a path, URL or file name (PLAN.md §31).
    std::string detail;
};

struct CleanupOutcome {
    std::vector<CleanupStepResult> steps;
    bool completed() const;
    std::size_t failedCount() const;
};

class CleanupSequence
{
public:
    // The step returns true on success. It must be safe to call even if the
    // thing it cleans was never created.
    using Step = std::function<bool(std::string *detail)>;

    void addStep(std::string name, Step step);

    // Runs every step that has not already succeeded. Steps that failed
    // before are retried.
    CleanupOutcome run();

    bool hasRun() const { return m_hasRun; }
    std::size_t stepCount() const { return m_steps.size(); }

private:
    struct Entry {
        std::string name;
        Step step;
        bool succeeded = false;
    };

    std::vector<Entry> m_steps;
    bool m_hasRun = false;
};

} // namespace pb::privacy

#endif // PB_PRIVACY_CORE_CLEANUP_SEQUENCE_H

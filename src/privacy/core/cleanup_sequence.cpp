#include "privacy/core/cleanup_sequence.h"

#include <algorithm>

namespace pb::privacy {

bool CleanupOutcome::completed() const
{
    return std::all_of(steps.begin(), steps.end(),
                       [](const CleanupStepResult &step) { return step.succeeded; });
}

std::size_t CleanupOutcome::failedCount() const
{
    return static_cast<std::size_t>(
        std::count_if(steps.begin(), steps.end(),
                      [](const CleanupStepResult &step) { return !step.succeeded; }));
}

void CleanupSequence::addStep(std::string name, Step step)
{
    if (!step)
        return;
    m_steps.push_back({ std::move(name), std::move(step), false });
}

CleanupOutcome CleanupSequence::run()
{
    CleanupOutcome outcome;
    outcome.steps.reserve(m_steps.size());

    for (Entry &entry : m_steps) {
        CleanupStepResult result;
        result.name = entry.name;

        if (entry.succeeded) {
            // Already done on an earlier run: report success without acting
            // again, which is what makes a second run harmless.
            result.succeeded = true;
            result.detail = "already done";
            outcome.steps.push_back(std::move(result));
            continue;
        }

        std::string detail;
        result.succeeded = entry.step(&detail);
        result.detail = std::move(detail);
        entry.succeeded = result.succeeded;
        outcome.steps.push_back(std::move(result));
    }

    m_hasRun = true;
    return outcome;
}

} // namespace pb::privacy

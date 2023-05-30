#include "polisher.hpp"

#include "error_detector.hpp"
#include "indel_corrector.hpp"
#include "mismatch_corrector.hpp"
#include "pattern_detector.hpp"

namespace aiedit {

PolishingResults Polisher::polish(SequenceIterator& seq_iter)
{
    PolishingResults results;
    ErrorDetector err_detector(seq_iter, bf);
    PatternDetector pattern_detector(pattern_length, bf, model);
    MismatchCorrector mismatch_corrector(bf);
    IndelCorrector indel_corrector(bf);
    while (err_detector.next()) {
        const auto pattern = pattern_detector.get_pattern(seq_iter);
        const auto mismatches = mismatch_corrector.fix(seq_iter, pattern);
        const auto indels = indel_corrector.fix(seq_iter, pattern);
        update_results(mismatches, indels, seq_iter.get_position(), results);
    }
    return results;
}

void Polisher::update_results(const std::vector<Edit>& mismatches,
                              const std::vector<Edit>& indels,
                              unsigned seq_iter_position,
                              PolishingResults& results)
{
    if (mismatches.empty() && indels.empty()) {
        results.ignored_positions.emplace_back(seq_iter_position);
        return;
    } else {
        ++results.num_fixed_patterns;
    }
    for (const auto& edit : mismatches) {
        ++results.num_mismatches;
        results.edits.emplace_back(edit);
    }
    for (const auto& edit : indels) {
        if (edit.type == Edit::INSERTION) {
            ++results.num_insertions;
        } else {
            ++results.num_deletions;
        }
        results.edits.emplace_back(edit);
    }
}

}  // namespace aiedit
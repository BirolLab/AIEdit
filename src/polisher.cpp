#include "polisher.hpp"

#include "error_detector.hpp"
#include "mismatch_corrector.hpp"
#include "pattern_detector.hpp"

namespace aiedit {

PolishingResults Polisher::polish(SequenceIterator& seq_iter)
{
    PolishingResults results;
    ErrorDetector err_detector(seq_iter, bf);
    PatternDetector pattern_detector(pattern_length, bf, model);
    MismatchCorrector err_corrector(bf);
    while (err_detector.next()) {
        const auto pattern = pattern_detector.get_pattern(seq_iter);
        const auto edits = err_corrector.get_fixes(seq_iter, pattern);
        const bool fixed = !edits.empty();
        if (fixed) {
            ++results.num_fixed_patterns;
        } else {
            for (unsigned i = 0; i < pattern.get_length(); i++) {
                if (pattern.get(i) != Edit::NONE) {
                    results.ignored_positions.emplace_back(seq_iter.get_position() + i);
                }
            }
        }
        seq_iter.previous();
        apply_edits(seq_iter, edits, results);
        seq_iter.next(fixed ? pattern_length : bf.get_k() + pattern_length);
    }
    return results;
}

void Polisher::apply_edits(SequenceIterator& seq_iter,
                           const std::vector<Edit>& edits,
                           PolishingResults& results)
{
    for (const auto& edit : edits) {
        seq_iter.update(edit.position, edit.after);
        results.edits.emplace_back(edit);
        ++results.num_mismatches;
    }
}

}  // namespace aiedit
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
        auto pattern = pattern_detector.get_pattern(seq_iter);
        auto num_detected_mismatches = pattern.get_count(Edit::Type::MISMATCH);
        auto num_detected_insertions = pattern.get_count(Edit::Type::INSERTION);
        auto num_detected_deletions = pattern.get_count(Edit::Type::DELETION);
        std::vector<Edit> fixed_mismatches;
        std::vector<Edit> fixed_indels;
        if (num_detected_mismatches > 0) {
            fixed_mismatches = mismatch_corrector.fix(seq_iter, pattern);
        }
        if (fixed_mismatches.empty() && num_detected_insertions + num_detected_deletions > 0) {
            fixed_indels = indel_corrector.fix(seq_iter, pattern);
        }
        std::cout << seq_iter.get_position() << " " << pattern.to_string() << " "
                  << fixed_mismatches.size() << " " << fixed_indels.size() << std::endl;
        update_results(fixed_mismatches, fixed_indels, seq_iter.get_position(), results);
        if (fixed_mismatches.empty()) {
            seq_iter.next(pattern.get_length() + seq_iter.get_seed_length());
        } else {
            seq_iter.next(pattern.get_length());
        }
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
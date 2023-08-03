#include "polisher.hpp"

#include <algorithm>

#include "error_corrector.hpp"
#include "error_detector.hpp"
#include "pattern_detector.hpp"

namespace aiedit {

void PolishingResults::add_edits(const std::vector<Edit>& edits)
{
    for (const auto& edit : edits) {
        this->edits.emplace_back(edit);
        if (edit.get_type() == Edit::Type::MISMATCH) {
            ++num_mismatches;
        } else if (edit.get_type() == Edit::Type::INSERTION) {
            ++num_insertions;
        } else if (edit.get_type() == Edit::Type::DELETION) {
            ++num_deletions;
        }
    }
    ++num_fixed_patterns;
}

void PolishingResults::add_ignored_pattern(unsigned position, const std::string& pattern)
{
    ignored.emplace_back(position, pattern);
}

void PolishingResults::merge(const PolishingResults& results)
{
    add_edits(results.edits);
    for (const auto& ignored : results.get_ignored_patterns()) {
        add_ignored_pattern(ignored.first, ignored.second);
    }
    num_fixed_patterns += results.num_fixed_patterns;
    num_mismatches += results.num_mismatches;
    num_insertions += results.num_insertions;
    num_deletions += results.num_deletions;
}

void PolishingResults::sort_edits()
{
    auto comp = [](const Edit& e1, const Edit& e2) {
        return e1.get_position() < e2.get_position();
    };
    std::sort(edits.begin(), edits.end(), comp);
}

void PolishingResults::sort_ignored() { std::sort(ignored.begin(), ignored.end()); }

const std::string PolishingResults::apply(const std::string& seq) const
{
    std::string edited;
    edited.reserve(seq.size() + num_insertions - num_deletions);
    unsigned current_edit = 0;
    for (unsigned i = 0; i < seq.size(); i++) {
        if (current_edit >= edits.size()) {
            edited.push_back(seq[i]);
            continue;
        }
        const auto& edit = edits[current_edit];
        if (edit.get_position() != i) {
            edited.push_back(seq[i]);
            continue;
        }
        if (edit.get_type() == Edit::Type::MISMATCH) {
            edited.push_back(edit.get_after());
        } else if (edit.get_type() == Edit::Type::INSERTION) {
            edited.push_back(edit.get_after());
            --i;
        } else if (edit.get_type() == Edit::Type::DELETION) {
            ++i;
        }
        ++current_edit;
    }
    return edited;
}

PolishingResults Polisher::polish(SequenceIterator& seq_iter)
{
    PolishingResults results;
    ErrorDetector error_detector(seq_iter, bf);
    PatternDetector pattern_detector(pattern_length, bf, model);
    ErrorCorrector error_corrector(bf);
    while (error_detector.next()) {
        const auto pattern = pattern_detector.get_pattern(seq_iter);
        const auto edits = error_corrector.fix(seq_iter, pattern);
        if (edits.empty()) {
            results.add_ignored_pattern(seq_iter.get_position(), pattern.to_string());
            seq_iter.next(pattern.get_length() + seq_iter.get_seed_length());
        } else {
            results.add_edits(edits);
        }
    }
    return results;
}

}  // namespace aiedit
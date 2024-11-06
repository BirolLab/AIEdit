#pragma once

#include <algorithm>
#include <btllib/counting_bloom_filter.hpp>
#include <fdeep/fdeep.hpp>
#include <vector>

#include "edit.hpp"
#include "error_corrector.hpp"
#include "error_detector.hpp"
#include "pattern.hpp"
#include "pattern_detector.hpp"
#include "sequence_iterator.hpp"

class PolishingResults
{
  public:

    void add_edits(const std::vector<Edit>& edits)
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

    const std::vector<Edit>& get_edits() const { return edits; }

    void add_ignored_pattern(unsigned position, const std::string& pattern)
    {
        ignored.emplace_back(position, pattern);
    }

    const std::vector<std::pair<unsigned, std::string>>& get_ignored_patterns() const
    {
        return ignored;
    }

    void merge(const PolishingResults& results)
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

    void sort_edits()
    {
        auto comp = [](const Edit& e1, const Edit& e2) {
            return e1.get_position() < e2.get_position();
        };
        std::sort(edits.begin(), edits.end(), comp);
    }

    void sort_ignored() { std::sort(ignored.begin(), ignored.end()); }

    unsigned get_num_fixed_patterns() const { return num_fixed_patterns; }
    unsigned get_num_ignored_patterns() const { return ignored.size(); }
    unsigned get_num_mismatches() const { return num_mismatches; }
    unsigned get_num_insertions() const { return num_insertions; }
    unsigned get_num_deletions() const { return num_deletions; }

    const std::string apply(const std::string& seq) const
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
            if (edit.get_position() == i && edit.get_type() == Edit::Type::MISMATCH) {
                edited.push_back(edit.get_after()[0]);
                ++current_edit;
            } else if (edit.get_position() == i && edit.get_type() == Edit::Type::INSERTION) {
                edited.append(edit.get_after());
                ++current_edit;
            } else if (edit.get_position() + 1 == i && edit.get_type() == Edit::Type::DELETION) {
                ++current_edit;
                i += edit.get_before().size() - 2;
            } else {
                edited.push_back(seq[i]);
            }
        }
        return edited;
    }

  private:

    std::vector<Edit> edits;
    std::vector<std::pair<unsigned, std::string>> ignored;
    unsigned num_fixed_patterns = 0;
    unsigned num_mismatches = 0;
    unsigned num_insertions = 0;
    unsigned num_deletions = 0;
};

class Polisher
{

  public:

    Polisher(const btllib::CountingBloomFilter8& bf, const fdeep::model& model)
      : bf(bf)
      , model(model)
    {}

    PolishingResults polish(SequenceIterator& seq_iter)
    {
        PolishingResults results;
        ErrorDetector error_detector(seq_iter, bf);
        PatternDetector pattern_detector(bf, model);
        ErrorCorrector error_corrector(bf);
        while (error_detector.next()) {
            auto pattern = pattern_detector.get_pattern(seq_iter);
            std::vector<Edit> edits;
            if (!pattern.is_empty()) {
                edits = error_corrector.fix(seq_iter, pattern);
            }
            if (!pattern.is_empty() && edits.empty()) {
                results.add_ignored_pattern(seq_iter.get_position(), pattern.to_string());
            } else if (!pattern.is_empty()) {
                update_seq_iter(seq_iter, edits);
                results.add_edits(edits);
            }
            find_next_hit(seq_iter, bf);
        }
        return results;
    }

  private:

    const btllib::CountingBloomFilter8& bf;
    const fdeep::model& model;

    void update_seq_iter(SequenceIterator& seq_iter, const std::vector<Edit>& edits)
    {
        if (edits[0].get_type() == Edit::Type::INSERTION) {
            for (const auto c : edits[0].get_after()) {
                seq_iter.insert_last(c);
            }
        } else if (edits[0].get_type() == Edit::Type::DELETION) {
            for (unsigned i = 1; i < edits[0].get_before().size(); i++) {
                seq_iter.delete_last();
            }
        } else {
            for (const auto& edit : edits) {
                seq_iter.next(edit.get_position() - seq_iter.get_position());
                if (edit.get_type() == Edit::Type::MISMATCH) {
                    seq_iter.substitute_last(edit.get_after()[0]);
                }
            }
        }
    }

    void find_next_hit(SequenceIterator& seq_iter, const btllib::CountingBloomFilter8& bf)
    {
        bool has_miss = true;
        while (has_miss && seq_iter.next()) {
            has_miss = bf.contains(seq_iter.get_kmer_hashes()) == 0;
        }
    }
};

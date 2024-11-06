#pragma once

#include <bitset>
#include <btllib/counting_bloom_filter.hpp>
#include <fdeep/fdeep.hpp>

#include "pattern.hpp"
#include "sequence_iterator.hpp"

class PatternDetector
{

  public:

    PatternDetector(const btllib::CountingBloomFilter8& bf, const fdeep::model& model)
      : bf(bf)
      , model(model)
    {}

    /**
     * Get the edit pattern detected by the model
     * @param signature Input signature
     * @return Model's output as pattern object
     */
    Pattern get_pattern(SequenceIterator& seq_iter)
    {
        const auto in_width = model.get_input_shapes()[0].width_;
        const auto signature_length = fplus::just_with_default<std::size_t>(0, in_width);
        const auto signature = get_model_input(seq_iter, bf, signature_length);
        const auto model_output = model.predict({signature});
        auto pattern = model_output_to_pattern(model_output);
        if (pattern.get_count(Edit::Type::MISMATCH) > 0) {
            return pattern;
        }
        const auto num_deletions = get_num_deletions(seq_iter, model_output[0].depth(), bf);
        if (num_deletions > 0) {
            fill_first(pattern, Edit::Type::DELETION, num_deletions);
            return pattern;
        }
        const auto num_insertions =
          get_num_insertions(seq_iter, model_output[0].depth(), bf, model);
        if (num_insertions > 0) {
            fill_first(pattern, Edit::Type::INSERTION, num_insertions);
            return pattern;
        }
        return pattern;
    }

  private:

    const btllib::CountingBloomFilter8& bf;
    const fdeep::model& model;

    fdeep::tensor get_model_input(SequenceIterator seq_iter,
                                  const btllib::CountingBloomFilter8& bf,
                                  unsigned signature_length)
    {

        const auto shape = fdeep::tensor_shape(signature_length, seq_iter.get_num_seeds());
        fdeep::tensor model_input(shape, 1);
        bool has_next = true;
        for (unsigned i = 0; i < signature_length && has_next; i++) {
            for (unsigned j = 0; j < seq_iter.get_num_seeds(); j++) {
                const auto is_miss = bf.contains(seq_iter.get_seed_hashes(j)) == 0;
                model_input.set(fdeep::tensor_pos(i, j), is_miss ? 0.0 : 1.0);
            }
            has_next = seq_iter.next();
        }
        return model_input;
    }

    unsigned argmax(const fdeep::tensor& x)
    {
        double max_val = 0;
        unsigned idx_max = 0;
        for (unsigned i = 0; i < x.depth(); i++) {
            const auto x_i = x.get(fdeep::tensor_pos(i));
            if (x_i > max_val) {
                max_val = x_i;
                idx_max = i;
            }
        }
        return idx_max;
    }

    void fill_first(Pattern& pattern, Edit::Type val, unsigned num)
    {
        for (unsigned i = 0; i < num; i++) {
            pattern.set(i, val);
        }
    }

    unsigned get_num_insertions(SequenceIterator seq_iter,
                                unsigned pattern_length,
                                const btllib::CountingBloomFilter8& bf,
                                const fdeep::model& model)
    {
        const auto in_width = model.get_input_shapes()[0].width_;
        const auto signature_length = fplus::just_with_default<std::size_t>(0, in_width);
        for (unsigned num_ins = 1; num_ins <= pattern_length; num_ins++) {
            seq_iter.insert_last('N');
            const auto signature = get_model_input(seq_iter, bf, signature_length);
            const auto y = model.predict({signature})[0];
            bool check = true;
            for (unsigned i = 0; i < pattern_length && check; i++) {
                if ((i < num_ins && y.get(fdeep::tensor_pos(i)) < 0.5) ||
                    (i > num_ins && y.get(fdeep::tensor_pos(i)) >= 0.5)) {
                    check = false;
                }
            }
            if (check) {
                return num_ins;
            }
        }
        return 0;
    }

    bool check_fixes(SequenceIterator seq_iter,
                     const btllib::CountingBloomFilter8& bf,
                     unsigned num_checks)
    {
        while (num_checks-- > 0) {
            if (bf.contains(seq_iter.get_kmer_hashes()) == 0) {
                return false;
            }
            if (!seq_iter.next()) {
                return true;
            }
        }
        return true;
    }

    Pattern model_output_to_pattern(const fdeep::tensors& model_output)
    {
        Pattern pattern(model_output[0].depth());
        for (unsigned i = 0; i < pattern.get_length(); i++) {
            if (model_output[0].get(fdeep::tensor_pos(i)) >= 0.5) {
                pattern.set(i, Edit::Type::MISMATCH);
            }
        }
        return pattern;
    }

    unsigned get_num_deletions(SequenceIterator seq_iter,
                               unsigned pattern_length,
                               const btllib::CountingBloomFilter8& bf)
    {
        for (unsigned num_del = 1; num_del <= pattern_length; num_del++) {
            seq_iter.delete_last();
            if (check_fixes(seq_iter, bf, pattern_length)) {
                return num_del;
            }
        }
        return 0;
    }
};
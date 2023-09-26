#include "pattern_detector.hpp"

#include <bitset>

namespace {

using namespace aiedit;

inline fdeep::tensor get_model_input(SequenceIterator seq_iter,
                                     const btllib::CountingBloomFilter8& bf)
{
    const auto shape = fdeep::tensor_shape(seq_iter.get_seed_length(), seq_iter.get_num_seeds());
    fdeep::tensor model_input(shape, 1);
    bool has_next = true;
    for (unsigned i = 0; i < seq_iter.get_seed_length() && has_next; i++) {
        for (unsigned j = 0; j < seq_iter.get_num_seeds(); j++) {
            const auto is_miss = bf.contains(seq_iter.get_hashes(j)) == 0;
            model_input.set(fdeep::tensor_pos(i, j), is_miss ? 0.0 : 1.0);
        }
        has_next = seq_iter.next();
    }
    return model_input;
}

inline unsigned argmax(const fdeep::tensor& x)
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

inline void fill_first(Pattern& pattern, Edit::Type val, unsigned num)
{
    for (unsigned i = 0; i < num; i++) {
        pattern.set(i, val);
    }
}

inline unsigned get_num_insertions(SequenceIterator seq_iter,
                                   unsigned pattern_length,
                                   const btllib::CountingBloomFilter8& bf,
                                   const fdeep::model& model)
{
    for (unsigned num_ins = 1; num_ins <= pattern_length; num_ins++) {
        seq_iter.insert_last('N');
        const auto signature = get_model_input(seq_iter, bf);
        const auto model_output = model.predict({signature});
        const auto argmax_y = argmax(model_output[0]);
        if (argmax_y > 0) {
            return num_ins;
        }
    }
    return 0;
}

inline bool
check_fixes(SequenceIterator seq_iter, const btllib::CountingBloomFilter8& bf, unsigned num_checks)
{
    while (num_checks-- > 0) {
        for (unsigned i = 0; i < seq_iter.get_num_seeds(); i++) {
            if (bf.contains(seq_iter.get_hashes(i)) == 0) {
                return false;
            }
        }
        if (!seq_iter.next()) {
            return true;
        }
    }
    return true;
}

inline unsigned get_num_deletions(SequenceIterator seq_iter,
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

}  // namespace

namespace aiedit {

Pattern PatternDetector::get_pattern(SequenceIterator& seq_iter)
{
    const auto signature = get_model_input(seq_iter, bf);
    const auto model_output = model.predict({signature});
    const auto argmax_y = argmax(model_output[0]);
    const std::string pattern_string = std::bitset<64>(argmax_y).to_string();
    Pattern pattern(pattern_length);
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern_string[pattern_string.size() - i - 1] == '1') {
            pattern.set(i, Edit::Type::MISMATCH);
        }
    }
    if (pattern.get_count(Edit::Type::MISMATCH) > 0) {
        return pattern;
    }
    const auto num_deletions = get_num_deletions(seq_iter, pattern_length, bf);
    if (num_deletions > 0) {
        fill_first(pattern, Edit::Type::DELETION, num_deletions);
        return pattern;
    }
    const auto num_insertions = get_num_insertions(seq_iter, pattern_length, bf, model);
    if (num_insertions > 0) {
        fill_first(pattern, Edit::Type::INSERTION, num_insertions);
        return pattern;
    }
    return pattern;
}

}  // namespace aiedit
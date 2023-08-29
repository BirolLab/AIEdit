#include "pattern_detector.hpp"

#include <bitset>

namespace {

using namespace aiedit;

inline fdeep::tensor get_model_input(SequenceIterator seq_iter,
                                     unsigned pattern_length,
                                     const btllib::CountingBloomFilter8& bf)
{
    const unsigned signature_length = pattern_length + seq_iter.get_seed_length() - 1;
    const auto input_shape = fdeep::tensor_shape(signature_length, seq_iter.get_num_seeds());
    fdeep::tensor model_input(input_shape, 1);
    bool has_next = true;
    for (unsigned i = 0; i < signature_length && has_next; i++) {
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
        const auto signature = get_model_input(seq_iter, pattern_length, bf);
        const auto model_output = model.predict({signature});
        const auto argmax_y = argmax(model_output[0]);
        if (argmax_y > 0) {
            return num_ins;
        }
    }
    return 0;
}

inline unsigned get_num_deletions(SequenceIterator seq_iter,
                                  unsigned pattern_length,
                                  const btllib::CountingBloomFilter8& bf)
{
    seq_iter.next(seq_iter.get_seed_length());
    for (unsigned num_del = 1; num_del <= pattern_length; num_del++) {
        bool fixed = true;
        for (unsigned i = 0; i < seq_iter.get_num_seeds() && fixed; i++) {
            if (bf.contains(seq_iter.get_hashes(i)) == 0) {
                fixed = false;
            }
        }
        if (fixed) {
            return num_del;
        }
        seq_iter.next();
    }
    return 0;
}

}  // namespace

namespace aiedit {

Pattern PatternDetector::get_pattern(SequenceIterator& seq_iter)
{
    const auto signature = get_model_input(seq_iter, pattern_length, bf);
    const auto model_output = model.predict({signature});
    const auto argmax_y = argmax(model_output[0]);
    const std::string pattern_string = std::bitset<64>(argmax_y).to_string();
    Pattern pattern(pattern_length);
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern_string[pattern_string.size() - i - 1] == '1') {
            pattern.set(i, Edit::Type::MISMATCH);
        }
    }
    return pattern;
}

}  // namespace aiedit
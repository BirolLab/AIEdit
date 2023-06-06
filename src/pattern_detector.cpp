#include "pattern_detector.hpp"

namespace {

using namespace fdeep;

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

}  // namespace

namespace aiedit {

fdeep::tensor PatternDetector::get_model_input(SequenceIterator& seq_iter)
{
    unsigned signature_length = pattern_length + seq_iter.get_seed_length() - 1;
    const unsigned num_seeds = seq_iter.get_num_seeds();
    fdeep::tensor model_input(fdeep::tensor_shape(signature_length, num_seeds), 1);
    SequenceIterator seq_iter_copy(seq_iter);
    seq_iter_copy.previous();
    for (unsigned i = 0; i < signature_length && seq_iter_copy.has_next(); i++) {
        seq_iter_copy.next();
        const auto hashes = seq_iter_copy.get_hashes();
        for (unsigned j = 0; j < num_seeds; j++) {
            const auto is_miss = !bf.contains(hashes[j]);
            model_input.set(fdeep::tensor_pos(i, j), is_miss ? 0.0 : 1.0);
        }
    }
    return model_input;
}

Pattern PatternDetector::get_pattern(SequenceIterator& seq_iter)
{
    const auto& signature = get_model_input(seq_iter);
    const auto model_output = model.predict({signature}).front();
    const auto pattern_string = patterns[argmax(model_output)];
    Pattern pattern(pattern_length);
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (pattern_string[i] == '0') {
            pattern.set(i, Edit::NONE);
        } else if (pattern_string[i] == 'M') {
            pattern.set(i, Edit::MISMATCH);
        } else if (pattern_string[i] == 'I') {
            pattern.set(i, Edit::INSERTION);
        } else if (pattern_string[i] == 'D') {
            pattern.set(i, Edit::DELETION);
        }
    }
    return pattern;
}

}  // namespace aiedit
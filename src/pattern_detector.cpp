#include "pattern_detector.hpp"

namespace {

using namespace aiedit;

const Edit::Type EDIT_TYPES[4] = {
  Edit::Type::NONE,
  Edit::Type::MISMATCH,
  Edit::Type::INSERTION,
  Edit::Type::DELETION,
};

inline fdeep::tensor get_model_input(SequenceIterator seq_iter,
                                     unsigned pattern_length,
                                     const btllib::CountingBloomFilter8& bf)
{
    const unsigned signature_length = pattern_length + seq_iter.get_seed_length() - 1;
    const auto input_shape = fdeep::tensor_shape(signature_length, seq_iter.get_num_seeds());
    fdeep::tensor model_input(input_shape, 1);
    for (unsigned i = 0; i < signature_length && seq_iter.has_next(); i++) {
        const auto hashes = seq_iter.get_hashes();
        for (unsigned j = 0; j < seq_iter.get_num_seeds(); j++) {
            const auto is_miss = bf.contains(hashes[j]) == 0;
            model_input.set(fdeep::tensor_pos(i, j), is_miss ? 0.0 : 1.0);
        }
        seq_iter.next();
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

}  // namespace

namespace aiedit {

Pattern PatternDetector::get_pattern(SequenceIterator& seq_iter)
{
    const auto& signature = get_model_input(seq_iter, pattern_length, bf);
    auto model_output = model.predict({signature});
    Pattern pattern(pattern_length);
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        const auto argmax_y = argmax(model_output[i]);
        pattern.set(i, EDIT_TYPES[argmax_y]);
    }
    return pattern;
}

}  // namespace aiedit
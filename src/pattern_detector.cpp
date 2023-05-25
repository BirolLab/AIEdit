#include "pattern_detector.hpp"

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
    const double threshold = 0.5;
    auto model_output = model.predict({signature});
    Pattern pattern(pattern_length);
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        auto edit_type = Edit::NONE;
        if (model_output.front().get(fdeep::tensor_pos(3 * i)) >= threshold) {
            edit_type = Edit::MISMATCH;
        } else if (model_output.front().get(fdeep::tensor_pos(3 * i + 1)) >= threshold) {
            edit_type = Edit::INSERTION;
        } else if (model_output.front().get(fdeep::tensor_pos(3 * i + 2)) >= threshold) {
            edit_type = Edit::DELETION;
        }
        pattern.set(i, edit_type);
    }
    return pattern;
}

}  // namespace aiedit
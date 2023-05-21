#include "pattern_detector.hpp"

namespace aiedit {

fdeep::tensor PatternDetector::get_model_input(SequenceIterator& seq_iter)
{
    const unsigned signature_length = pattern_length + bf.get_seeds()[0].size() - 1;
    const unsigned num_seeds = bf.get_seeds().size();
    fdeep::tensor model_input(fdeep::tensor_shape(signature_length, num_seeds), 0);
    auto hashes = seq_iter.peek_hashes(signature_length);
    for (unsigned i = 0; i < signature_length; i++) {
        for (unsigned j = 0; j < num_seeds; j++) {
            const auto is_miss = !bf.contains(hashes[i][j]);
            model_input.set(fdeep::tensor_pos(i, j), is_miss ? 0.0 : 1.0);
        }
    }
    return model_input;
}

EditPattern PatternDetector::get_pattern(SequenceIterator& seq_iter)
{
    const auto& signature = get_model_input(seq_iter);
    const double threshold = 0.5;
    auto model_output = model.predict({signature});
    EditPattern pattern(pattern_length);
    for (unsigned i = 0; i < pattern.get_length(); i++) {
        if (model_output.front().get(fdeep::tensor_pos(i)) >= threshold) {
            pattern.set(i, Edit::MISMATCH);
        } else {
            pattern.set(i, Edit::NONE);
        }
    }
    return pattern;
}

}  // namespace aiedit
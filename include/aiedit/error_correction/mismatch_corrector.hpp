#ifndef AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP
#define AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP

#include <btllib/bloom_filter.hpp>
#include <torch/script.h>

#include "aiedit/edit_pattern.hpp"
#include "aiedit/error_correction/error_corrector.hpp"
#include "aiedit/signature.hpp"

namespace aiedit {

class MismatchCorrector : public ErrorCorrector
{
  public:
    MismatchCorrector(unsigned pattern_length,
                      const btllib::SeedBloomFilter& bf,
                      torch::jit::script::Module& model)
      : ErrorCorrector(pattern_length)
      , bf(bf)
      , model(model)
    {}

    /**
     * Fix the mismatches in the current position of the sequence iterator
     * @param seq_iter Sequence iterator pointing to the mismatch region
     * @return `true` if any edits were applied
     */
    bool fix(SequenceIterator& seq_iter) override;

  private:
    const btllib::SeedBloomFilter& bf;
    torch::jit::script::Module& model;

    /**
     * Prepare model input by updating the signature values
     * @param
     * @return Signature object containing model's input tensor
     */
    ModelInput get_model_input(SequenceIterator& seq_iter);

    /**
     * Get the edit pattern detected by the model
     * @param signature Input signature
     * @return Model's output as pattern object
     */
    EditPattern get_pattern(ModelInput& signature);

    /**
     * Get the positions in the sequence that need to be updated
     * @param base_position Position of the edit pattern in the sequence
     * @param pattern Edit pattern containing mismatches
     * @return Vector of positions containing mismatches
     */
    std::vector<size_t> get_mismatch_positions(const size_t base_position,
                                               const EditPattern& pattern);

    /**
     * Update the sequence with the new bases
     * @param positions Positions of the bases to be updated
     * @param new_bases New base values to replace the positions
     */
    void update_seq(SequenceIterator& seq_iter,
                    const std::vector<size_t>& positions,
                    const std::string& new_bases);

    /**
     * Peek the next signature and check for misses
     * @return `true` if the signature contains no misses
     */
    bool check_fixes(SequenceIterator& seq_iter);

    /**
     * Add local changes to list of fixes
     * @param positions List of updated positions in the sequence
     * @param reference Original content of the positions
     * @param updated Updated value in the positions
     */
    void add_edits(const std::vector<size_t>& positions,
                   const std::string& reference,
                   const std::string& updated);
};

}

#endif // AIEDIT_BLOOM_FILTER_MISMATCH_CORRECTOR_HPP
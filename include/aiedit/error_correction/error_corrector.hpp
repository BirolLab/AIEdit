#ifndef AIEDIT_ERROR_CORRECTOR_HPP
#define AIEDIT_ERROR_CORRECTOR_HPP

#include <vector>

#include "aiedit/edit.hpp"
#include "aiedit/edit_pattern.hpp"
#include "aiedit/sequence_iterator.hpp"

namespace aiedit {

class ErrorCorrector
{
  protected:
    std::vector<Edit> edits;
    const unsigned pattern_length;

  public:
    ErrorCorrector(unsigned pattern_length)
      : pattern_length(pattern_length)
    {}

    virtual ~ErrorCorrector() = default;

    /**
     * Fix the errors at the current position of the sequence iterator
     * @param seq_iter Sequence iterator pointing to the region to be edited
     * @return `true` if the sequence was edited
     */
    virtual bool fix(SequenceIterator& seq_iter) = 0;

    /**
     * Clear the list of edits
     */
    void clear_edits() { edits.clear(); }

    /**
     * Get a list of applied edits
     * @return List of pairs containing the positions, type, and value of the
     * fixes
     */
    const std::vector<Edit>& get_edits() const { return edits; };
};

}

#endif // AIEDIT_ERROR_CORRECTOR_HPP
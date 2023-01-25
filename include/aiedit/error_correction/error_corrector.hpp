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
    SequenceIterator& seq_iter;
    std::vector<Edit> edits;

  public:
    ErrorCorrector(SequenceIterator& seq_iter)
      : seq_iter(seq_iter)
    {}

    virtual ~ErrorCorrector() = default;

    /**
     * Fix the errors at the current position of the sequence iterator
     * @param pattern Edit pattern for the current position
     * @return `true` if the sequence was edited
     */
    virtual bool fix(const EditPattern& pattern) = 0;

    /**
     * Get a list of applied edits
     * @return List of pairs containing the positions, type, and value of the
     * fixes
     */
    const std::vector<Edit>& get_edits() const { return edits; };
};

}

#endif // AIEDIT_ERROR_CORRECTOR_HPP
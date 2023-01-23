#ifndef AIEDIT_ERROR_DETECTOR
#define AIEDIT_ERROR_DETECTOR

#include "sequence_iterator.hpp"

namespace aiedit {

class ErrorDetector
{
protected:
  SequenceIterator& seq_iter;

public:
  ErrorDetector(SequenceIterator& seq_iter)
    : seq_iter(seq_iter)
  {}

  /**
   * Check if the sequence has any more errors
   * @return `true` if there are any more errors in the sequence, `false`
   * otherwise
   */
  virtual bool has_error() = 0;

  /**
   * Advance the sequence iterator to the next erroneous position
   */
  virtual void next_error() = 0;

};

};

#endif // AIEDIT_ERROR_DETECTOR
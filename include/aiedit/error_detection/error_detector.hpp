#ifndef AIEDIT_ERROR_DETECTOR
#define AIEDIT_ERROR_DETECTOR

#include "aiedit/sequence_iterator.hpp"

namespace aiedit {

class ErrorDetector
{
  protected:
    SequenceIterator& seq_iter;

  public:
    ErrorDetector(SequenceIterator& seq_iter)
      : seq_iter(seq_iter)
    {}

    virtual ~ErrorDetector() = default;

    /**
     * Advance the sequence iterator to the next erroneous position
     * @return `false` if iteration has ended
     */
    virtual bool next_error() = 0;
};

}

#endif // AIEDIT_ERROR_DETECTOR
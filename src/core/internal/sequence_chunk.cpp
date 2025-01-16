#include "sequence_chunk.hpp"

namespace aiedit::internal {

SequenceChunk::SequenceChunk(
  const std::string& seq, size_t start_pos, size_t end_pos, unsigned num_hashes, unsigned kmer_size)
  : seq(seq)
  , hash_fn(seq, num_hashes, kmer_size, start_pos)
  , end_pos(end_pos)
{}

bool SequenceChunk::roll()
{
    if (hash_fn.get_pos() < end_pos) {
        hash_fn.roll(seq[hash_fn.get_pos() + hash_fn.get_k()]);
        return true;
    } else {
        return false;
    }
}

const uint64_t* SequenceChunk::hashes() const { return hash_fn.hashes(); }

size_t SequenceChunk::get_pos() const { return hash_fn.get_pos(); }

}
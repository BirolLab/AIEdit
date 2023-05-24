#ifndef AIEDIT_SEQ_HPP
#define AIEDIT_SEQ_HPP

#include <nthash/nthash.hpp>
#include <string>
#include <vector>

namespace aiedit {

class SequenceIterator
{
  public:

    using HashVector = std::vector<std::vector<uint64_t>>;

    SequenceIterator(std::string& seq, const std::vector<std::string>& seeds, unsigned num_hashes)
      : SequenceIterator(seq, seeds, num_hashes, 0, seq.size())
    {}

    SequenceIterator(std::string& seq,
                     const std::vector<std::string>& seeds,
                     unsigned num_hashes,
                     size_t begin,
                     size_t end)
      : seq(seq)
      , seeds(seeds)
      , hash_fn(seq, seeds, num_hashes, seeds[0].size(), begin)
      , begin(begin)
      , end(end)
    {}

    SequenceIterator(const SequenceIterator& seq_iter)
      : seq(seq_iter.seq)
      , seeds(seq_iter.seeds)
      , hash_fn(seq_iter.hash_fn)
      , begin(seq_iter.begin)
      , end(seq_iter.end)
    {}

    /**
     * Advance to the next k-mer
     * @param n Number of bases to roll
     */
    void next(unsigned n = 1);

    /**
     * Roll to the previous k-mer
     * @param n Number of bases to roll
     */
    void previous(unsigned n = 1);

    /**
     * Check if the iterator can advance
     * @return `true` if the iterator can advance, `false` if iterator is at the
     * end
     */
    bool has_next();

    /**
     * Get value of a base in a position
     * @param position Position of the base
     * @return Value in the position
     */
    char get_base(size_t position);

    /**
     * Get the position of the current base
     * @return Position of the current k-mer's last character in the sequence
     */
    size_t get_position();

    /**
     * Get the current k-mers hash values
     * @return Vector containing hash arrays generated for each spaced seed
     */
    HashVector get_hashes();

    /**
     * Get the length of the spaced seeds
     * @return Length of each spaced seed
     */
    unsigned get_seed_length();

    /**
     * Get the length of the spaced seeds
     * @return Length of each spaced seed
     */
    unsigned get_num_seeds();

    /**
     * Get the sequence contents
     * @return `const` reference to the sequence string
     */
    const std::string& get_sequence();

    /**
     * Get the next `window_size` hashes without advancing the iterator
     * @param window_size Number of iteration to peek
     * @return Array of vectors containing the hash arrays (from `get_hashes`)
     */
    std::vector<HashVector> peek_hashes(unsigned window_size);

    /**
     * Update a base's value
     * @param position Position of the base
     * @param value Base's new value
     */
    void update(size_t position, char value);

  private:

    std::string& seq;
    const std::vector<std::string>& seeds;
    nthash::SeedNtHash hash_fn;
    const size_t begin, end;

    /**
     * Convert an ntHash hash array to a vector of hashes
     * @param nthash_hashes Array of hash values from ntHash
     * @return Vector of hash arrays, one for each spaced seed
     */
    HashVector to_hash_vector(const uint64_t* nthash_hashes);
};

}  // namespace aiedit

#endif  // AIEDIT_SEQ_HPP
#ifndef AIEDIT_SEQ_HPP
#define AIEDIT_SEQ_HPP

#include <nthash/nthash.hpp>
#include <string>
#include <vector>

namespace aiedit {

class SequenceIterator
{
  public:
    using HashVector = std::vector<const uint64_t*>;

    SequenceIterator(std::string& seq,
                     const std::vector<std::string>& seeds,
                     const unsigned num_hashes)
      : seq(seq)
      , seeds(seeds)
      , nthash(new nthash::SeedNtHash(seq, seeds, num_hashes, seeds[0].size()))
    {}

    /**
     * Advance to the next k-mer
     */
    void next(unsigned n = 1);

    /**
     * Roll to the previous k-mer
     */
    void previous();

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
     * Get the sequence contents
     * @return `const` reference to the sequence string
     */
    const std::string& get_sequence();

    /**
     * Get the next `window_size` hashes without advancing the iterator
     * @param window_size Number of iteration to peek
     * @return Array of vectors containing the hash arrays (from `get_hashes`)
     */
    HashVector* peek_hashes(unsigned window_size);

    /**
     * Update a base's value
     * @param position Position of the base
     * @param value Base's new value
     */
    void update(size_t position, char value);

    /**
     * Insert a base in a position
     * @param position Final position of the new base
     * @param value Value to insert
     */
    void insert(size_t position, char value);

    /**
     * Delete a base in a position
     * @param position Base's position
     */
    void remove(size_t position);

  private:
    std::string& seq;
    const std::vector<std::string>& seeds;
    nthash::SeedNtHash* nthash;

    /**
     * Convert an ntHash hash array to a vector of hashes
     * @param nthash_hashes Array of hash values from ntHash
     * @return Vector of hash arrays, one for each spaced seed
     */
    HashVector to_hash_vector(const uint64_t* nthash_hashes);
};

}

#endif // AIEDIT_SEQ_HPP
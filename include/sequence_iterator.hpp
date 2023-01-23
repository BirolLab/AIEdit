#ifndef AIEDIT_SEQ_HPP
#define AIEDIT_SEQ_HPP

#include <nthash/nthash.hpp>
#include <string>
#include <vector>

namespace aiedit {

class SequenceIterator
{
public:
  SequenceIterator(std::string& seq,
                   const std::vector<std::string>& seeds,
                   const unsigned num_hashes)
    : nthash(new nthash::SeedNtHash(seq, seeds, num_hashes, seeds[0].size()))
    , can_roll(true)
    , seeds(seeds)
  {}

  /**
   * Advance to the next k-mer
   */
  void next();

  /**
   * Check if the iterator can advance
   * @return `true` if the iterator can advance, `false` if iterator is at the
   * end
   */
  bool has_next();

  /**
   * Get the position of the current k-mer
   * @return Position of the current k-mer's first character in the sequence
   */
  size_t get_position();

  /**
   * Get the current k-mers hash values
   * @return Vector containing hash arrays generated for each spaced seed
   */
  std::vector<const uint64_t*> get_hashes();

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
  nthash::SeedNtHash* nthash;
  bool can_roll;
  const std::vector<std::string>& seeds;
};

};

#endif // AIEDIT_SEQ_HPP
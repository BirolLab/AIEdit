#ifndef AIEDIT_DATA_TYPES_HPP
#define AIEDIT_DATA_TYPES_HPP

#include <deque>
#include <string>
#include <vector>

namespace ai_edit {

using SpacedSeed = std::string;

class EditPattern
{
public:
  enum Value
  {
    CLEAN,
    MISMATCH
  };

  explicit EditPattern(const unsigned window_size)
    : window_size(window_size)
    , values(new Value[window_size])
  {
    for (size_t i = 0; i < window_size; i++) {
      values[i] = Value::CLEAN;
    }
  }

  void set(size_t i, Value x) { values[i] = x; }
  [[nodiscard]] Value get(size_t i) const { return values[i]; }
  [[nodiscard]] std::string to_string() const;
  [[nodiscard]] std::vector<size_t> get_edit_positions();
  [[nodiscard]] unsigned size() const { return window_size; }

private:
  const unsigned window_size;
  Value* values;
};

class Signature
{
public:
  enum Value
  {
    HIT,
    MISS
  };

public:
  class Hashes
  {
  private:
    uint64_t*** hashes;

  public:
    Hashes(const unsigned window_length, const unsigned num_seeds)
    {
      hashes = new uint64_t**[window_length];
      for (size_t i = 0; i < window_length; i++) {
        hashes[i] = new uint64_t*[num_seeds];
        for (size_t j = 0; j < num_seeds; j++) {
          hashes[i][j] = new uint64_t[2];
        }
      }
    }

    void set_forward_hash(uint64_t value,
                          const size_t slide,
                          const size_t i_seed)
    {
      hashes[slide][i_seed][0] = value;
    }

    void set_reverse_hash(uint64_t value,
                          const size_t slide,
                          const size_t i_seed)
    {
      hashes[slide][i_seed][1] = value;
    }

    [[nodiscard]] uint64_t get_forward_hash(const size_t slide,
                                            const size_t i_seed)
    {
      return hashes[slide][i_seed][0];
    }

    [[nodiscard]] uint64_t get_reverse_hash(const size_t slide,
                                            const size_t i_seed)
    {
      return hashes[slide][i_seed][1];
    }
  };

  Signature(const unsigned length, const unsigned num_seeds)
    : length(length)
    , num_seeds(num_seeds)
    , hash_values(new Hashes(length, num_seeds))
  {
    for (unsigned i = 0; i < length; i++) {
      values.push_back(new Value[num_seeds]);
    }
  }

  [[nodiscard]] unsigned get_frame_size() const { return length; }

  [[nodiscard]] unsigned get_num_seeds() const { return num_seeds; }

  [[nodiscard]] std::vector<std::string> to_string_vec() const;

  [[nodiscard]] Value get(unsigned i_slide, unsigned i_seed) const
  {
    return values[i_slide][i_seed];
  }

  void set(unsigned i_slide,
           unsigned i_seed,
           Value value,
           uint64_t fwd_hash = 0,
           uint64_t rev_hash = 0)
  {
    values[i_slide][i_seed] = value;
  }

  [[nodiscard]] Hashes& hashes() { return *hash_values; }

  void push(Value* x);

  static Signature predict(const EditPattern& pattern,
                           unsigned frame_size,
                           const std::vector<SpacedSeed>& seeds);

private:
  std::deque<Value*> values;
  const unsigned length, num_seeds;
  Hashes* hash_values;
};

}

#endif // AIEDIT_DATA_TYPES_HPP

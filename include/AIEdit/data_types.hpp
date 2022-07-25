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

  Signature(const unsigned length, const unsigned num_seeds)
    : length(length)
    , num_seeds(num_seeds)
  {
    for (unsigned i = 0; i < length; i++) {
      values.push_back(new Value[num_seeds]);
    }
  }

  [[nodiscard]] unsigned get_frame_size() const { return length; }
  [[nodiscard]] unsigned get_num_seeds() const { return num_seeds; }

  [[nodiscard]] Value get(unsigned i_slide, unsigned i_seed) const
  {
    return values[i_slide][i_seed];
  }

  [[nodiscard]] std::vector<std::string> to_string_vec() const;

  void set(unsigned i_slide, unsigned i_seed, Value value)
  {
    values[i_slide][i_seed] = value;
  }

  void push(Value* x);

  static Signature predict(const EditPattern& pattern,
                           unsigned frame_size,
                           const std::vector<SpacedSeed>& seeds);

private:
  std::deque<Value*> values;
  const unsigned length, num_seeds;
};

}

#endif // AIEDIT_DATA_TYPES_HPP

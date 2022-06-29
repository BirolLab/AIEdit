#ifndef AIEDIT_DATA_TYPES_HPP
#define AIEDIT_DATA_TYPES_HPP

#include <deque>
#include <string>
#include <vector>

namespace ai_edit {

using SpacedSeed = std::string;

class Pattern
{
public:
  enum PatternValue
  {
    CLEAN,
    MISMATCH
  };

  explicit Pattern(const unsigned window_size)
    : window_size(window_size)
    , values(new PatternValue[window_size])
  {
    for (size_t i = 0; i < window_size; i++) {
      values[i] = PatternValue::CLEAN;
    }
  }

  void set(size_t i, PatternValue x) { values[i] = x; }
  [[nodiscard]] PatternValue get(size_t i) const { return values[i]; }
  [[nodiscard]] std::string to_string() const;

private:
  PatternValue* values;
  const unsigned window_size;
};

class Signature
{
public:
  enum SignatureValue
  {
    HIT,
    MISS
  };

  Signature(const unsigned frame_size, const unsigned num_seeds)
    : frame_size(frame_size)
    , num_seeds(num_seeds)
  {
    for (unsigned i = 0; i < frame_size; i++) {
      values.push_back(new SignatureValue[num_seeds]);
    }
  }

  [[nodiscard]] unsigned get_frame_size() const { return frame_size; }
  [[nodiscard]] unsigned get_num_seeds() const { return num_seeds; }
  [[nodiscard]] SignatureValue get(unsigned i, unsigned j) const
  {
    return values[i][j];
  }

  [[nodiscard]] std::vector<std::string> to_string_vec() const;

  void set(unsigned i, unsigned j, SignatureValue value)
  {
    values[i][j] = value;
  }
  void push(SignatureValue* x);

  static Signature predict(const Pattern& pattern,
                           unsigned frame_size,
                           const std::vector<SpacedSeed>& seeds);

private:
  std::deque<SignatureValue*> values;
  const unsigned frame_size, num_seeds;
};

}

#endif // AIEDIT_DATA_TYPES_HPP

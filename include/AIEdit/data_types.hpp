#ifndef AIEDIT_DATA_TYPES_HPP
#define AIEDIT_DATA_TYPES_HPP

#include <deque>
#include <string>
#include <vector>

namespace ai_edit {

using Pattern = std::vector<bool>;
using SpacedSeed = std::string;

class Signature
{
private:
  std::deque<bool*> values;
  const unsigned frame_size, num_seeds;

public:
  Signature(const unsigned frame_size, const unsigned num_seeds)
    : frame_size(frame_size)
    , num_seeds(num_seeds)
  {
    for (unsigned i = 0; i < frame_size; i++) {
      values.push_back(new bool[num_seeds]());
    }
  }

  [[nodiscard]] unsigned get_frame_size() const { return frame_size; }
  [[nodiscard]] unsigned get_num_seeds() const { return num_seeds; }
  [[nodiscard]] bool get(unsigned i, unsigned j) { return values[i][j]; }

  [[nodiscard]] std::vector<std::string> to_string_vec() const;

  void set(unsigned i, unsigned j, bool value) { values[i][j] = value; }
  void push(bool* x);

  static Signature predict(const Pattern& pattern,
                           unsigned frame_size,
                           const std::vector<SpacedSeed>& seeds);

};

}

#endif // AIEDIT_DATA_TYPES_HPP

#include "AIEdit/observer.hpp"

bool
ai_edit::Observer::next()
{
  while (true) {
    for (size_t i = 0; i < hash_fns.size(); i++) {
      if (!hash_fns[i]->roll()) {
        return false;
      }
      if (!filter.contains(hash_fns[i]->hashes())) {
        for (size_t j = 0; j <= i; j++) {
          hash_fns[j]->roll_back();
        }
        update_signature();
        return true;
      }
    }
  }
}

void
ai_edit::Observer::update_signature()
{
  for (size_t i = 0; i < frame_size; i++) {
    for (unsigned j = 0; j < filter.get_seeds().size(); j++) {
      if (!hash_fns[j]->roll()) {
        signature.set(i, j, Signature::Value::HIT);
      } else if (filter.contains(hash_fns[j]->hashes())) {
        signature.set(i, j, Signature::Value::HIT);
      } else {
        signature.set(i, j, Signature::Value::MISS);
      }
    }
  }
  for (unsigned i = 0; i < filter.get_seeds().size(); i++) {
    unsigned rolls = filter.get_seeds()[i].size() - frame_size + window_size;
    for (unsigned j = 0; j < rolls; j++) {
      hash_fns[i]->roll();
    }
  }
}

size_t
ai_edit::Observer::get_position() const
{
  size_t position = hash_fns[0]->get_pos();
  for (const auto& hash_fn : hash_fns) {
    if (hash_fn->get_pos() != position) {
      const std::string red("\033[0;31m");
      const std::string reset("\033[0m");
      std::cout << std::endl;
      std::cout << red << "[ERROR]" << reset
                << " Hash functions are out of sync: ";
      for (const auto& nth : hash_fns) {
        std::cout << nth->get_pos() << " ";
      }
      std::cout << std::endl;
      exit(1);
    }
  }
  return position - window_size + 1;
}

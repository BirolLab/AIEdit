#include "AIEdit/observer.hpp"

bool
ai_edit::Observer::next()
{
  bool miss = false;
  while (!miss) {
    for (const auto& hash_fn : hash_fns) {
      if (!hash_fn->roll()) {
        return false;
      }
      if (!filter.contains(hash_fn->hashes())) {
        miss = true;
        hash_fn->roll_back();
        break;
      }
    }
  }
  update_signature();
  return true;
}

void
ai_edit::Observer::update_signature()
{
  std::cout << get_position() << std::endl;
  for (size_t i = 0; i < frame_size; i++) {
    for (unsigned j = 0; j < seeds.size(); j++) {
      if (!hash_fns[j]->roll()) {
        signature.set(i, j, true);
      } else {
        signature.set(i, j, filter.contains(hash_fns[j]->hashes()));
      }
    }
  }
}

size_t
ai_edit::Observer::get_position()
{
  size_t position = 0;
  for (const auto& hash_fn : hash_fns) {
    position = std::max(position, hash_fn->get_pos() + hash_fn->get_k());
  }
  return position;
}
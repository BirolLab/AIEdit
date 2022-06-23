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
      miss = !filter.contains(hash_fn->hashes());
    }
    ++position;
  }
  update_signature();
  return true;
}

void
ai_edit::Observer::update_signature()
{
  for (size_t i = 0; i < frame_size; i++) {
    for (unsigned j = 0; j < seeds.size(); j++) {
      if (!hash_fns[j]->roll()) {
        return;
      }
      signature.set(i, j, filter.contains(hash_fns[j]->hashes()));
    }
  }
}
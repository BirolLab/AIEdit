#include "AIEdit/observer.hpp"

bool
ai_edit::Observer::next()
{
  while (true) {
    for (const auto& hash_fn : hash_fns) {
      if (!hash_fn->roll()) {
        return false;
      }
      if (!filter.contains(hash_fn->hashes())) {
        hash_fn->roll_back();
        update_signature();
        return true;
      }
    }
    ++position;
  }
}

void
ai_edit::Observer::update_signature()
{
  for (size_t i = 0; i < frame_size; i++) {
    for (unsigned j = 0; j < filter.get_seeds().size(); j++) {
      if (!hash_fns[j]->roll()) {
        signature.set(i, j, Signature::SignatureValue::HIT);
      } else if (filter.contains(hash_fns[j]->hashes())) {
        signature.set(i, j, Signature::SignatureValue::HIT);
      } else {
        signature.set(i, j, Signature::SignatureValue::MISS);
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

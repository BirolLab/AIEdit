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
      bool rolled = hash_fns[j]->roll();
      Signature::Value value;
      if (rolled && !filter.contains(hash_fns[j]->hashes())) {
        value = Signature::Value::MISS;
      } else {
        value = Signature::Value::MISS;
      }
      signature.set(i, j, value);
      std::copy(hash_fns[j]->hashes(),
                hash_fns[j]->hashes() + filter.get_hash_num_per_seed(),
                signature_hashes[i][j]);
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

uint64_t***
ai_edit::Observer::get_signature_hashes()
{
  size_t x = frame_size, y = filter.get_seeds().size(),
         z = filter.get_hash_num_per_seed();
  uint64_t*** hashes_copy = new uint64_t**[x];
  for (size_t i = 0; i < x; i++) {
    hashes_copy[i] = new uint64_t*[y];
    for (size_t j = 0; j < y; j++) {
      hashes_copy[i][j] = new uint64_t[z];
    }
  }
  std::copy(&signature_hashes[0][0][0],
            &signature_hashes[0][0][0] + x * y * z,
            &hashes_copy[0][0][0]);
  return hashes_copy;
}
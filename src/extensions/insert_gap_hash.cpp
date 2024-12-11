#include "insert_gap_hash.hpp"

#include <btllib/nthash.hpp>
#include <stdexcept>

InsertGapHash::InsertGapHash(
  const std::string& seq, unsigned num_hashes, unsigned kmer_size, unsigned max_gap, size_t pos)
  : num_hashes(num_hashes)
  , seq(seq)
{
    if (kmer_size / 2 < 3) {
        throw std::runtime_error("[InsertGapHash] k-mer size is too short");
    }
    for (unsigned gap = 1; gap <= max_gap; gap++) {
        const unsigned k_f = (kmer_size - gap) / 2;
        const unsigned k_s = k_f + (kmer_size - gap) % 2;
        const size_t pos_s = pos + k_f;
        hashers.emplace_back(btllib::NtHash(seq, 1, k_f, pos), btllib::NtHash(seq, 1, k_s, pos_s));
    }
}

bool InsertGapHash::roll()
{
    bool can_roll = false;
    for (auto& hasher_pair : hashers) {
        can_roll = hasher_pair.first.roll();
        can_roll = hasher_pair.second.roll();
    }
    return can_roll;
}

std::vector<uint64_t*> InsertGapHash::hashes() const
{
    std::vector<uint64_t*> hashes;
    const auto k = hashers[0].first.get_k() + hashers[0].second.get_k() + 1;
    for (unsigned g = 1; g <= hashers.size(); g++) {
        const auto& h_pair = hashers[g - 1];
        auto f_f = h_pair.first.get_forward_hash();
        auto f_r = h_pair.first.get_reverse_hash();
        auto s_f = h_pair.second.get_forward_hash();
        auto s_r = h_pair.second.get_reverse_hash();
        auto f_i = btllib::hashing_internals::srol(f_f, h_pair.second.get_k() + g) ^ s_f;
        auto r_i = f_r ^ btllib::hashing_internals::srol(s_r, h_pair.first.get_k() + g);
        if (h_pair.first.get_k() != h_pair.second.get_k()) {
            unsigned char b_out = seq[h_pair.first.get_pos() + h_pair.first.get_k()];
            f_i ^= btllib::hashing_internals::srol_table(b_out, h_pair.second.get_k() - 1);
            b_out &= btllib::hashing_internals::CP_OFF;
            r_i ^= btllib::hashing_internals::srol_table(b_out, h_pair.first.get_k() + g);
        }
        hashes.emplace_back(new uint64_t[num_hashes]);
        btllib::hashing_internals::extend_hashes(f_i, r_i, k, num_hashes, hashes[g - 1]);
    }
    return hashes;
}

size_t InsertGapHash::get_pos() const { return hashers[0].first.get_pos(); }

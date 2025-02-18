#include "environment.hpp"

#include <stdexcept>

namespace aiedit {

Environment::State::State(unsigned signature_length, unsigned num_seeds)
  : signature(signature_length, num_seeds)
{}

Environment::Environment(const std::string& seq,
                         size_t start,
                         size_t end,
                         std::shared_ptr<KmerModel> km)
  : seq(seq)
  , start(start)
  , end(end)
  , kmer_model(km)
  , hash_fn(seq.data(), km->seeds, km->get_num_hashes(), km->get_kmer_size(), start - 1)
  , state(std::make_shared<Environment::State>(end - start, km->seeds.size()))
  , pos(start + km->get_kmer_size())
{
    update_state();
    initial_value = get_value();
}

float Environment::act(Edit::Type edit_type, char new_base)
{
    if (is_terminated()) {
        throw std::runtime_error("Environment has been terminated");
    }
    if (edit_type == Edit::Type::SUBSTITUTE) {
        edit_history.emplace_back(Edit::substitution(pos, seq[pos], new_base));
        hash_fn.roll(new_base);
        ++pos;
    } else if (edit_type == Edit::Type::INSERT) {
        edit_history.emplace_back(Edit::insertion(pos, new_base));
        hash_fn.roll(new_base);
    } else if (edit_type == Edit::Type::DELETE) {
        edit_history.emplace_back(Edit::deletion(pos, seq[pos]));
        ++pos;
    } else if (edit_type == Edit::Type::NONE) {
        hash_fn.roll(seq[pos]);
        ++pos;
    }
    update_state();
    return get_value() - initial_value;
}

std::shared_ptr<Environment::State> Environment::get_state() const { return state; }

void Environment::terminate() { pos = end; }

bool Environment::is_terminated() const { return pos >= end; }

void Environment::update_state()
{
    btllib::BlindSeedNtHash hash_fn(this->hash_fn);
    for (size_t pos = edit_history.size(); pos < end - start; pos++) {
        hash_fn.roll(seq[start + pos]);
        for (size_t seed = 0; seed < kmer_model->seeds.size(); seed++) {
            const uint64_t* hashes = hash_fn.hashes() + (seed * kmer_model->get_num_hashes());
            const auto prob = kmer_model->score(hashes);
            state->signature.set(pos, seed, prob);
        }
    }
}

float Environment::get_value()
{
    float value = 0;
    const auto signature_size = state->signature.get_length() * state->signature.get_num_seeds();
    for (size_t i = 0; i < signature_size; i++) {
        value += state->signature.data()[i];
    }
    return value;
}

}


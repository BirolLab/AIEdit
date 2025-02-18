#pragma once

#include <btllib/nthash.hpp>
#include <cstddef>
#include <memory>
#include <string>

#include "edit.hpp"
#include "kmer_model.hpp"
#include "signature.hpp"

namespace aiedit {

class Environment
{

  public:

    struct State {
        Signature signature;

        State(unsigned signature_length, unsigned num_seeds);
    };

    Environment(const std::string& seq, size_t start, size_t end, std::shared_ptr<KmerModel> kmer_model);

    float act(Edit::Type edit_type, char new_base);

    std::shared_ptr<State> get_state() const;

    void terminate();

    bool is_terminated() const;

  private:

    const std::string& seq;
    const size_t start, end;
    std::shared_ptr<KmerModel> kmer_model;
    btllib::BlindSeedNtHash hash_fn;
    std::shared_ptr<State> state;
    std::vector<Edit> edit_history;
    size_t pos;
    float initial_value;

    void update_state();
    float get_value();
};

}
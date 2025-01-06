#include "edit_finder.hpp"

#include "utils.hpp"

#include <btllib/nthash.hpp>
#include <cmath>
#include <queue>
#include <tuple>

namespace {

constexpr char UNKNOWN_BASE = '?';

double score(btllib::BlindNtHash nh,
             const std::string& candidate,
             const aiedit::CountProbabilities& cprobs)
{
    double sum_log_probs = 0;
    for (size_t i = 0; i < candidate.size(); i++) {
        sum_log_probs += std::log(cprobs.get_prob(nh.hashes()) + 1e-9);
    }
    return -sum_log_probs / candidate.size();
}

std::string generate_unknown(const std::string& subseq,
                             const std::vector<aiedit::Edit::Type>& pattern)
{
    std::vector<aiedit::Edit> edits;
    for (size_t i = 0; i < pattern.size(); i++) {
        if (pattern[i] == aiedit::Edit::Type::SUBSTITUION) {
            edits.emplace_back(aiedit::Edit::substitution(i, subseq[i], UNKNOWN_BASE));
        } else if (pattern[i] == aiedit::Edit::Type::DELETION) {
            edits.emplace_back(aiedit::Edit::deletion(i, subseq[i]));
        } else if (pattern[i] == aiedit::Edit::Type::INSERTION) {
            edits.emplace_back(aiedit::Edit::insertion(i, UNKNOWN_BASE));
        }
    }
    return aiedit::utils::apply_edits(subseq, edits);
}

std::priority_queue<std::pair<double, std::string>>
rank_candidates(const std::string& unknown,
                btllib::BlindNtHash base_hash,
                const aiedit::CountProbabilities& cprobs)
{
    std::priority_queue<std::pair<double, std::string>> results;
    std::queue<std::tuple<double, std::shared_ptr<btllib::BlindNtHash>, std::string>> q;
    q.emplace(0.0, std::make_shared<btllib::BlindNtHash>(base_hash), "");
    while (!q.empty()) {
        auto score = std::get<0>(q.front());
        auto candidate = std::get<2>(q.front());
        if (candidate.size() > 2 && score / candidate.size() < -std::log(0.5)) {
            q.pop();
            continue;
        }
        if (candidate.size() == unknown.size()) {
            results.emplace(score, std::move(candidate));
        } else if (unknown[candidate.size()] == UNKNOWN_BASE) {
            for (auto b : {'A', 'C', 'G', 'T'}) {
                btllib::BlindNtHash hash_func = *std::get<1>(q.front()).get();
                hash_func.roll(b);
                auto b_prob = -std::log(cprobs.get_prob(hash_func.hashes()) + 1e-9);
                const auto h_ptr = std::make_shared<btllib::BlindNtHash>(base_hash);
                q.emplace(score + b_prob, h_ptr, candidate + b);
            }
        } else {
            btllib::BlindNtHash hash_func = *std::get<1>(q.front()).get();
            hash_func.roll(unknown[candidate.size()]);
            const auto h_ptr = std::make_shared<btllib::BlindNtHash>(base_hash);
            score += -std::log(cprobs.get_prob(hash_func.hashes()) + 1e-9);
            q.emplace(score, h_ptr, candidate + unknown[candidate.size()]);
        }
        q.pop();
    }
    return results;
}

}

namespace aiedit {

EditFinder::EditFinder(const CountProbabilities& cprobs, unsigned kmer_size)
  : cprobs(cprobs)
  , kmer_size(kmer_size)
{}

bool EditFinder::get_edits(const std::string& seq,
                           size_t pos,
                           const std::vector<Edit::Type>& pattern,
                           std::vector<Edit>& out_edits) const
{
    const auto sub_seq = seq.substr(pos + kmer_size, pattern.size() + kmer_size);
    const auto unknown = generate_unknown(sub_seq, pattern);
    btllib::BlindNtHash base_hash(seq, cprobs.get_num_hashes(), kmer_size, pos - 1);
    const auto results = rank_candidates(unknown, base_hash, cprobs);
    if (results.empty() || results.top().first <= score(base_hash, results.top().second, cprobs)) {
        return false;
    }
    std::queue<char> afters;
    for (const auto b : unknown) {
        if (b == UNKNOWN_BASE) {
            afters.push(b);
        }
    }
    for (size_t i = 0; i < pattern.size(); i++) {
        const auto base_pos = pos + kmer_size + i;
        if (pattern[i] == Edit::Type::SUBSTITUION) {
            out_edits.emplace_back(Edit::substitution(base_pos, seq[base_pos], afters.front()));
            afters.pop();
        } else if (pattern[i] == Edit::Type::DELETION) {
            out_edits.emplace_back(Edit::deletion(base_pos, seq[base_pos]));
        } else if (pattern[i] == Edit::Type::INSERTION) {
            out_edits.emplace_back(Edit::insertion(base_pos, afters.front()));
            afters.pop();
        }
    }
    return true;
}

}
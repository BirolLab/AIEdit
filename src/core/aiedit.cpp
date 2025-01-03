#include "aiedit.hpp"

#include "edit_finder.hpp"
#include "feature_extraction.hpp"

#include <algorithm>
#include <fstream>
#include <queue>
#include <stdexcept>

namespace {

constexpr size_t MIN_CHUNK_SIZE = 1000;

std::vector<std::string> read_seeds(const std::string& seeds_path)
{
    std::ifstream seeds_file(seeds_path);
    if (!seeds_file) {
        throw std::runtime_error("Unable to open seeds file: " + seeds_path);
    }
    return {std::istream_iterator<std::string>(seeds_file), std::istream_iterator<std::string>()};
}

struct SequenceIterator {
    const std::string& seq;
    btllib::BlindNtHash& nh;
    size_t end;
    const aiedit::CountProbabilities& cprobs;
    double threshold;

    bool next(bool hit)
    {
        while (nh.get_pos() <= end) {
            nh.roll(seq[nh.get_pos() + nh.get_k()]);
            if ((cprobs.get_prob(nh.hashes()) >= threshold) == hit) {
                return true;
            }
        }
        return false;
    }

    bool next_hit() { return next(true); }
    bool next_miss() { return next(false); }
};

struct SequenceChunk {
    const size_t start, end;
    aiedit::Results results;
};

inline std::vector<SequenceChunk> init_chunks(size_t seq_len, unsigned num_threads)
{
    std::vector<SequenceChunk> chunks;
    unsigned num_chunks = (seq_len / num_threads < MIN_CHUNK_SIZE) ? 1 : num_threads;
    const unsigned chunk_size = seq_len / num_chunks;
    chunks.reserve(num_chunks);
    for (unsigned i = 0; i < num_chunks; i++) {
        const size_t start = i * chunk_size;
        const size_t end = (i < num_chunks - 1) ? (i + 1) * chunk_size : seq_len;
        chunks.emplace_back(SequenceChunk{start, end, aiedit::Results()});
    }
    return chunks;
}

inline aiedit::Results merge_chunks(const std::vector<SequenceChunk>& chunks)
{
    aiedit::Results res;
    for (const auto& c : chunks) {
        res.edits.insert(res.edits.end(), c.results.edits.begin(), c.results.edits.end());
        res.ignored.insert(res.ignored.end(), c.results.ignored.begin(), c.results.ignored.end());
    }
    return res;
}

}

namespace aiedit {

AIEdit::AIEdit(const std::string& cbf_path,
               const std::string& hist_path,
               const std::string& seeds_path,
               const std::string& model_path,
               unsigned num_threads)
  : model(model_path, read_seeds(seeds_path))
  , cprobs(hist_path, cbf_path)
  , num_threads(num_threads)
{}

size_t AIEdit::get_cbf_size() const { return cprobs.cbf.get_bytes(); }

unsigned AIEdit::get_max_indels() const { return model.model.attr("max_indels").toInt(); }

unsigned AIEdit::get_max_k() const { return model.model.attr("max_k").toInt(); }

unsigned AIEdit::get_num_seeds() const { return model.model.attr("num_seeds").toInt(); }

Results AIEdit::get_edits(const std::string& seq)
{
    auto chunks = init_chunks(seq.size(), num_threads);
#pragma omp parallel for num_threads(chunks.size())
    for (auto& chunk : chunks) {
        chunk.results = process_chunk(seq, chunk.start, chunk.end);
    }
    return merge_chunks(chunks);
}

Results AIEdit::process_chunk(const std::string& seq, size_t start, size_t end)
{
    Results results;
    const auto kmer_size = model.seeds[0].size();
    const auto num_hashes = cprobs.cbf.get_hash_num();
    btllib::BlindNtHash hash_fn(seq, num_hashes, kmer_size, start);
    SequenceIterator seq_iter{seq, hash_fn, end, cprobs, 0.5};
    EditFinder edit_finder(cprobs);
    seq_iter.next_hit();
    while (seq_iter.next_miss()) {
        const size_t miss_pos = hash_fn.get_pos();
        if (!seq_iter.next_hit()) {
            break;
        }
        // TODO
        if (hash_fn.get_pos() - miss_pos > 1000) {
            continue;
        }
        auto pattern =
          model.get_pattern(seq, miss_pos, hash_fn.get_pos(), cprobs.cbf, cprobs.probs);
        if (pattern.size() > get_max_indels()) {
            continue;
        }
        auto found =
          edit_finder.get_edits(seq, miss_pos, kmer_size, num_hashes, pattern, results.edits);
        if (!found) {
            results.ignored.emplace_back(IgnoredPattern{miss_pos, pattern});
        }
    }
    return results;
}

}
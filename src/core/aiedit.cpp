#include "aiedit.hpp"

#include "internal/editor.hpp"
#include "internal/feature_extractor.hpp"
#include "internal/file_io.hpp"
#include "internal/sequence_chunk.hpp"

#include "extensions/delete_gap_hash.hpp"
#include "extensions/insert_gap_hash.hpp"
#include <torch/nn/functional/activation.h>

#include <algorithm>
#include <queue>
#include <stdexcept>

namespace {

bool next(aiedit::internal::SequenceChunk& seq_chunk,
          const btllib::CountingBloomFilter8& cbf,
          const std::vector<double>& probs,
          double threshold,
          bool hit)
{
    while (seq_chunk.roll()) {
        const auto count = cbf.contains(seq_chunk.hashes());
        const auto prob = probs[count];
        if ((prob >= threshold) == hit) {
            return true;
        }
    }
    return false;
}

int get_indels(const std::string& seq, size_t pos, const std::string& seed, unsigned num_hashes) {
    
}

}

namespace aiedit {

AIEdit::AIEdit(const std::string& cbf_path,
               const std::string& probs_path,
               const std::string& seeds_path,
               const std::string& model_path,
               unsigned max_edits)
  : cbf(cbf_path)
  , probs(internal::read_probs(probs_path))
  , seeds(internal::read_seeds(seeds_path))
  , editor(model_path)
  , max_edits(max_edits)
{}

std::vector<TrainingStep> AIEdit::train(const std::string& seq)
{
    std::vector<TrainingStep> steps;
    const auto kmer_size = seeds[0].size();
    const auto num_hashes = cbf.get_hash_num();
    internal::SequenceChunk seq_chunk(seq, 0, seq.size() - kmer_size, num_hashes, kmer_size);
    internal::FeatureExtractor features(cbf, probs, seeds, max_edits);
    next(seq_chunk, cbf, probs, 0.5, true);
    while (next(seq_chunk, cbf, probs, 0.5, false)) {
        const auto miss_pos = seq_chunk.get_pos();
        if (!next(seq_chunk, cbf, probs, 0.5, true)) {
            break;
        }
    }
    return steps;
}

std::vector<Edit> AIEdit::get_edits(const std::string& seq, size_t start, size_t end)
{
    std::vector<Edit> edits;
    const auto kmer_size = seeds[0].size();
    const auto num_hashes = cbf.get_hash_num();
    internal::SequenceChunk seq_chunk(seq, start, end, num_hashes, kmer_size);
    internal::FeatureExtractor features(cbf, probs, seeds, max_edits);
    next(seq_chunk, cbf, probs, 0.5, true);
    while (next(seq_chunk, cbf, probs, 0.5, false)) {
        const size_t miss_pos = seq_chunk.get_pos();
        if (!next(seq_chunk, cbf, probs, 0.5, true)) {
            break;
        }
        const auto x_probs = features.extract(seq, miss_pos, seq_chunk.get_pos());
    }
    return edits;
}

}
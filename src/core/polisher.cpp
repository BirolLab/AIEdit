#include "polisher.hpp"

#include "edit_region_finder.hpp"
#include "gap_filler.hpp"
#include "model_interface.hpp"

#include <ATen/ops/from_blob.h>
#include <atomic>
#include <btllib/nthash.hpp>
#include <c10/core/GradMode.h>
#include <torch/csrc/jit/serialization/import.h>

namespace {

at::Tensor seeds_to_tensor(const std::vector<std::string>& seeds)
{
    const long num_rows = seeds[0].size();
    const long num_cols = seeds.size();
    aiedit::Buffer2D buffer(num_rows, num_cols);
    for (long i = 0; i < seeds[0].size(); i++) {
        for (long j = 0; j < seeds.size(); j++) {
            buffer.set(i, j, seeds[j][i] == '1' ? 1.0 : 0.0);
        }
    }
    return at::from_blob(buffer.data(), {num_rows, num_cols}).clone();
}

std::vector<aiedit::Edit> align(const std::string_view ref, const std::string_view alt)
{
    std::vector<aiedit::Edit> edits;
    constexpr auto PASS = aiedit::Edit::Status::PASS;
    constexpr auto INSERT = aiedit::Edit::Type::INSERT;
    constexpr auto DELETE = aiedit::Edit::Type::DELETE;
    constexpr auto SUBSTITUTE = aiedit::Edit::Type::SUBSTITUTE;
    if (ref == alt) {
        return edits;
    } else if (ref.empty()) {
        aiedit::Edit edit{0, 0, INSERT, std::string(alt), 0, 0, 0, PASS};
        edits.emplace_back(edit);
        return edits;
    } else if (alt.empty()) {
        aiedit::Edit edit{0, 0, DELETE, std::string(ref), 0, 0, 0, PASS};
        edits.emplace_back(edit);
        return edits;
    }
    std::vector<std::vector<int>> dp(ref.size() + 1, std::vector<int>(alt.size() + 1));
    for (size_t i = 0; i <= ref.size() + 1; ++i) {
        dp[i][0] = i;
    }
    for (size_t j = 0; j <= alt.size(); ++j) {
        dp[0][j] = j;
    }
    for (size_t i = 1; i <= ref.size(); ++i) {
        for (size_t j = 1; j <= alt.size(); ++j) {
            if (ref[i - 1] == alt[j - 1])
                dp[i][j] = dp[i - 1][j - 1];
            else
                dp[i][j] = std::min({dp[i - 1][j - 1], dp[i][j - 1], dp[i - 1][j]}) + 1;
        }
    }
    size_t i = ref.size(), j = alt.size();
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0 && dp[i][j] == dp[i - 1][j - 1] + 1) {
            aiedit::Edit edit{i - 1, 0, SUBSTITUTE, std::string{alt[j - 1]}, 0, 0, 0, PASS};
            edits.emplace_back(edit);
            --i;
            --j;
        } else if (j > 0 && dp[i][j] == dp[i][j - 1] + 1) {
            aiedit::Edit edit{i, 0, INSERT, std::string{alt[j - 1]}, 0, 0, 0, PASS};
            edits.emplace_back(edit);
            --j;
        } else {
            aiedit::Edit edit{i - 1, 0, DELETE, "-", 0, 0, 0, PASS};
            edits.emplace_back(edit);
            --i;
        }
    }
    std::reverse(edits.begin(), edits.end());
    return edits;
}

}

namespace aiedit {

Polisher::Polisher(const std::string_view model_path,
                   const std::shared_ptr<KmerModel>& kmer_model,
                   unsigned num_threads,
                   float min_score,
                   unsigned num_tries)
  : kmer_model(kmer_model)
  , min_score(min_score)
  , num_tries(num_tries)
  , is_terminated(false)
{
    c10::NoGradGuard no_grad;
    try {
        model = torch::jit::load(model_path.data());
        model.eval();
    } catch (const c10::Error& e) {
        throw std::runtime_error("Failed to load model: " + std::string(e.what()));
    }
    auto x_seeds = seeds_to_tensor(kmer_model->get_seeds());
    h_seeds = model.run_method("encode_seeds", x_seeds).toTensor();
    for (unsigned i = 0; i < num_threads; i++) {
        threads.emplace_back(&Polisher::thread, this);
    }
}

Polisher::~Polisher()
{
    is_terminated = true;
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

unsigned Polisher::get_max_mismatches() const { return model.attr("max_mismatches").toInt(); }

unsigned Polisher::get_max_indels() const { return model.attr("max_indels").toInt(); }

std::shared_ptr<EditList> Polisher::polish(const std::string_view seq)
{
    auto results = std::make_shared<EditList>();
    if (seq.size() < kmer_model->get_kmer_size()) {
        return results;
    }
    EditRegionFinder edit_regions(seq, kmer_model, min_score);
    pending_tasks = 0;
    for (const auto region : edit_regions) {
        ++pending_tasks;
        tasks.push([&, seq, region]() {
            process_region(seq, region, results);
            --pending_tasks;
            if (pending_tasks == 0 && tasks.size() == 0) {
                std::lock_guard<std::mutex> lock(results_ready_mutex);
                results_ready.notify_one();
            }
        });
    }
    std::unique_lock<std::mutex> lock(results_ready_mutex);
    results_ready.wait(lock, [&]() { return pending_tasks == 0 && tasks.size() == 0; });
    results->sort();
    return results;
}

void Polisher::process_region(const std::string_view seq,
                              std::pair<size_t, size_t> region,
                              const std::shared_ptr<EditList>& results)
{
    Edit edit;
    const auto start = region.first;
    auto end = region.second;
    edit.position = start + kmer_model->get_kmer_size() - 1;
    edit.num_kmers = end - start;
    c10::NoGradGuard no_grad;
    ModelInterface interface(seq, start, end, get_max_mismatches(), get_max_indels(), kmer_model);
    const auto sig = interface.get_signature();
    auto x_sig = at::from_blob(sig.data(), {(long)sig.get_num_rows(), (long)sig.get_num_cols()});
    auto y_pred = model.run_method("predict", h_seeds, x_sig).toTensor();
    y_pred = y_pred.exp().div(y_pred.exp().sum());
    auto top_preds = y_pred.argsort(1, true);
    edit.status = Edit::Status::MODEL_FAIL;
    for (unsigned i_try = 0; i_try < num_tries; i_try++) {
        edit.i_try = i_try + 1;
        const auto i_edit = top_preds.data_ptr<int64_t>()[i_try];
        const auto result = interface.update(i_edit);
        edit.type = std::get<0>(result);
        edit.edited = std::get<1>(result);
        edit.kmer_score = std::get<2>(result);
        edit.model_confidence = y_pred.data_ptr<float>()[i_edit];
        if (!edit.edited.empty() && edit.kmer_score <= min_score) {
            edit.status = Edit::Status::LOW_KMER_SCORE;
        } else if (std::get<1>(result).empty()) {
            edit.status = Edit::Status::MODEL_FAIL;
        } else {
            edit.status = Edit::Status::PASS;
            break;
        }
    }
    if (edit.status == Edit::Status::PASS) {
        results->add(edit);
    } else if (region.second - region.first <= kmer_model->get_kmer_size() * kmer_model->get_kmer_size()) {
        GapFiller gap_filler(kmer_model, get_max_indels(), min_score, kmer_model->get_kmer_size());
        const auto filled = gap_filler.fill(seq, region.first, region.second);
        if (std::get<0>(filled) > min_score) {
            unsigned num_bases = 0;
            if (region.second - region.first > kmer_model->get_kmer_size()) {
                num_bases = region.second - region.first - kmer_model->get_kmer_size();
            }
            std::string_view ref = seq.substr(region.first, num_bases);
            edit.model_confidence = 0;
            for (auto edit : align(ref, std::get<1>(filled))) {
                edit.position += region.first + kmer_model->get_kmer_size();
                edit.num_kmers = region.second - region.first;
                edit.kmer_score = std::get<0>(filled);
                results->add(edit);
            }
        } else {
            results->add(edit);
        }
    }
}

void Polisher::thread()
{
    while (!is_terminated) {
        auto task = tasks.pop();
        if (task.has_value()) {
            task.value()();
        }
    }
}

}
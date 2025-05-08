#include "polisher.hpp"

#include "edit_region_finder.hpp"
#include "model_interface.hpp"

#include <ATen/ops/from_blob.h>
#include <atomic>
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

}

namespace aiedit {

Polisher::Polisher(const std::string_view model_path,
                   const std::shared_ptr<KmerModel>& kmer_model,
                   unsigned num_threads,
                   float min_score)
  : kmer_model(kmer_model)
  , min_score(min_score)
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
    EditRegionFinder erf(seq, kmer_model, min_score, get_max_mismatches());
    pending_tasks = 0;
    for (const auto region : erf) {
        ++pending_tasks;
        tasks.push([&, seq, region]() {
            auto edit = process_region(seq, region);
            results->add(edit);
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

Edit Polisher::process_region(const std::string_view seq, std::pair<size_t, size_t> region)
{
    c10::NoGradGuard no_grad;
    ModelInterface interface(seq, region.first, region.second, get_max_indels(), kmer_model);
    const auto sig = interface.get_signature();
    auto x_sig = at::from_blob(sig.data(), {(long)sig.get_num_rows(), (long)sig.get_num_cols()});
    auto y_pred = model.run_method("predict", h_seeds, x_sig);
    const auto& outputs = y_pred.toTuple();
    std::vector<float*> data_ptrs;
    std::vector<long> sizes;
    for (const auto& output : outputs->elements()) {
        const auto& output_tensor = output.toTensor().squeeze(0);
        data_ptrs.push_back(output_tensor.data_ptr<float>());
        sizes.push_back(static_cast<long>(output_tensor.size(0)));
    }
    const auto position = region.first + kmer_model->get_kmer_size() - 1;
    const auto result = interface.update(data_ptrs, sizes);
    const auto edit_type = std::get<0>(result);
    const auto edited = std::get<1>(result);
    const auto score = std::get<2>(result);
    Edit::Status status;
    if (!edited.empty() && score < min_score) {
        status = Edit::Status::LOW_KMER_SCORE;
    } else if (edited.empty()) {
        status = Edit::Status::MODEL_FAIL;
    } else {
        status = Edit::Status::PASS;
    }
    return Edit{position, edit_type, edited, score, status};
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
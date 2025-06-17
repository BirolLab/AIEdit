#pragma once

#include <ATen/Tensor.h>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <torch/jit.h>
#include <vector>

#include "edit_list.hpp"
#include "gap_filler.hpp"
#include "kmer_model.hpp"
#include "thread_safe_queue.hpp"

namespace aiedit {

class Polisher
{

  public:

    Polisher(const std::string_view model_path,
             const std::shared_ptr<KmerModel>& kmer_model,
             unsigned num_threads,
             float min_score,
             unsigned num_tries,
             unsigned max_gap);

    ~Polisher();

    std::shared_ptr<EditList> polish(const std::string_view seq);

    unsigned get_max_mismatches() const;
    unsigned get_max_indels() const;

  private:

    torch::jit::Module model;
    std::shared_ptr<KmerModel> kmer_model;
    const float min_score;
    const unsigned num_tries;
    at::Tensor h_seeds;

    std::vector<std::thread> threads;
    ThreadSafeQueue<std::function<void()>> tasks;
    std::atomic_size_t pending_tasks;
    std::condition_variable results_ready;
    std::mutex results_ready_mutex;
    bool is_terminated;

    const GapFiller gap_filler;

    void thread();
    void process_region(const std::string_view seq,
                        std::pair<size_t, size_t> region,
                        const std::shared_ptr<EditList>& results);
};

}
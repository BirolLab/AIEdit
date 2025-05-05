#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace aiedit {

template <typename T>
class ThreadSafeQueue
{

  public:

    ThreadSafeQueue() = default;

    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void push(const T& item)
    {
        std::lock_guard<std::mutex> lock(items_mutex);
        items.push_back(item);
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(items_mutex);
        if (items.empty()) {
            return {};
        }
        T front = std::move(items.front());
        items.pop_front();
        return front;
    }

    size_t size()
    {
        std::lock_guard<std::mutex> lock(items_mutex);
        return items.size();
    }

  protected:

    std::deque<T> items;
    std::mutex items_mutex;
};

}
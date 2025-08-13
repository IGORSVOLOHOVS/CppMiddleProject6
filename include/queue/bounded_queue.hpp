#pragma once
#include "queue/queue.hpp"
#include <mutex>
#include <queue>
#include <condition_variable>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
    std::queue<Task> queue_;
    std::mutex mutex_;
    std::condition_variable not_full_cv_;
    std::condition_variable not_empty_cv_;
    const unsigned capacity_;
public:
    explicit BoundedQueue(unsigned capacity);

    void push(Task task) override;

    std::optional<Task> try_pop() override;

    ~BoundedQueue() override = default;
};

}  // namespace dispatcher::queue
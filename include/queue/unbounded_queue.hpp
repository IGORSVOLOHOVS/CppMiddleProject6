#pragma once
#include "queue.hpp"
#include <condition_variable>
#include <mutex>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    std::queue<Task> queue_;
    std::mutex mutex_;
    std::condition_variable not_empty_cv_;

public:
    UnboundedQueue() = default;
    void push(Task task) override;
    std::optional<Task> try_pop() override;
    ~UnboundedQueue() override = default;
};

}  // namespace dispatcher::queue

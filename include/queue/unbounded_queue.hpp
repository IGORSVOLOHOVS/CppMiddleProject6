#pragma once
#include "queue/queue.hpp"
#include <condition_variable>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    std::queue<Task> queue_;
    std::mutex mutex_;
    std::condition_variable not_empty_cv_;
public:
    explicit UnboundedQueue();

    void push(Task task) override;

    std::optional<Task> try_pop() override;

    ~UnboundedQueue() override = default;
};

}  // namespace dispatcher::queue
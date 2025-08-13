#pragma once
#include "queue/queue.hpp"
#include <condition_variable>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    std::queue<Task> queue_{};
    std::condition_variable cv_{};
    std::mutex m_{};
public:
    explicit UnboundedQueue();

    void push(Task task) override;

    std::optional<Task> try_pop() override;

    ~UnboundedQueue() override;
};

}  // namespace dispatcher::queue
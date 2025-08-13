#pragma once
#include "queue/queue.hpp"
#include <mutex>
#include <queue>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
    std::queue<Task> queue_{};
    std::condition_variable cv_;
    std::mutex m_{};
    unsigned capacity_;
public:
    explicit BoundedQueue(unsigned capacity);

    void push(Task task) override;

    std::optional<Task> try_pop() override;

    ~BoundedQueue() override;
};

}  // namespace dispatcher::queue
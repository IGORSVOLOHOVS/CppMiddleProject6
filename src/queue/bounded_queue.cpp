#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {

BoundedQueue::BoundedQueue(unsigned capacity) : capacity_(capacity) {}

void BoundedQueue::push(Task task) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_cv_.wait(lock, [this] { return queue_.size() < capacity_; });
    queue_.push(std::move(task));
    not_empty_cv_.notify_one();
}

std::optional<Task> BoundedQueue::try_pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return std::nullopt;
    }
    auto task = std::move(queue_.front());
    queue_.pop();
    not_full_cv_.notify_one();
    return task;
}

}  // namespace dispatcher::queue

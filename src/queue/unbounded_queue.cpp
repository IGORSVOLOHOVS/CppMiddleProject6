#include "queue/unbounded_queue.hpp"

namespace dispatcher::queue {

void UnboundedQueue::push(Task task) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(std::move(task));
    not_empty_cv_.notify_one();
}

std::optional<Task> UnboundedQueue::try_pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return std::nullopt;
    }
    auto task = std::move(queue_.front());
    queue_.pop();
    return task;
}

}  // namespace dispatcher::queue

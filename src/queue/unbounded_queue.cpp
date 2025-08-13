#include "queue/unbounded_queue.hpp"

#include <functional>
#include <mutex>
#include <queue>
#include <semaphore>

namespace dispatcher::queue {

// здесь ваш код
UnboundedQueue::UnboundedQueue(){}

void UnboundedQueue::push(Task task) {
    std::lock_guard lg{m_};
    queue_.push(std::move(task));
    cv_.notify_all();
}

std::optional<Task> UnboundedQueue::try_pop() {
    std::lock_guard lg{m_};
    if(!queue_.empty()){
        Task task = queue_.front();
        queue_.pop();
        return task;
    }
    return std::nullopt;
}

UnboundedQueue::~UnboundedQueue() {}

} // namespace dispatcher::queue
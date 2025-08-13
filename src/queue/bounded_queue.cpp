#include "queue/bounded_queue.hpp"
#include <mutex>
#include <optional>
#include <stdexcept>

namespace dispatcher::queue {

// здесь ваш код
BoundedQueue::BoundedQueue(unsigned capacity): capacity_(capacity){
    if (capacity == 0) 
    {
        throw std::invalid_argument("Capacity must be greater than zero.");
    }
}

void BoundedQueue::push(Task task) {
    std::unique_lock ul{m_};
    cv_.wait(ul, [&](){ return queue_.size() < capacity_; });
    queue_.push(std::move(task));
}

std::optional<Task> BoundedQueue::try_pop() {
    std::lock_guard lg{m_};
    if(!queue_.empty()){
        Task task = queue_.front();
        queue_.pop();
        cv_.notify_all();
        return task;
    }
    return std::nullopt;
}

BoundedQueue::~BoundedQueue() {}


} // namespace dispatcher::queue
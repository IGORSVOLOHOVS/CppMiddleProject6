#pragma once
#include "queue.hpp"
#include "types.hpp"

#include <atomic>
#include <condition_variable>
#include <flat_map>
#include <memory>
#include <mutex>
#include <optional>

namespace dispatcher::queue {

class PriorityQueue {
    std::flat_map<TaskPriority, std::unique_ptr<IQueue>> queues_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> is_shutdown_{false};

public:
    explicit PriorityQueue(const std::flat_map<TaskPriority, QueueOptions> &options);
    void push(TaskPriority priority, Task task);
    std::optional<Task> pop();
    void shutdown();
    ~PriorityQueue() = default;
};

}  // namespace dispatcher::queue

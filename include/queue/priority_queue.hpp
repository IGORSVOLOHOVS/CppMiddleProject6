#pragma once
#include "queue.hpp"
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <atomic>
#include <cstdint>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <condition_variable>
#include <flat_map>

namespace dispatcher::queue {

class PriorityQueue {
    std::flat_map<TaskPriority, std::unique_ptr<IQueue>> queues_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> is_shutdown_{false};
public:
    explicit PriorityQueue(const std::flat_map<TaskPriority, QueueOptions>& options);

    void push(TaskPriority priority, Task task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<Task> pop();

    void shutdown();

    ~PriorityQueue() = default;
};

}  // namespace dispatcher::queue
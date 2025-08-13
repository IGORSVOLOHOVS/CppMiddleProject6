#pragma once
#include "queue.hpp"
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <atomic>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace dispatcher::queue {

class PriorityQueue {
    std::flat_map<TaskPriority, std::unique_ptr<IQueue>> tasks_queue_;
    std::atomic<bool> stop_flag{false};
    std::condition_variable not_empty_cv_{};
    std::condition_variable not_full_cv_{};
    std::mutex m_{};
public:
    explicit PriorityQueue(std::flat_map<TaskPriority, QueueOptions> tasks_options);

    void push(TaskPriority priority, Task task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<Task> pop();

    void shutdown();

    ~PriorityQueue();
};

}  // namespace dispatcher::queue
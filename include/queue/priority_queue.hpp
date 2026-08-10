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
// <flat_map> — часть C++23, но в MSVC STL 14.44 этого заголовка ещё нет:
// libstdc++ 15 его уже отдаёт, cl.exe падает на "cannot open source file".
// Сама очередь работает на std::map, так что заголовок нужен только тем, кто
// захочет переключиться; поэтому подключаем его условно, а не убираем.
#if defined(__has_include)
#  if __has_include(<flat_map>)
#    include <flat_map>
#  endif
#endif

namespace dispatcher::queue {

class PriorityQueue {
    std::map<TaskPriority, std::unique_ptr<IQueue>> queues_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> is_shutdown_{false};
public:
    explicit PriorityQueue(const std::map<TaskPriority, QueueOptions>& options);

    void push(TaskPriority priority, Task task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<Task> pop();

    void shutdown();

    ~PriorityQueue() = default;
};

}  // namespace dispatcher::queue
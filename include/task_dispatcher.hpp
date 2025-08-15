#pragma once

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"
#include <flat_map>
#include <memory>

namespace dispatcher {

class TaskDispatcher {
    std::shared_ptr<queue::PriorityQueue> queue_;
    thread_pool::ThreadPool pool_;

public:
    explicit TaskDispatcher(size_t thread_count,
                            const std::flat_map<TaskPriority, queue::QueueOptions> &tasks_options = {
                                {TaskPriority::High, {true, 1000}}, 
                                {TaskPriority::Normal, {false, std::nullopt}}});

    void schedule(TaskPriority priority, std::function<void()> task);
    ~TaskDispatcher();
};

}  // namespace dispatcher

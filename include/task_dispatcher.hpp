#pragma once

#include <memory>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
public:
    explicit TaskDispatcher(size_t thread_count,
                            const std::flat_map<TaskPriority, queue::QueueOptions>&
                            tasks_options = {
                                {TaskPriority::High, {true, 1000}},
                                {TaskPriority::Normal, {false, std::nullopt}}
                            });

    void schedule(TaskPriority priority, queue::Task task);
    ~TaskDispatcher();

private:
    std::shared_ptr<queue::PriorityQueue> queue_;
    thread_pool::ThreadPool pool_;
};

}  // namespace dispatcher
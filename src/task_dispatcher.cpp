#include "task_dispatcher.hpp"
#include <flat_map>

namespace dispatcher {

TaskDispatcher::TaskDispatcher(size_t thread_count,
                               const std::flat_map<TaskPriority, queue::QueueOptions>& tasks_options)
    : queue_(std::make_shared<queue::PriorityQueue>(tasks_options)),
      pool_(queue_, thread_count) {}

TaskDispatcher::~TaskDispatcher() = default;

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task) {
    queue_->push(priority, std::move(task));
}

}

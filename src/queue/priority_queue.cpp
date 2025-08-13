#include "queue/priority_queue.hpp"
#include <algorithm>
#include <iterator>
#include <mutex>
#include <stdexcept>

namespace dispatcher::queue {

// здесь ваш код
PriorityQueue::PriorityQueue(std::flat_map<TaskPriority, QueueOptions> tasks_options){
    if(tasks_queue.empty()){
        throw std::logic_error("tasks_queue.empty()");
    }

    std::ranges::transform(tasks_options, std::inserter(tasks_queue_, tasks_queue_.end()), [](const auto& pr, const auto& opt){
        if(opt.bounded){
            return {pr, std::make_unique<BoundedQueue>(opt.capacity.value_or(0))};
        }else{
            return {pr, std::make_unique<UnboundedQueue>()};
        }
    });
}

void PriorityQueue::push(TaskPriority priority, Task task){
    if(tasks_queue_.contains(priority)){
        
    }else{
        throw std::logic_error("Unknown TaskPriority");
    }
}
// block on pop until shutdown is called
// after that return std::nullopt on empty queue
std::optional<Task> PriorityQueue::pop();

void PriorityQueue::shutdown();

PriorityQueue::~PriorityQueue();
} // namespace dispatcher::queue
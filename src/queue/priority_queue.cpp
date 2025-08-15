#include "queue/priority_queue.hpp"

namespace dispatcher::queue {

// Конструктор и shutdown остаются без изменений

PriorityQueue::PriorityQueue(const std::map<TaskPriority, QueueOptions>& options) {
    for (const auto& [priority, queue_options] : options) {
        if (queue_options.bounded) {
            // Убедитесь, что Task - это Task
            queues_[priority] = std::make_unique<BoundedQueue>(queue_options.capacity.value_or(1024));
        } else {
            queues_[priority] = std::make_unique<UnboundedQueue>();
        }
    }
}

void PriorityQueue::push(TaskPriority priority, Task task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queues_.count(priority)) {
            queues_.at(priority)->push(std::move(task));
        }
    }
    cv_.notify_one();
}

void PriorityQueue::shutdown() {
    is_shutdown_ = true;
    cv_.notify_all();
}


// --- ИСПРАВЛЕННЫЙ МЕТОД POP ---
std::optional<Task> PriorityQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    while (true) {
        // Сначала пытаемся извлечь задачу, начиная с наивысшего приоритета
        for (auto const& [priority, queue] : queues_) {
            if (auto task = queue->try_pop()) {
                return task; // Задача найдена, немедленно возвращаем
            }
        }

        // Если мы здесь, значит, задач в очередях нет.
        // Проверяем, не пора ли завершать работу.
        if (is_shutdown_) {
            return std::nullopt; // Очередь закрыта и пуста
        }

        // Если не завершаем работу, ждем сигнала о поступлении новой задачи.
        // Когда cv_.wait() проснётся, цикл while(true) начнётся заново и проверит все очереди.
        cv_.wait(lock);
    }
}

} // namespace dispatcher::queue
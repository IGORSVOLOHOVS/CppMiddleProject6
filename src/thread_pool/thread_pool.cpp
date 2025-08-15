#include "thread_pool/thread_pool.hpp"

#include <functional>
#include <utility>
#include <iostream>
#include "logger.hpp"

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t thread_count)
    : queue_(std::move(queue)) {
    threads_.reserve(thread_count);

    std::ranges::for_each(std::views::iota(0U, thread_count), [this](auto&&) {
        threads_.emplace_back(&ThreadPool::Worker, this);
    });
}

ThreadPool::~ThreadPool() {
    queue_->shutdown();
}

void ThreadPool::Worker() {
    while (auto task = queue_->pop()) {
        try {
            std::invoke(task.value());
        } catch (const std::exception& e) {
            Logger::Get().Log("Task threw an exception: " + std::string(e.what()));
        } catch (...) {
            Logger::Get().Log("Task threw an unknown exception.");
        }
    }
}

} // namespace dispatcher::thread_pool
#include "thread_pool/thread_pool.hpp"

#include <utility>
#include <iostream>

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t thread_count)
    : queue_(std::move(queue)) {
    threads_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        threads_.emplace_back(&ThreadPool::Worker, this);
    }
}

ThreadPool::~ThreadPool() {
    queue_->shutdown();
}

void ThreadPool::Worker() {
    while (true) {
        if (auto task = queue_->pop()) {
            try {
                (*task)();
            } catch (const std::exception& e) {
                std::cerr << "Task threw an exception: " << e.what() << '\n';
            } catch (...) {
                std::cerr << "Task threw an unknown exception.\n";
            }
        } else {
            break;
        }
    }
}

} // namespace dispatcher::thread_pool
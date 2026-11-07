#ifndef DISPATCHER_THREAD_POOL_THREAD_POOL_HPP
#define DISPATCHER_THREAD_POOL_THREAD_POOL_HPP
#include "queue/priority_queue.hpp"
#include <memory>
#include <thread>

namespace dispatcher::thread_pool {

class ThreadPool {
    // здесь ваш код
    std::vector<std::jthread> threads_;
    std::shared_ptr<queue::PriorityQueue> queue_;

public:
    ThreadPool(std::shared_ptr<queue::PriorityQueue> queue, size_t thread_count);
    ~ThreadPool();

private:
    void Worker();
};

}  // namespace dispatcher::thread_pool

#endif  // DISPATCHER_THREAD_POOL_THREAD_POOL_HPP

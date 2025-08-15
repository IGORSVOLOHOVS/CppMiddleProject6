#include <gtest/gtest.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <ranges>
#include <chrono>

#include "task_dispatcher.hpp"

using namespace std::chrono_literals;

TEST(TaskDispatcherTest, ExecutesTasks) {
    const size_t high_priority_tasks = 10;
    const size_t normal_priority_tasks = 10;
    const size_t total_tasks = high_priority_tasks + normal_priority_tasks;

    std::atomic<size_t> task_counter = 0;
    std::mutex m;
    std::condition_variable cv;

    {
        dispatcher::TaskDispatcher td(4);
        
        auto schedule_and_notify = [&](dispatcher::TaskPriority p) {
            td.schedule(p, [&]() {
                task_counter.fetch_add(1, std::memory_order_relaxed);
                cv.notify_one();
            });
        };

        std::ranges::for_each(std::views::iota(0u, high_priority_tasks),
            [&](auto){ schedule_and_notify(dispatcher::TaskPriority::High); });
            
        std::ranges::for_each(std::views::iota(0u, normal_priority_tasks),
            [&](auto){ schedule_and_notify(dispatcher::TaskPriority::Normal); });
            
        std::unique_lock<std::mutex> lock(m);
        cv.wait_for(lock, 1s, [&]() { return task_counter == total_tasks; });
    }

    ASSERT_EQ(task_counter, total_tasks);
}

TEST(TaskDispatcherTest, HighPriorityTasksFirst) {
    const size_t high_priority_tasks = 5;
    const size_t normal_priority_tasks = 5;
    const size_t total_tasks = high_priority_tasks + normal_priority_tasks;

    std::vector<dispatcher::TaskPriority> execution_order;
    std::mutex m;
    std::condition_variable cv;

    {
        dispatcher::TaskDispatcher td(2);
        
        auto schedule_and_log = [&](dispatcher::TaskPriority p) {
             td.schedule(p, [&, p]() {
                std::lock_guard<std::mutex> lock(m);
                execution_order.push_back(p);
                cv.notify_one();
            });
        };
        
        std::ranges::for_each(std::views::iota(0u, normal_priority_tasks),
            [&](auto){ schedule_and_log(dispatcher::TaskPriority::Normal); });
            
        std::ranges::for_each(std::views::iota(0u, high_priority_tasks),
            [&](auto){ schedule_and_log(dispatcher::TaskPriority::High); });

        std::unique_lock<std::mutex> lock(m);
        cv.wait_for(lock, 1s, [&]() { return execution_order.size() == total_tasks; });
    }
    
    auto high_priority_part = execution_order | std::views::take(high_priority_tasks);
    ASSERT_TRUE(std::ranges::all_of(high_priority_part, [](auto p){ return p == dispatcher::TaskPriority::High; }));

    auto normal_priority_part = execution_order | std::views::drop(high_priority_tasks);
    ASSERT_TRUE(std::ranges::all_of(normal_priority_part, [](auto p){ return p == dispatcher::TaskPriority::Normal; }));
}

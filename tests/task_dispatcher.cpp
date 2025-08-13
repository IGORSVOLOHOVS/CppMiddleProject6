#include <gtest/gtest.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>

#include "task_dispatcher.hpp"

TEST(TaskDispatcherTest, ExecutesTasks) {
    const size_t high_priority_tasks = 10;
    const size_t normal_priority_tasks = 10;
    const size_t total_tasks = high_priority_tasks + normal_priority_tasks;

    std::atomic<size_t> task_counter = 0;
    std::mutex m;
    std::condition_variable cv;

    {
        dispatcher::TaskDispatcher td(4);

        for (size_t i = 0; i < high_priority_tasks; ++i) {
            td.schedule(dispatcher::TaskPriority::High, [&]() {
                task_counter++;
                cv.notify_one();
            });
        }

        for (size_t i = 0; i < normal_priority_tasks; ++i) {
            td.schedule(dispatcher::TaskPriority::Normal, [&]() {
                task_counter++;
                cv.notify_one();
            });
        }

        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&]() { return task_counter == total_tasks; });
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

        for (size_t i = 0; i < normal_priority_tasks; ++i) {
            td.schedule(dispatcher::TaskPriority::Normal, [&, i]() {
                std::lock_guard<std::mutex> lock(m);
                execution_order.push_back(dispatcher::TaskPriority::Normal);
                cv.notify_one();
            });
        }

        for (size_t i = 0; i < high_priority_tasks; ++i) {
            td.schedule(dispatcher::TaskPriority::High, [&, i]() {
                std::lock_guard<std::mutex> lock(m);
                execution_order.push_back(dispatcher::TaskPriority::High);
                cv.notify_one();
            });
        }

        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&]() { return execution_order.size() == total_tasks; });
    }

    for (size_t i = 0; i < high_priority_tasks; ++i) {
        ASSERT_EQ(execution_order[i], dispatcher::TaskPriority::High);
    }

    for (size_t i = high_priority_tasks; i < total_tasks; ++i) {
        ASSERT_EQ(execution_order[i], dispatcher::TaskPriority::Normal);
    }
}
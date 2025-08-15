#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <algorithm>
#include <ranges>
#include <flat_map>

#include "queue/priority_queue.hpp"

using namespace dispatcher;
using namespace dispatcher::queue;
using namespace std::chrono_literals;

TEST(PriorityQueueTest, HighPriorityFirst) {
    std::flat_map<TaskPriority, QueueOptions> options = {
        {TaskPriority::High, {false, std::nullopt}},
        {TaskPriority::Normal, {false, std::nullopt}}
    };
    PriorityQueue pq(options);

    std::vector<TaskPriority> execution_order;

    pq.push(TaskPriority::Normal, [&] { execution_order.push_back(TaskPriority::Normal); });
    pq.push(TaskPriority::High, [&] { execution_order.push_back(TaskPriority::High); });
    
    std::invoke(pq.pop().value());
    std::invoke(pq.pop().value());

    ASSERT_EQ(execution_order.size(), 2);
    ASSERT_EQ(execution_order[0], TaskPriority::High);
    ASSERT_EQ(execution_order[1], TaskPriority::Normal);
}

TEST(PriorityQueueTest, PopBlocksOnEmpty) {
    std::flat_map<TaskPriority, QueueOptions> options = {{TaskPriority::Normal, {false, std::nullopt}}};
    PriorityQueue pq(options);
    std::atomic<bool> task_popped = false;

    std::jthread t([&]() {
        if (auto task = pq.pop()) {
            std::invoke(task.value());
            task_popped = true;
        }
    });

    std::this_thread::sleep_for(100ms);
    ASSERT_FALSE(task_popped);

    pq.push(TaskPriority::Normal, [] {});
}

TEST(PriorityQueueTest, Shutdown) {
    std::flat_map<TaskPriority, QueueOptions> options = {{TaskPriority::High, {false, std::nullopt}}};
    PriorityQueue pq(options);
    std::atomic<bool> thread_unblocked = false;

    std::jthread t([&]() {
        auto task = pq.pop();
        ASSERT_FALSE(task.has_value());
        thread_unblocked = true;
    });

    std::this_thread::sleep_for(50ms);
    ASSERT_FALSE(thread_unblocked);

    pq.shutdown();
    t.join();
    
    ASSERT_TRUE(thread_unblocked);
    ASSERT_FALSE(pq.pop().has_value());
}

TEST(PriorityQueueTest, ShutdownDrainsTasks) {
    std::flat_map<TaskPriority, QueueOptions> options = {{TaskPriority::Normal, {false, std::nullopt}}};
    PriorityQueue pq(options);
    
    std::ranges::for_each(std::views::iota(0, 2), 
        [&](int){ pq.push(TaskPriority::Normal, [] {}); });

    pq.shutdown();

    ASSERT_TRUE(pq.pop().has_value());
    ASSERT_TRUE(pq.pop().has_value());
    ASSERT_FALSE(pq.pop().has_value());
}

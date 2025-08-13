#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

#include "queue/priority_queue.hpp"

using namespace dispatcher;
using namespace dispatcher::queue;

TEST(PriorityQueueTest, HighPriorityFirst) {
    std::map<TaskPriority, QueueOptions> options = {
        {TaskPriority::High, {false, std::nullopt}},
        {TaskPriority::Normal, {false, std::nullopt}}
    };
    PriorityQueue pq(options);

    std::vector<TaskPriority> execution_order;

    pq.push(TaskPriority::Normal, [&] { execution_order.push_back(TaskPriority::Normal); });
    pq.push(TaskPriority::High, [&] { execution_order.push_back(TaskPriority::High); });
    
    (*pq.pop())();
    (*pq.pop())();

    ASSERT_EQ(execution_order.size(), 2);
    ASSERT_EQ(execution_order[0], TaskPriority::High);
    ASSERT_EQ(execution_order[1], TaskPriority::Normal);
}

TEST(PriorityQueueTest, PopBlocksOnEmpty) {
    PriorityQueue pq({{TaskPriority::Normal, {false, std::nullopt}}});
    std::atomic<bool> task_popped = false;

    std::thread t([&]() {
        auto task = pq.pop(); 
        if (task) {
            (*task)();
            task_popped = true;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(task_popped);

    pq.push(TaskPriority::Normal, [] {});
    t.join();
    ASSERT_TRUE(task_popped);
}

TEST(PriorityQueueTest, Shutdown) {
    PriorityQueue pq({{TaskPriority::High, {false, std::nullopt}}});
    std::atomic<bool> thread_unblocked = false;

    std::thread t([&]() {
        auto task = pq.pop(); 
        ASSERT_FALSE(task.has_value()); 
        thread_unblocked = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ASSERT_FALSE(thread_unblocked); 

    pq.shutdown();
    t.join();
    ASSERT_TRUE(thread_unblocked); 
    ASSERT_FALSE(pq.pop().has_value());
}

TEST(PriorityQueueTest, ShutdownDrainsTasks) {
    PriorityQueue pq({{TaskPriority::Normal, {false, std::nullopt}}});
    pq.push(TaskPriority::Normal, [] {});
    pq.push(TaskPriority::Normal, [] {});

    pq.shutdown();

    ASSERT_TRUE(pq.pop().has_value());
    ASSERT_TRUE(pq.pop().has_value());
    ASSERT_FALSE(pq.pop().has_value());
}
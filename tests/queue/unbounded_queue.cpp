#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <ranges>
#include <chrono>

#include "queue/unbounded_queue.hpp"

using namespace dispatcher::queue;

class UnboundedQueueTest : public ::testing::Test {
protected:
    UnboundedQueue queue;
};

TEST_F(UnboundedQueueTest, PushAndPopWorksInSingleThread) {
    bool task_executed = false;
    Task my_task = [&](){ task_executed = true; };
    
    queue.push(my_task);
    
    auto popped_task = queue.try_pop();
    ASSERT_TRUE(popped_task.has_value());
    
    popped_task.value()();
    ASSERT_TRUE(task_executed);
}

TEST_F(UnboundedQueueTest, TryPopOnEmptyQueueReturnsNullopt) {
    ASSERT_FALSE(queue.try_pop().has_value());
}

TEST_F(UnboundedQueueTest, MultipleProducersRaceCondition) {
    const int num_threads = 4;
    const int tasks_per_thread = 1000;
    std::atomic<int> tasks_executed_count = 0;
    
    std::vector<std::jthread> producers;
    for (int i = 0; i < num_threads; ++i) {
        producers.emplace_back([&]() {
            for (int j = 0; j < tasks_per_thread; ++j) {
                queue.push([&](){ tasks_executed_count++; });
            }
        });
    }

    for (auto& thread : producers) {
        thread.join();
    }

    int popped_count = 0;
    while (auto task = queue.try_pop()) {
        task.value()();
        popped_count++;
    }
    
    ASSERT_EQ(popped_count, num_threads * tasks_per_thread);
    ASSERT_EQ(tasks_executed_count.load(), num_threads * tasks_per_thread);
}

TEST_F(UnboundedQueueTest, MultipleConsumersRaceCondition) {
    const int num_tasks = 10000;
    for (int i = 0; i < num_tasks; ++i) {
        queue.push([](){});
    }

    const int num_threads = 4;
    std::atomic<int> tasks_popped_count = 0;
    std::vector<std::jthread> consumers;

    for (int i = 0; i < num_threads; ++i) {
        consumers.emplace_back([&]() {
            while (queue.try_pop().has_value()) {
                tasks_popped_count++;
            }
        });
    }

    for (auto& thread : consumers) {
        thread.join();
    }
    
    ASSERT_EQ(tasks_popped_count.load(), num_tasks);
    ASSERT_FALSE(queue.try_pop().has_value());
}
// test BoundedQueue
    // test min/max bound
// test push
    // test empty
    // test non-empty
    // test more then bounded
// test try_pop
    // test when exists
    // test when not exists
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <ranges>
#include <chrono>
#include <algorithm>
#include <functional>

#include "queue/bounded_queue.hpp"

using namespace dispatcher::queue;
using namespace std::chrono_literals;

TEST(BoundedQueueTest, PushAndPop) {
    BoundedQueue bq(5);
    bq.push([] {});
    auto task = bq.try_pop();
    ASSERT_TRUE(task.has_value());
    ASSERT_FALSE(bq.try_pop().has_value());
}

TEST(BoundedQueueTest, RespectsCapacity) {
    const int capacity = 3;
    BoundedQueue bq(capacity);

    std::ranges::for_each(std::views::iota(0, capacity),
                          [&](int) { bq.push([] {}); });

    {
        std::jthread pusher([&]() {
            bq.push([] {});
        });

        std::this_thread::sleep_for(100ms);

        auto task = bq.try_pop();
        ASSERT_TRUE(task.has_value());
    }

    std::ranges::for_each(std::views::iota(0, capacity), [&](int) {
        ASSERT_TRUE(bq.try_pop().has_value());
    });
    ASSERT_FALSE(bq.try_pop().has_value());
}

TEST(BoundedQueueTest, MultiThreaded) {
    BoundedQueue bq(100);
    const int items_per_thread = 50;
    std::atomic<int> tasks_executed = 0;

    auto pusher_func = [&]() {
        std::ranges::for_each(std::views::iota(0, items_per_thread),
            [&](int) {
                bq.push([&] { tasks_executed.fetch_add(1, std::memory_order_relaxed); });
            });
    };

    auto popper_func = [&]() {
        std::ranges::for_each(std::views::iota(0, items_per_thread),
            [&](int) {
                while (true) {
                    if (auto task = bq.try_pop()) {
                        std::invoke(*task);
                        break;
                    }
                    std::this_thread::yield();
                }
            });
    };

    {
        std::vector<std::jthread> threads;
        threads.reserve(4);
        
        threads.emplace_back(pusher_func);
        threads.emplace_back(pusher_func);
        threads.emplace_back(popper_func);
        threads.emplace_back(popper_func);
    }

    ASSERT_EQ(tasks_executed, 2 * items_per_thread);
    ASSERT_FALSE(bq.try_pop().has_value());
}

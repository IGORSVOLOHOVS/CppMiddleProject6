#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <ranges>
#include <algorithm>
#include <functional>

#include "queue/unbounded_queue.hpp"

using namespace dispatcher::queue;

TEST(UnboundedQueueTest, PushAndPop) {
    UnboundedQueue uq;
    uq.push([] {});
    auto task = uq.try_pop();
    ASSERT_TRUE(task.has_value());
    ASSERT_FALSE(uq.try_pop().has_value());
}

TEST(UnboundedQueueTest, HandlesManyItems) {
    UnboundedQueue uq;
    const int item_count = 10000;
    std::ranges::for_each(std::views::iota(0, item_count),
                          [&](int) { uq.push([] {}); });

    std::ranges::for_each(std::views::iota(0, item_count),
                          [&](int) { ASSERT_TRUE(uq.try_pop().has_value()); });

    ASSERT_FALSE(uq.try_pop().has_value());
}

TEST(UnboundedQueueTest, MultiThreaded) {
    UnboundedQueue uq;
    const int items_per_thread = 500;
    std::atomic<int> items_popped = 0;
    const int num_threads = 4;

    auto pusher_func = [&]() {
        std::ranges::for_each(std::views::iota(0, items_per_thread),
                              [&](int) { uq.push([] {}); });
    };

    auto popper_func = [&]() {
        std::ranges::for_each(std::views::iota(0, items_per_thread),
            [&](int) {
                while (!uq.try_pop()) {
                    std::this_thread::yield();
                }
                items_popped.fetch_add(1, std::memory_order_relaxed);
            });
    };
    
    {
        std::vector<std::jthread> threads;
        threads.reserve(num_threads * 2);
        std::ranges::for_each(std::views::iota(0, num_threads),
                              [&](int) { threads.emplace_back(pusher_func); });
        std::ranges::for_each(std::views::iota(0, num_threads),
                              [&](int) { threads.emplace_back(popper_func); });
    }

    ASSERT_EQ(items_popped, num_threads * items_per_thread);
    ASSERT_FALSE(uq.try_pop().has_value());
}

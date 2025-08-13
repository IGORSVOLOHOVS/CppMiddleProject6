#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>

#include "queue/unbounded_queue.hpp"

using namespace dispatcher::queue;

TEST(UnboundedQueueTest, PushAndPop) {
    UnboundedQueue uq{};
    uq.push([] {});
    auto task = uq.try_pop();
    ASSERT_TRUE(task.has_value());
    ASSERT_FALSE(uq.try_pop().has_value());
}

TEST(UnboundedQueueTest, HandlesManyItems) {
    UnboundedQueue uq{};
    const int item_count = 10000;
    for (int i = 0; i < item_count; ++i) {
        uq.push([] {});
    }
    for (int i = 0; i < item_count; ++i) {
        ASSERT_TRUE(uq.try_pop().has_value());
    }
    ASSERT_FALSE(uq.try_pop().has_value());
}

TEST(UnboundedQueueTest, MultiThreaded) {
    UnboundedQueue uq{};
    const int items_per_thread = 500;
    std::atomic<int> items_popped = 0;

    std::vector<std::thread> pushers;
    for(int i = 0; i < 4; ++i) {
        pushers.emplace_back([&]() {
            for (int j = 0; j < items_per_thread; ++j) {
                uq.push([] {});
            }
        });
    }

    std::vector<std::thread> poppers;
     for(int i = 0; i < 4; ++i) {
        poppers.emplace_back([&]() {
            for (int j = 0; j < items_per_thread; ++j) {
                while(!uq.try_pop()) {
                    std::this_thread::yield();
                }
                items_popped++;
            }
        });
    }

    for(auto& t : pushers) t.join();
    for(auto& t : poppers) t.join();

    ASSERT_EQ(items_popped, 4 * items_per_thread);
    ASSERT_FALSE(uq.try_pop().has_value());
}
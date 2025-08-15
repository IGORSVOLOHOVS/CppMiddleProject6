#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>

#include "queue/bounded_queue.hpp"

using namespace dispatcher::queue;

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

    for (int i = 0; i < capacity; ++i) {
        bq.push([] {});
    }

    std::thread pusher([&]() {
        bq.push([] {}); 
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto task = bq.try_pop();
    ASSERT_TRUE(task.has_value());

    pusher.join(); 

    for (int i = 0; i < capacity; ++i) {
        ASSERT_TRUE(bq.try_pop().has_value());
    }
    ASSERT_FALSE(bq.try_pop().has_value());
}

TEST(BoundedQueueTest, MultiThreaded) {
    BoundedQueue bq(100);
    const int items_per_thread = 50;
    std::atomic<int> items_pushed = 0;
    std::atomic<int> items_popped = 0;

    auto pusher_func = [&]() {
        for (int i = 0; i < items_per_thread; ++i) {
            bq.push([&items_pushed] { items_pushed++; });
        }
    };

    auto popper_func = [&]() {
        for (int i = 0; i < items_per_thread; ++i) {
            auto task = bq.try_pop();
            if(task) {
                (*task)();
                items_popped++;
            }
        }
    };

    std::thread p1(pusher_func);
    std::thread p2(pusher_func);
    std::thread c1(popper_func);
    std::thread c2(popper_func);

    p1.join();
    p2.join();
    c1.join();
    c2.join();

    while(bq.try_pop()) {
        items_popped++;
    }

    ASSERT_EQ(items_pushed, 2 * items_per_thread);
    ASSERT_EQ(items_popped, 2 * items_per_thread);
}
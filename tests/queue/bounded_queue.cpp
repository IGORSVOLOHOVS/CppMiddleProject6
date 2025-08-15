#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
<<<<<<< HEAD
#include <functional>
#include <stdexcept>
#include <chrono>
=======
#include <ranges>
#include <chrono>
#include <algorithm>
#include <functional>
>>>>>>> 6603adc (Refactoring ended)

#include "queue/bounded_queue.hpp"

using namespace dispatcher::queue;
using namespace std::chrono_literals;

using Task = std::function<void()>;

class BoundedQueueTest : public ::testing::Test {
protected:
    std::unique_ptr<BoundedQueue> queue_ptr;
    const unsigned int capacity = 3;

    void SetUp() override {
        queue_ptr = std::make_unique<BoundedQueue>(capacity);
    }
};

TEST_F(BoundedQueueTest, ConstructorSetsCapacity) {
    ASSERT_NO_THROW(BoundedQueue(10));
}

TEST_F(BoundedQueueTest, ThrowsOnZeroCapacity) {
    ASSERT_THROW(BoundedQueue(0), std::invalid_argument);
}

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
                bq.push([&] { tasks_executed++; });
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
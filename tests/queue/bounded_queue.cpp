#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <functional>
#include <stdexcept>
#include <chrono>

#include "queue/bounded_queue.hpp"

using namespace dispatcher::queue;

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

TEST_F(BoundedQueueTest, PushAndPopWorksInSingleThread) {
    bool task_executed = false;
    Task my_task = [&]() { task_executed = true; };

    queue_ptr->push(my_task);
    auto popped_task = queue_ptr->try_pop();

    ASSERT_TRUE(popped_task.has_value());
    popped_task.value()();
    ASSERT_TRUE(task_executed);
    ASSERT_FALSE(queue_ptr->try_pop().has_value());
}

TEST_F(BoundedQueueTest, PushBlocksWhenQueueIsFull) {
    for (unsigned int i = 0; i < capacity; ++i) {
        queue_ptr->push([]() {});
    }

    std::atomic<bool> push_started = false;
    std::thread producer_thread([&]() {
        push_started = true;
        queue_ptr->push([]() {});
    });

    while (!push_started) {
        std::this_thread::yield();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto task = queue_ptr->try_pop();
    ASSERT_TRUE(task.has_value());
    ASSERT_NE(task.value(), nullptr);

    task = queue_ptr->try_pop();
    ASSERT_TRUE(task.has_value());
    ASSERT_NE(task.value(), nullptr);  

    task = queue_ptr->try_pop();
    ASSERT_TRUE(task.has_value());
    ASSERT_NE(task.value(), nullptr);

    ASSERT_FALSE(queue_ptr->try_pop().has_value());

    queue_ptr->try_pop();
    
    producer_thread.join(); 
}
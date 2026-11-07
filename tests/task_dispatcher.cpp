#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>
#include <vector>

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

        // Счётчик увеличивается под тем же мьютексом, на котором ждёт
        // основной поток. Без этого возможен потерянный сигнал: основной
        // поток проверяет предикат (19 из 20), и до того, как cv.wait()
        // успевает атомарно отпустить мьютекс и заснуть, последняя задача
        // доводит счётчик до 20 и вызывает notify_one() ни на чём не держась.
        // Уведомление уходит в никуда, предикат больше никто не пересчитает,
        // и тест висит до таймаута вместо того, чтобы упасть. Атомарность
        // самого счётчика от этого не спасает - важно, что изменение
        // происходит под мьютексом ожидающего.
        auto count_one_task = [&]() {
            {
                std::lock_guard<std::mutex> lock(m);
                task_counter++;
            }
            cv.notify_one();
        };

        for (size_t i = 0; i < high_priority_tasks; ++i) {
            td.schedule(dispatcher::TaskPriority::High, count_one_task);
        }

        for (size_t i = 0; i < normal_priority_tasks; ++i) {
            td.schedule(dispatcher::TaskPriority::Normal, count_one_task);
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

    std::mutex gate_mutex;
    std::condition_variable gate_cv;
    bool everything_is_queued = false;

    {
        // Один рабочий поток, а не два. Порядок выполнения вообще наблюдаем
        // только там, где его кто-то один определяет: при двух потоках на
        // границе High/Normal поток A забирает пятую High-задачу, поток B -
        // первую Normal, и кто из них раньше допишет в execution_order, не
        // определено ничем. Это утверждение теста ломалось бы даже на
        // идеальной очереди. Многопоточность проверяет соседний тест
        // ExecutesTasks; здесь проверяется порядок, который даёт pop().
        dispatcher::TaskDispatcher td(1);

        // Заглушка, занимающая единственный поток, пока в очередь не лягут
        // все задачи. Без неё тест проверял не приоритет, а скорость машины:
        // пул начинает разбирать очередь с первого же push, поэтому часть
        // Normal-задач успевала выполниться раньше, чем в очередь попадала
        // первая High. PriorityQueue при этом ни в чём не виновата - она
        // отдаёт High первой, если та уже лежит в очереди.
        //
        // Заглушка уходит с приоритетом High и push'ится первой, а очередь
        // одного приоритета - FIFO, поэтому первый pop() гарантированно
        // достаёт именно её, в каком бы порядке ни просыпался поток.
        td.schedule(dispatcher::TaskPriority::High, [&]() {
            std::unique_lock<std::mutex> lock(gate_mutex);
            gate_cv.wait(lock, [&]() { return everything_is_queued; });
        });

        for (size_t i = 0; i < normal_priority_tasks; ++i) {
            td.schedule(dispatcher::TaskPriority::Normal, [&]() {
                std::lock_guard<std::mutex> lock(m);
                execution_order.push_back(dispatcher::TaskPriority::Normal);
                cv.notify_one();
            });
        }

        for (size_t i = 0; i < high_priority_tasks; ++i) {
            td.schedule(dispatcher::TaskPriority::High, [&]() {
                std::lock_guard<std::mutex> lock(m);
                execution_order.push_back(dispatcher::TaskPriority::High);
                cv.notify_one();
            });
        }

        {
            std::lock_guard<std::mutex> lock(gate_mutex);
            everything_is_queued = true;
        }
        gate_cv.notify_one();

        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&]() { return execution_order.size() == total_tasks; });
    }

    ASSERT_EQ(execution_order.size(), total_tasks);

    for (size_t i = 0; i < high_priority_tasks; ++i) {
        ASSERT_EQ(execution_order[i], dispatcher::TaskPriority::High);
    }

    for (size_t i = high_priority_tasks; i < total_tasks; ++i) {
        ASSERT_EQ(execution_order[i], dispatcher::TaskPriority::Normal);
    }
}
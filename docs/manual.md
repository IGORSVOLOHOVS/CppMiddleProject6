# Manual

## The library

A task is a `std::function<void()>`. The dispatcher takes the size of its pool, and a queue per priority:

```cpp
#include "task_dispatcher.hpp"

using namespace dispatcher;

TaskDispatcher td(4);
td.schedule(TaskPriority::High, [] { /* runs first */ });
td.schedule(TaskPriority::Normal, [] { /* runs when no high task waits */ });
```

The default is a bounded queue of 1000 for `High` and an unbounded one for `Normal`. Pass your own map of `TaskPriority` to `queue::QueueOptions` as the second argument to change that.

The destructor of `TaskDispatcher` shuts the queue down, lets the workers finish what is queued, and joins them.

A task that throws does not stop the pool: the worker writes the message to stderr and takes the next task.

## A known limit

`PriorityQueue::push` keeps its own lock while it calls the inner queue's `push`, and `BoundedQueue::push` blocks while the queue is full. So a push into a full bounded queue stops `pop` too, and the dispatcher deadlocks. Keep a bounded queue's capacity above what can be waiting, or use an unbounded one. No test reaches this.

## The demo

`.\recipe.ps1 run` builds `dispatcher` and runs it: five threads schedule ten normal and ten high priority tasks each, and the pool prints 100 lines. Their order varies from run to run.

## The tests

`.\recipe.ps1 test` runs them through `ctest`, each binary with a limit of 120 seconds, so a hang fails instead of holding a runner. By hand, after a build:

| command | what it does |
| --- | --- |
| `build\tests\dispatcher_test.exe` | the dispatcher's cases |
| `build\tests\queue\queue_test.exe` | the queues' cases |
| `build\tests\queue\queue_test.exe --gtest_filter=PriorityQueueTest.*` | one suite |

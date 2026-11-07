# cpp-middle-project6

A priority task dispatcher in modern C++23: tasks go in with a priority, and a pool of threads runs them, the high priority ones first. It began as the sprint 6 project of the Yandex Practicum "Middle C++ developer" course. It depends on nothing but the standard library; only its tests fetch GoogleTest.

## What it does

- **Queues**: `BoundedQueue` and `UnboundedQueue` behind `IQueue`, each with `push` and `try_pop`.
- **Priority queue**: `PriorityQueue` holds one queue per `TaskPriority` and pops from the highest that has a task. `pop` blocks until a task or `shutdown` arrives, and drains what is left after `shutdown`.
- **Thread pool**: `ThreadPool` runs tasks on `std::jthread` workers. A task's exception stops at the worker and is written to stderr.
- **Dispatcher**: `TaskDispatcher` joins the two: `schedule(priority, task)`.
- **Demo**: `dispatcher` has five threads schedule 100 messages and prints them through `Logger`.

The queues lock and the pool catches on purpose, and `.\recipe.ps1 test` pins both: a blocking queue is the assignment, and a task is someone else's code.

## Layout

```text
include/  the headers, with queue/ and thread_pool/
src/      the queues, the pool and the dispatcher
main.cpp  the demo
tests/    GoogleTest cases
```

## Build and test

`.\recipe.ps1 test` builds everything, runs `ctest`, `clang-tidy` and `clang-format`, and searches `include` and `src` for locks and for `throw`, `try` and `catch`. It needs mingw g++, cmake, ninja and LLVM. `docs/install.md` has the details, and `docs/manual.md` covers the library and the demo.

MIT licensed: `LICENSE` is the course's, copyright Yandex Practicum.

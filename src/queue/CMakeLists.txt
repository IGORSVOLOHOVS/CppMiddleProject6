add_library(queue
    bounded_queue.cpp
    unbounded_queue.cpp
    priority_queue.cpp
)
warn_as_errors(queue)
target_link_libraries(queue PUBLIC Threads::Threads)

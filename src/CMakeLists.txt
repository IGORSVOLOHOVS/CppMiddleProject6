add_subdirectory(queue)
add_subdirectory(thread_pool)

add_library(task_dispatcher
    task_dispatcher.cpp
)
warn_as_errors(task_dispatcher)

target_link_libraries(task_dispatcher
    PUBLIC
        thread_pool
        queue
)

add_library(thread_pool
    thread_pool.cpp
)
warn_as_errors(thread_pool)
target_link_libraries(thread_pool PUBLIC queue Threads::Threads)

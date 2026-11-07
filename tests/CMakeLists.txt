set(target dispatcher_test)

add_executable(${target}
    task_dispatcher.cpp
)
warn_as_errors(${target})

target_link_libraries(${target}
    PRIVATE
        GTest::gtest_main
        task_dispatcher
)

add_test(NAME ${target} COMMAND ${target})
# These tests wait on threads: a hang must fail here, not hold a runner slot for ctest's default 1500 s.
set_tests_properties(${target} PROPERTIES TIMEOUT 120)

add_subdirectory(queue)

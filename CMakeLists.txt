cmake_minimum_required(VERSION 3.25)
project(priority_task_dispatcher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)# Добавление флага для генерации compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    # Исходники в UTF-8 и с кириллицей в комментариях и строковых литералах.
    # Без /utf-8 cl.exe читает их в кодовой странице системы (на русской
    # Windows это 1251), сыплет C4819 и портит литералы вроде "сообщение №".
    # Один флаг закрывает и вход, и выход: /utf-8 = /source-charset:utf-8
    # /execution-charset:utf-8.
    add_compile_options(/utf-8)
endif()

# Configure local githooks
execute_process(
    COMMAND git config core.hooksPath .githooks
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_COMMAND_RESULT
)

find_package(GTest REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

set(target dispatcher)

add_executable(${target} main.cpp)
target_include_directories(${target} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${target} 
    PRIVATE
        task_dispatcher
)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

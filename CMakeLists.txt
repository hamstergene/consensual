cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(runtests)

set(CMAKE_BUILD_TYPE Debug)

add_executable(runtests
    src/runtime.c
    src/bytes.c
    src/storage.c
    tests/runtime_tests.c
    tests/bytes_tests.c
    tests/storage_tests.c
    tests/alloc.c
    tests/main.c
    )

target_link_libraries(runtests check)

include_directories(runtests
    include
    )



cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(runtests)

add_executable(runtests
    main.c
    )

target_link_libraries(runtests check)


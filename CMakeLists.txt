cmake_minimum_required(VERSION 3.20)

project(PADT_Smart_Pointers LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(include)
add_subdirectory(src)

include(CTest)

if(BUILD_TESTING)  # We can compile or not compile tests (depends on flag)
    add_subdirectory(tests)
endif()

option(BUILD_BENCHMARKS "Build pointer allocation benchmarks" ON)
if(BUILD_BENCHMARKS)
    add_subdirectory(tests/benchmarks)
endif()

option(BUILD_MEMORY_TESTS "Build memory ownership checks" ON)
if(BUILD_MEMORY_TESTS)
    add_subdirectory(tests/memory)
endif()

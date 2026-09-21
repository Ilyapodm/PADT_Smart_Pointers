cmake_minimum_required(VERSION 3.20)

project(PADT_Smart_Pointers LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(include)
add_subdirectory(src)

include(CTest)

if(BUILD_TESTING)  # We can compile or not compile tests (depends on flag)
    add_subdirectory(tests)
endif()
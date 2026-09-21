include(FetchContent)  # connect the CMake module, that allows to get external dependences

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # for windows
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)  # turn off the mock tests
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# describes the dependence, but not downloads it
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)  # download, plug it's CMakeLists.txt in
# after that several targets appear: GTest::gtest and GTest::gtest_main

add_subdirectory(utils)
add_subdirectory(functional)

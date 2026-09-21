add_executable(functional_tests test_unique_ptr.cpp)

target_link_libraries(functional_tests
    PRIVATE
        PADT::smart_pointers
        GTest::gtest_main
)

include(GoogleTest) # add CMake module for useful functions (gtest_discover_tests)
gtest_discover_tests(functional_tests) # GTests show CTest each test
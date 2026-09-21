add_library(test_utils INTERFACE)

target_include_directories(test_utils
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(test_utils
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/lifetime_counter.hpp
)
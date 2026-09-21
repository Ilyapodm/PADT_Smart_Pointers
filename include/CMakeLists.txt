add_library(smart_pointers INTERFACE)
add_library(PADT::smart_pointers ALIAS smart_pointers)

# every target, that uses smart_pointers - should use 20 std
target_compile_features(smart_pointers 
    INTERFACE 
        cxx_std_20
)

target_include_directories(smart_pointers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(smart_pointers
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/unique_ptr.hpp
)

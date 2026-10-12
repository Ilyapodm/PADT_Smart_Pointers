add_executable(memory_tests test_memory.cpp)
target_link_libraries(memory_tests PRIVATE PADT::smart_pointers)
target_include_directories(memory_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../utils)

# Debug information and light optimization keep memory reports readable.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(memory_tests PRIVATE -g -O0)
endif()

if(BUILD_TESTING)
    add_test(NAME memory_tests COMMAND memory_tests)
endif()

find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    add_custom_target(check_valgrind
        COMMAND ${VALGRIND_EXECUTABLE}
            --tool=memcheck
            --leak-check=full
            --show-leak-kinds=all
            --errors-for-leak-kinds=definite,indirect,possible
            --track-origins=yes
            --error-exitcode=1
            $<TARGET_FILE:memory_tests>
        DEPENDS memory_tests
        USES_TERMINAL
        VERBATIM
    )
else()
    add_custom_target(check_valgrind
        COMMAND ${CMAKE_COMMAND} -E echo
            "Valgrind not found. Install it and rerun CMake configuration."
        COMMAND ${CMAKE_COMMAND} -E false
        VERBATIM
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Separate binary: Valgrind runs the executable without sanitizer instrumentation.
    add_executable(memory_tests_sanitized EXCLUDE_FROM_ALL test_memory.cpp)
    target_link_libraries(memory_tests_sanitized PRIVATE PADT::smart_pointers)
    target_include_directories(memory_tests_sanitized PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../utils)
    target_compile_options(memory_tests_sanitized PRIVATE
        -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )
    target_link_options(memory_tests_sanitized PRIVATE -fsanitize=address,undefined)

    add_custom_target(check_sanitizers
        COMMAND ${CMAKE_COMMAND} -E env
            ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
            UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
            $<TARGET_FILE:memory_tests_sanitized>
        DEPENDS memory_tests_sanitized
        USES_TERMINAL
        VERBATIM
    )
endif()

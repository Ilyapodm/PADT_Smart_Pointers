add_executable(pointer_benchmarks
    benchmark_pointers.cpp
    benchmark_barrier.cpp
)

target_link_libraries(pointer_benchmarks PRIVATE PADT::smart_pointers)

# Keep the barrier in a separate translation unit so allocations cannot be elided.
set_target_properties(pointer_benchmarks PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)

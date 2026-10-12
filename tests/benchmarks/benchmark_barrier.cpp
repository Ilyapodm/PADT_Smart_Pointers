// Defined separately, without LTO: the compiler must assume this function can
// inspect the owner and its allocation, keeping construction/destruction real.
void benchmark_barrier(const void*) {}

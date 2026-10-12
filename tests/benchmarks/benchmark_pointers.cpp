#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <type_traits>

#include "shared_ptr.hpp"
#include "unique_ptr.hpp"

void benchmark_barrier(const void* owner);

namespace {

using Value = std::uint64_t;
using Clock = std::chrono::steady_clock;
constexpr std::array<std::size_t, 3> allocation_counts{100'000, 1'000'000, 10'000'000};
constexpr std::size_t repetitions = 3;

template <typename Pointer>
double measure(std::size_t count) {
    const auto start = Clock::now();
    for (std::size_t i = 0; i < count; ++i) {
        Pointer pointer(new Value(i));
        benchmark_barrier(&pointer);
        if constexpr (std::is_pointer_v<Pointer>) {
            delete pointer;
        }
        // Smart pointers release their allocations at the end of each iteration.
    }
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

using Benchmark = double (*)(std::size_t);

void compare(const char* title, const std::array<const char*, 3>& names,
             const std::array<Benchmark, 3>& benchmarks) {
    std::cout << '\n' << title << '\n'
              << std::setw(14) << "Objects";
    for (const char* name : names) {
        std::cout << std::setw(22) << name;
    }
    std::cout << '\n';

    for (std::size_t count : allocation_counts) {
        std::array<std::array<double, repetitions>, 3> times{};
        for (Benchmark benchmark : benchmarks) {
            benchmark(10'000);  // Untimed warm-up for the allocator and code paths.
        }
        for (std::size_t repeat = 0; repeat < repetitions; ++repeat) {
            // Rotate execution order to reduce the advantage of running first.
            for (std::size_t offset = 0; offset < benchmarks.size(); ++offset) {
                const std::size_t index = (repeat + offset) % benchmarks.size();
                times[index][repeat] = benchmarks[index](count);
            }
        }
        std::cout << std::setw(14) << count;
        for (auto& samples : times) {
            std::sort(samples.begin(), samples.end());
            std::cout << std::setw(22) << samples[repetitions / 2];
        }
        std::cout << std::endl;
    }
}

}  // namespace

int main() {
    try {
        std::cout << std::fixed << std::setprecision(3)
                  << "Pointer allocation benchmarks (milliseconds, median of 3 runs)\n"
                  << "Each iteration: new uint64_t -> owner construction -> delete/destruction.\n"
                  << "Objects are released immediately; the count is per run.\n"
                  << "Shared pointers also allocate a separate control block.\n"
                  << "std::shared_ptr uses new, not make_shared, for the same allocation strategy.\n"
                  << "All variants include the same external optimization barrier call.\n";
#ifndef NDEBUG
        std::cout << "Debug build: use Release for meaningful timing comparisons.\n";
#endif
        compare("Unique ownership",
                {"raw ptr", "UniquePtr", "std::unique_ptr"},
                {measure<Value*>, measure<UniquePtr<Value>>, measure<std::unique_ptr<Value>>});
        compare("Shared ownership",
                {"raw ptr", "SharedPtr", "std::shared_ptr"},
                {measure<Value*>, measure<SharedPtr<Value>>, measure<std::shared_ptr<Value>>});
    } catch (const std::exception& error) {
        std::cerr << "Benchmark failed: " << error.what() << '\n';
        return 1;
    }
}

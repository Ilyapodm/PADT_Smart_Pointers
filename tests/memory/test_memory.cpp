#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "dynamic_array.hpp"
#include "lifetime_counter.hpp"
#include "shared_ptr.hpp"
#include "unique_ptr.hpp"

namespace {

using Object = test_utils::LifetimeCounter;

// Unlike assert(), these checks remain active in Release builds.
void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void unique_object() {
    UniquePtr<Object> source(new Object);
    Object* original = source.get();
    UniquePtr<Object> moved(std::move(source));
    check(!source && moved.get() == original, "UniquePtr move lost ownership");

    UniquePtr<Object> destination(new Object);
    destination = std::move(moved);
    check(!moved && destination.get() == original, "UniquePtr move assignment failed");
    check(Object::alive == 1, "UniquePtr did not release its old object");

    destination.reset(new Object);
    destination.reset(destination.get());
    check(Object::alive == 1, "UniquePtr reset failed");

    Object* released = destination.release();
    check(!destination && Object::alive == 1, "UniquePtr release destroyed the object");
    delete released;
    destination.reset();
}

void unique_array() {
    UniquePtr<Object[]> source(new Object[3]);
    Object* original = source.get();
    UniquePtr<Object[]> moved(std::move(source));
    UniquePtr<Object[]> destination(new Object[2]);
    destination = std::move(moved);
    check(!source && !moved && destination.get() == original, "UniquePtr array move failed");
    check(Object::alive == 3, "UniquePtr did not release its old array");

    destination.reset(new Object[4]);
    destination.reset(destination.get());
    check(Object::alive == 4, "UniquePtr array reset failed");
    Object* released = destination.release();
    check(!destination && Object::alive == 4, "UniquePtr array release failed");
    delete[] released;
    destination.reset(new Object[2]);
}

void shared_object() {
    SharedPtr<Object> source(new Object);
    SharedPtr<Object> copy(source);
    SharedPtr<Object> destination(new Object);
    destination = copy;
    check(Object::alive == 1, "SharedPtr copy assignment leaked its old object");

    SharedPtr<Object> moved(std::move(copy));
    check(!copy && moved.get() == source.get(), "SharedPtr move failed");
    source.reset(new Object);
    check(Object::alive == 2, "SharedPtr reset destroyed a shared object");
    destination.reset();
    check(Object::alive == 2, "SharedPtr destroyed an object with a remaining owner");

    source = std::move(moved);
    check(!moved && Object::alive == 1, "SharedPtr move assignment failed");
    source.reset();
    check(Object::alive == 0, "SharedPtr last owner did not destroy the object");
    source.reset();
    source.reset(new Object);
}

void shared_array() {
    SharedPtr<Object[]> source(new Object[3]);
    SharedPtr<Object[]> copy(source);
    SharedPtr<Object[]> destination(new Object[2]);
    destination = copy;
    check(Object::alive == 3, "SharedPtr copy assignment leaked its old array");

    SharedPtr<Object[]> moved(std::move(copy));
    check(!copy && moved.get() == source.get(), "SharedPtr array move failed");
    source.reset(new Object[4]);
    check(Object::alive == 7, "SharedPtr reset destroyed a shared array");
    destination.reset();
    check(Object::alive == 7, "SharedPtr destroyed an array with a remaining owner");

    source = std::move(moved);
    check(!moved && Object::alive == 3, "SharedPtr array move assignment failed");
    source.reset();
    check(Object::alive == 0, "SharedPtr last owner did not destroy the array");
    source.reset();
    source.reset(new Object[2]);
}

void indexed_access() {
    UniquePtr<int[]> unique(new int[3]{10, 20, 30});
    unique[2] = 40;
    check(unique[0] + unique[1] + unique[2] == 70, "UniquePtr array access failed");

    SharedPtr<int[]> shared(new int[3]{10, 20, 30});
    SharedPtr<int[]> copy(shared);
    shared.reset();
    copy[2] = 40;
    check(copy[0] + copy[1] + copy[2] == 70, "SharedPtr array access failed");
}

void dynamic_array_ownership() {
    Object value;
    DynamicArray<Object> source;
    for (int i = 0; i < 5; ++i) {
        source.push_back(value);
    }
    check(Object::alive == 9, "DynamicArray growth leaked an old buffer");

    DynamicArray<Object> copy(source);
    DynamicArray<Object> destination(2);
    destination = copy;
    check(Object::alive == 25, "DynamicArray copy assignment leaked an old buffer");

    DynamicArray<Object> moved(std::move(source));
    check(source.get_size() == 0 && moved.get_size() == 5, "DynamicArray move failed");
    destination = std::move(moved);
    check(moved.get_size() == 0 && Object::alive == 17, "DynamicArray move assignment failed");

    DynamicArray<Object> empty;
    destination = empty;
    copy = std::move(empty);
    check(Object::alive == 1, "DynamicArray empty assignment did not release buffers");
}

void dynamic_array_values() {
    DynamicArray<int> initialized(3);
    check(initialized[0] + initialized[1] + initialized[2] == 0,
          "DynamicArray elements are not initialized");

    DynamicArray<std::string> source;
    source.push_back("a string long enough to use a heap allocation");
    source.push_back(source[0]);
    check(source[0] == source[1], "DynamicArray push_back invalidated its argument");

    DynamicArray<std::string> copy(source);
    copy[0] = "changed";
    check(source[0] != copy[0], "DynamicArray copy shares storage");
    source.push_back("third");
    source.push_back("fourth");
    source.push_back(source[0]);
    check(source[0] == source[4], "DynamicArray growth corrupted values");
}

void run_test(const char* name, void (*test)()) {
    Object::reset_counters();
    test();
    check(Object::alive == 0 && Object::created == Object::destroyed,
          "Some allocated objects were not destroyed");
    std::cout << "[PASS] " << name << '\n';
}

}  // namespace

int main() {
    try {
        run_test("UniquePtr object ownership", unique_object);
        run_test("UniquePtr array ownership", unique_array);
        run_test("SharedPtr object ownership", shared_object);
        run_test("SharedPtr array ownership", shared_array);
        run_test("Array indexed access", indexed_access);
        run_test("DynamicArray buffer ownership", dynamic_array_ownership);
        run_test("DynamicArray values and growth", dynamic_array_values);
        std::cout << "All memory scenarios passed.\n";
    } catch (const std::exception& error) {
        std::cerr << "[FAIL] " << error.what() << '\n';
        return 1;
    }
}

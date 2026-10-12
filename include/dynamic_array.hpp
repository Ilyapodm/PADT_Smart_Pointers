#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include "unique_ptr.hpp"

template <typename T>
class DynamicArray{
public:
    DynamicArray() : data(nullptr), size(0), capacity(0) {}

    DynamicArray(std::size_t count) : size(count), capacity(count) {
        if (count > 0) {
            data.reset(new T[count]{});
        }
    }

    DynamicArray(const T* array, std::size_t count) : size(count), capacity(count) {
        if (array == nullptr && count > 0) {
            throw std::invalid_argument("DynamicArray<T>: null source array");
        }
        if (count > 0) {
            data.reset(new T[count]);

            for (std::size_t i = 0; i < count; ++i) {
                data[i] = array[i];
            }
        }
    }

    DynamicArray(const DynamicArray<T>& other) : size(other.size), capacity(other.capacity) {
        if (capacity > 0) {
            data.reset(new T[capacity]{});
            for (std::size_t i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        }
    }

    DynamicArray& operator=(const DynamicArray<T>& other) {
        if (this != &other) {
            DynamicArray<T> copy(other);
            *this = std::move(copy);
        }

        return *this;
    }

    DynamicArray(DynamicArray<T>&& other) noexcept
    : data(std::move(other.data)), size(other.size), capacity(other.capacity) {
        other.size = other.capacity = 0;
    }

    DynamicArray& operator=(DynamicArray<T>&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            size = other.size;
            capacity = other.capacity;

            other.capacity = other.size = 0;
        }

        return *this;
    }

    const T& get(std::size_t idx) const {
        check_index(idx);

        return data[idx];
    }

    const T& operator[](std::size_t idx) const {
        check_index(idx);
        return data[idx];
    }

    T& operator[](std::size_t idx) {
        check_index(idx);
        return data[idx];
    }


    std::size_t get_size() const {
        return size;
    }

    std::size_t get_capacity() const {
        return capacity;
    }

    void push_back(const T& value) {
        if (size == capacity) {
            if (capacity > std::numeric_limits<std::size_t>::max() / 2) {
                throw std::length_error("DynamicArray<T>: capacity overflow");
            }
            reallocate(capacity == 0 ? 1 : capacity * 2, value);
        } else {
            data[size] = value;
        }
        size++;
    }

private:
    UniquePtr<T[]> data;
    std::size_t size;
    std::size_t capacity;

    void check_index(std::size_t idx) const {
        if (idx >= size) {
            throw std::out_of_range("DynamicArray<T>::get: index out of range");
        }
    }

    void reallocate(std::size_t new_capacity, const T& value) {
        UniquePtr<T[]> new_data(new T[new_capacity]);

        for (std::size_t i = 0; i < size; ++i) {
            new_data[i] = data[i];
        }
        // value may refer to an element of the old buffer.
        new_data[size] = value;

        data = std::move(new_data);
        capacity = new_capacity;
    }
};

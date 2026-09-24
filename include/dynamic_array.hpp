#pragma once

#include <cstddef>
#include <stdexcept>
#include "unique_ptr.hpp"

template <typename T>
class DynamicArray{
public:
    DynamicArray() : data(nullptr), size(0), capacity(0) {}

    DynamicArray(std::size_t count) : size(count), capacity(count) {
        if (count > 0) {
            data.reset(new T[count]);
        }
    }

    DynamicArray(const T* array, std::size_t count) : size(count), capacity(count) {
        if (count > 0) {
            data.reset(new T[count]);

            for (std::size_t idx = 0; idx < count; ++idx) {
                data[idx] = array[idx];
            }
        }
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

private:
    UniquePtr<T[]> data;
    std::size_t size;
    std::size_t capacity;

    void check_index(std::size_t idx) const {
        if (idx >= size) {
            throw std::out_of_range("DynamicArray<T>::get: index out of range");
        }
    }

    void reallocate(std::size_t size) {
        
    }
};
#pragma once
#include <cstddef>

template <typename T>
class UniquePtr {
public:
    explicit UniquePtr(T* ptr = nullptr) : ptr{ptr} {}
    
    ~UniquePtr() {
        delete ptr;
    }

    // copying is forbidden for unique_ptr
    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr& other) = delete;

    UniquePtr(UniquePtr&& other) noexcept : ptr{other.ptr} {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;

            ptr = other.ptr;
            other.ptr = nullptr;
        }

        return *this;
    }

    T& operator*() const {
        return *ptr;
    }

    T* operator->() const {
        return ptr;
    }

    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }

    T* get() const noexcept {
        return ptr;
    }

    T* release() noexcept {
        T* result = ptr;
        ptr = nullptr;
        return result;
    }

    void reset(T* ptr = nullptr) noexcept {
        if (this->ptr != ptr) {
            delete this->ptr;
            this->ptr = ptr;
        }
    }

    void swap(UniquePtr<T>& other) noexcept {
        T* tmp = ptr;
        ptr = other.ptr;
        other.ptr = tmp;
    }
private:
    T* ptr;
};

template <typename T>
class UniquePtr<T[]> {
public:
    explicit UniquePtr(T* ptr = nullptr) : ptr{ptr} {}
    
    ~UniquePtr() {
        delete[] ptr;
    }

    // copying is forbidden for unique_ptr
    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr& other) = delete;

    // TODO написать для наследующихся классов
    UniquePtr(UniquePtr&& other) noexcept : ptr{other.ptr} {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete[] ptr;

            ptr = other.ptr;
            other.ptr = nullptr;
        }

        return *this;
    }

    T& operator[](std::size_t idx) const {
        return ptr[idx];
    }

    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }

    T* get() const noexcept {
        return ptr;
    }

    T* release() noexcept {
        T* result = ptr;
        ptr = nullptr;
        return result;
    }

    void reset(T* ptr = nullptr) noexcept {
        if (this->ptr != ptr) {
            delete[] this->ptr;
            this->ptr = ptr;
        }
    }

    void swap(UniquePtr<T[]>& other) noexcept {
        T* tmp = ptr;
        ptr = other.ptr;
        other.ptr = tmp;
    }
private:
    T* ptr;
};
#include <cstddef>

template <typename T>
class SharedPtr{
public:
    SharedPtr() : ptr{nullptr}, count{nullptr} {}

    SharedPtr(T* ptr) : ptr{ptr}, count{new std::size_t} {
        ++(*count);
    }

    ~SharedPtr() {
        release_ownership();
    }

    SharedPtr(const SharedPtr<T>& other) : ptr{other.ptr}, count{other.count} {
        ++(*count);
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& other) {
        if (this != &other) {
            // this my own the data, so we need to resease the ownership
            release_ownership();
            ptr = other.ptr;
            count = other.count;
            ++(*count);
        }
        return *this;
    }

    SharedPtr(SharedPtr<T>&& other) : ptr{other.ptr}, count{other.count} {
        other.ptr = nullptr;
        other.count = nullptr;
        // don't need to increase count, because we "move" (just change the owner), don't copy
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& other) {
        if (this != &other) {
            release_ownership();
            
            ptr = other.ptr;
            count = other.count;
            
            other.ptr = nullptr;
            other.count = nullptr;
        }

        return *this;
    }





private:
    T* ptr;
    std::size_t* count;

    void release_ownership() noexcept {
        // "this ptr doesn't belong to any ptrs 'group'"
        if (count == nullptr) {
            return;  // this way we don't own any data, so nothing to release
        }

        --(*count);

        if (*count == 0) {
            delete ptr;
            delete count;
        }

        ptr = nullptr;
        count = nullptr;
    }
};
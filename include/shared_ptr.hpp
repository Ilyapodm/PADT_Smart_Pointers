#include <cstddef>
#include <new>

template <typename T>
class SharedPtr{
public:
    explicit SharedPtr(T* ptr = nullptr) : ptr{ptr}, count{nullptr} {
        if (ptr != nullptr) {
            try{
                count = new std::size_t{1};
            } catch(const std::bad_alloc&) {
                delete ptr;
                throw;
            }
        }
    }

    ~SharedPtr() {
        release_ownership();
    }

    SharedPtr(const SharedPtr<T>& other) : ptr{other.ptr}, count{other.count} {
        if (count != nullptr) {
            ++(*count);
        }
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& other) {
        if (this != &other) {
            // this may already own an object, so release the current ownership first
            release_ownership();
            ptr = other.ptr;
            count = other.count;

            if (count != nullptr) {
                ++(*count);
            }
        }
        return *this;
    }

    SharedPtr(SharedPtr<T>&& other) noexcept : ptr{other.ptr}, count{other.count} {
        other.ptr = nullptr;
        other.count = nullptr;
        // don't need to increase count, because we "move" (just change the owner), don't copy
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& other) noexcept {
        if (this != &other) {
            release_ownership();
            
            ptr = other.ptr;
            count = other.count;
            
            other.ptr = nullptr;
            other.count = nullptr;
        }

        return *this;
    }

    explicit operator bool() const noexcept {
        return ptr != nullptr;
    }

    T& operator*() const noexcept {
        return *ptr;
    }

    T* operator->() const noexcept {
        return ptr;
    }

    T* get() const noexcept {
        return ptr;
    }

    void swap(SharedPtr<T>& other) noexcept {
        // exchange both: count ptrs and data ptrs
        T* tmp_ptr = ptr;
        ptr = other.ptr;
        other.ptr = tmp_ptr;

        std::size_t* tmp_count = count;
        count = other.count;
        other.count = tmp_count;
    }

    void reset(T* ptr = nullptr) {
        release_ownership();
        
        if (ptr != nullptr) {
            
            try {
                std::size_t* new_count = new std::size_t{1};
                
                this->count = new_count;
                this->ptr = ptr;
            } catch(const std::bad_alloc&) {
                delete ptr;
                throw;
            }
            
        }
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
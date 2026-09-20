
template <typename T>
class UniquePtr {
public:
    UniquePtr(T* ptr) : ptr{ptr} {}
    ~UniquePtr() {delete ptr;}  // FIXME

    // copying is forbedden for unique_ptr
    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr& other) = delete;

    UniquePtr(UniquePtr&& other) noexcept : ptr{other.ptr} {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) {
        if (this != &other) {
            // FIXME
            delete ptr;

            ptr = other.ptr;
            other.ptr = nullptr;
        }

        return *this;
    }

private:
    T* ptr;
};
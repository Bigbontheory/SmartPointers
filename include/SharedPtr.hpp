#pragma once

namespace detail {
    struct ControlBlock {
        int count = 1;
        void (*deleter_func)(void*) = nullptr;
    };
}

template<typename T>
class SharedPtr {
private:
    T* ptr;
    detail::ControlBlock* cb;

    template <typename U>
    friend class SharedPtr;

    void increment_count() {
        if (cb) {
            ++(cb->count);
        }
    }

    void decrement_count() {
        if (cb) {
            (cb->count)--;
            if (cb->count == 0) {
                if (ptr)
                {
                    if (cb->deleter_func) {
                        cb->deleter_func(ptr);
                    } else {
                        delete ptr;
                    }
                }
                delete cb;
                cb = nullptr;
                ptr = nullptr;
            }
        }
    }

public:
    explicit SharedPtr(T* ptr_ = nullptr, void (*del)(void*) = nullptr): ptr(ptr_), cb(nullptr) {
        if (ptr) {
            cb = new detail::ControlBlock{1, del};
        }
    }

    void deleter(void (*del)(void*)) {
        if (cb) {
            cb->deleter_func = del;
        }
    }

    SharedPtr(const SharedPtr<T>& other) :
        ptr(other.ptr), cb(other.cb)  {
        increment_count();
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& other) {
        if (this != &other) {
            decrement_count();
            ptr = other.ptr;
            cb = other.cb;
            increment_count();
        }
        return *this;
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other)
        : ptr(other.ptr), cb(other.cb) {
        increment_count();
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) noexcept : ptr(other.ptr), cb(other.cb) {
        other.ptr = nullptr;
        other.cb= nullptr;
    }

    template <typename U>
    SharedPtr<T>& operator=(SharedPtr<U>&& other) noexcept {
        decrement_count();
        ptr = other.ptr;
        cb = other.cb;
        other.ptr = nullptr;
        other.cb = nullptr;
        return *this;
    }

    SharedPtr(SharedPtr<T>&& other) noexcept : ptr(other.ptr), cb(other.cb) {
        other.ptr = nullptr;
        other.cb= nullptr;
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& other) noexcept {
        if(this != &other) {
            decrement_count();
            ptr = other.ptr;
            cb = other.cb;
            other.ptr = nullptr;
            other.cb = nullptr;
        }
        return *this;
    }

    ~SharedPtr() {
        decrement_count();
    }

    void reset(T* ptr_ = nullptr, void (*del)(void*) = nullptr) {
        if (ptr != ptr_) {
            decrement_count();
            ptr = ptr_;
            if (ptr_ != nullptr) {
                cb = new detail::ControlBlock{1, del};
            }
            else {
                cb = nullptr;
            }
        }
    }

    T* get() const {return ptr;}
    T& operator*() const {return *ptr;}
    T* operator->() const {return ptr;}

    int get_count() const {
        if (cb == nullptr) {
            return 0;
        } else { return cb->count; }
    }

    bool operator==(const SharedPtr<T>& other) const {
        return (ptr == other.ptr);
    }

    bool operator!=(const SharedPtr<T>& other) const {
        return (!(ptr == other.ptr));
    }

    explicit operator bool() const {
        return ptr != nullptr;
    }

};

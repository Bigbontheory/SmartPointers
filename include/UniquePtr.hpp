#pragma once

template<typename T>
class UniquePtr {
private:
    T* ptr;
public:
    explicit UniquePtr(T* ptr_) : ptr(ptr_) {}

    UniquePtr(const UniquePtr<T>& other) = delete;
    UniquePtr<T>& operator=(const UniquePtr<T> & other) = delete;

    UniquePtr(UniquePtr<T>&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    UniquePtr<T>& operator=(UniquePtr<T>&& other) noexcept {
        if(this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    ~UniquePtr() {
        delete ptr;
    }

    T* get() const {return ptr;}
    T* operator->() const {return ptr;}
    T& operator*() const {return ptr;}

    T* release() {
        T* old_ptr = ptr;
        ptr = nullptr;
        return old_ptr;
    }

    void reset(T* new_ptr = nullptr) {
        T* old_ptr = ptr;
        ptr = new_ptr;
        delete old_ptr;
    }
};


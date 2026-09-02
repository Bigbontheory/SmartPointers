#pragma once

template<typename T>
class SharedPtr {
private:
    T* ptr;
    int* count;

    template <typename U>
    friend class SharedPtr;

    void increment_count() {
        if (count) {
            (*count)++;
        }
    }

    void decrement_count() {
        if (count) {
            (*count)--;
            if (*count == 0) {
                delete ptr;
                delete count;
                ptr = nullptr;
                count = nullptr;
            }
        }
    }

public:
    explicit SharedPtr(T* ptr_ = nullptr): ptr(ptr_), count(nullptr) {
        if (ptr) {
            count = new int(1);
        }
    }

    SharedPtr(const SharedPtr<T>& other) : ptr(other.ptr), count(other.count) {
        increment_count();
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& other) {
        if (this != &other) {
            decrement_count();
            ptr = other.ptr;
            count = other.count;
            increment_count();
        }
        return *this;
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : ptr(other.ptr), count(other.count) {
        increment_count();
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) noexcept : ptr(other.ptr), count(other.count) {
        other.ptr = nullptr;
        other.count = nullptr;
    }

    SharedPtr(SharedPtr<T>&& other) noexcept : ptr(other.ptr), count(other.count) {
        other.ptr = nullptr;
        other.count = nullptr;
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& other) noexcept {
        if(this != &other) {
            decrement_count();
            ptr = other.ptr;
            count = other.count;
            other.ptr = nullptr;
            other.count = nullptr;
        }
        return *this;
    }

    ~SharedPtr() {
        decrement_count();
    }

    void reset(T* ptr_ = nullptr) {
        if (ptr != ptr_) {
            decrement_count();
            ptr = ptr_;
            if (ptr_ != nullptr) {
                count = new int(1);
            }
            else {
                count = nullptr;
            }
        }
    }

    T* get() const {return ptr;}
    T& operator*() const {return *ptr;}
    T* operator->() const {return ptr;}
    int get_count() const {
        if (count == nullptr) {
            return 0;
        } else {return *count;}
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

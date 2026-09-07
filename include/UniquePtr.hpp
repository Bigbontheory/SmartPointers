#pragma once

template<typename T>
class UniquePtr {
private:
    T* ptr;
    void(*deleter_func)(void*);

    template <typename U>
    friend class UniquePtr;

    void clean_up(T* p) {
        if (p) {
            if (deleter_func) {
                deleter_func(p);
            } else { delete p; }
        }
    }


public:
    explicit UniquePtr(T* ptr_ = nullptr, void (*del)(void*) = nullptr) :
        ptr(ptr_), deleter_func(del)  {}

    void deleter(void (*del)(void*)) {
        deleter_func = del;
    }

    template <typename U>
    UniquePtr(UniquePtr<U>&& other) noexcept : ptr(other.ptr), deleter_func(other.deleter_func) {
        other.ptr = nullptr;
        other.deleter_func = nullptr;
    }

    template <typename U>
    UniquePtr<T>& operator=(UniquePtr<U>&& other) noexcept {
        clean_up(ptr);
        ptr = other.ptr;
        deleter_func = other.deleter_func;

        other.ptr = nullptr;
        other.deleter_func = nullptr;
        return *this;
    }

    UniquePtr(const UniquePtr<T>& other) = delete;
    UniquePtr<T>& operator=(const UniquePtr<T> & other) = delete;

    UniquePtr(UniquePtr<T>&& other) noexcept : ptr(other.ptr), deleter_func(other.deleter_func)  {
        other.ptr = nullptr;
        other.deleter_func = nullptr;
    }

    UniquePtr<T>& operator=(UniquePtr<T>&& other) noexcept {
        if(this != &other) {
            clean_up(ptr);
            ptr = other.ptr;
            deleter_func = other.deleter_func;

            other.ptr = nullptr;
            other.deleter_func = nullptr;
        }
        return *this;
    }

    ~UniquePtr() {
        clean_up(ptr);
    }

    T* get() const {return ptr;}
    T* operator->() const {return ptr;}
    T& operator*() const {return *ptr;}

    T* release() {
        T* old_ptr = ptr;
        ptr = nullptr;
        deleter_func = nullptr;
        return old_ptr;
    }

    void reset(T* new_ptr = nullptr) {
        T* old_ptr = ptr;
        ptr = new_ptr;
        clean_up(old_ptr);
    }

    explicit operator bool() const{
        return ptr!= nullptr;
    }

};


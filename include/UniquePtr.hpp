#pragma once
#include<utility>
#include <type_traits>

template<typename T>
struct DefaultDelete {
    constexpr DefaultDelete() noexcept = default;

    template<typename U>
    DefaultDelete(const DefaultDelete<U>&) noexcept {}

    template<typename U>
    DefaultDelete& operator=(const DefaultDelete<U>&) noexcept {
        return *this;
    }


    void operator()(T* ptr) const {
        delete ptr;
    }
};

template<typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr {
private:
    T* ptr = nullptr;

    Deleter deleter;

    template <typename U, typename D>
    friend class UniquePtr;

public:
    explicit UniquePtr(T* ptr_ = nullptr) :
        ptr(ptr_), deleter(Deleter()) {}

    UniquePtr(T* p, Deleter d) :
        ptr(p), deleter(std::move(d)) {}

    template <typename U, typename D>
    UniquePtr(UniquePtr<U, D>&& other) noexcept : ptr(other.ptr), deleter(std::move(other.deleter)) {
        other.ptr = nullptr;
    }

    template <typename U, typename D>
    UniquePtr<T, Deleter>& operator=(UniquePtr<U, D>&& other) noexcept {
        reset();
        ptr = other.ptr;
        deleter = std::move(other.deleter);
        other.ptr = nullptr;
        return *this;
    }

    UniquePtr(const UniquePtr<T>& other) = delete;
    UniquePtr& operator=(const UniquePtr<T> & other) = delete;

    template<typename U, typename D>
    UniquePtr(const UniquePtr<U, D>& other) = delete;

    template<typename U, typename D>
    UniquePtr& operator=(const UniquePtr<U, D>& other) = delete;

    UniquePtr(UniquePtr<T>&& other) noexcept : ptr(other.ptr), deleter(std::move(other.deleter))  {
        other.ptr = nullptr;
    }

    UniquePtr& operator=(UniquePtr<T>&& other) noexcept {
        if(this != &other) {
            reset();
            ptr = other.ptr;
            other.ptr = nullptr;
            deleter = std::move(other.deleter);
        }
        return *this;
    }


    ~UniquePtr() {
        if (ptr) {
            deleter(ptr);
        }
    }

    Deleter& get_deleter() {return deleter;}
    const Deleter& get_deleter() const {return deleter;}

    T* get() const { return ptr; }
    T* operator->() const { return ptr; }
    T& operator*() const { return *ptr; }


    T* release() {
        T* old_ptr = ptr;
        ptr = nullptr;
        return old_ptr;
    }

    void reset(T* new_ptr = nullptr) {
        T* old_ptr = ptr;
        ptr = new_ptr;
        if  (old_ptr) {
            deleter(old_ptr);
        };
    }

    explicit operator bool() const{
        return ptr!= nullptr;
    }

};

template <typename T, typename... Args>
std::enable_if_t<!std::is_array_v<T>, UniquePtr<T>>
make_unique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}


template<typename T>
struct DefaultDelete<T[]> {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

template<typename T>
class UniquePtr<T[], DefaultDelete<T[]>> {
private:
    T* ptr = nullptr;
public:
    explicit UniquePtr(T* p = nullptr) : ptr(p) {}

    ~UniquePtr() {
        delete[] ptr;
    }

    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept: ptr(other.ptr) {
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

    T& operator[](std::size_t index) const {
        return ptr[index];
    }

    T* get() const noexcept {
        return ptr;
    }

    explicit operator bool() const {
        return ptr != nullptr;
    }

    T* release() {
        T* old_ptr = ptr;
        ptr = nullptr;
        return old_ptr;
    }

    void reset(T* new_ptr = nullptr) {
        T* old_ptr = ptr;
        ptr = new_ptr;
        delete[] old_ptr;
    }
};

template <typename T>
std::enable_if_t<std::is_unbounded_array_v<T>, UniquePtr<T>>
make_unique(std::size_t size) {
    using ElementType = std::remove_extent_t<T>;
    return UniquePtr<T>(new ElementType[size]());
}


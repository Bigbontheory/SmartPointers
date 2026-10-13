#pragma once
#include <cstddef>
#include <utility>


template<typename T>
struct DefaultDelete {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

struct ControlBlockBase {
    size_t ref_count = 1;

    virtual ~ControlBlockBase() = default;

    void add_ref() noexcept {
        ++ref_count;
    }

    void release_ref() noexcept {
        if (--ref_count == 0) {
            destroy_and_free();
        }
    }

protected:
    virtual void destroy_and_free() noexcept = 0;
};

template <typename Ptr, typename Deleter>
struct ControlBlockPtr final : ControlBlockBase {
    Ptr ptr;
    Deleter deleter;

    ControlBlockPtr(Ptr p, Deleter d)
        : ptr(p), deleter(std::move(d)) {}

protected:
    void destroy_and_free() noexcept override {
        deleter(ptr);
        delete this;
    }
};

template <typename T>
struct ControlBlockArray final : ControlBlockBase {
    T* ptr;

    explicit ControlBlockArray(T* p) : ptr(p) {}

protected:
    void destroy_and_free() noexcept override {
        delete[] ptr;
        delete this;
    }
};



template<typename T>
class SharedPtr {
private:
    T* ptr_ = nullptr;
    ControlBlockBase* cb_ = nullptr;
    template <typename U>
    friend class SharedPtr;

public:
    SharedPtr() = default;

    explicit SharedPtr(T* p)
        : SharedPtr(p, DefaultDelete<T>{}) {}

    template <typename Deleter>
    SharedPtr (T* p, Deleter d)
        : ptr_(p) {
        if  (p) {
            cb_ = new ControlBlockPtr<T*, Deleter>(p, std::move(d));
        }
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) noexcept
        : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->add_ref();
        }
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& other) noexcept
        : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) noexcept {
        if (this->cb_ != other.cb_) {
            if (cb_) {
                cb_->release_ref();
            }
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            if (cb_) {
                cb_->add_ref();
            }
        }
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) noexcept {
        if (this->cb_ != other.cb_) {
            if (cb_) {
                cb_->release_ref();
            }
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            other.ptr_ = nullptr;
            other.cb_ = nullptr;
        }
        return *this;
    }


    ~SharedPtr() {
        if (cb_) {
            cb_->release_ref();
        }
    }

    SharedPtr(const SharedPtr& other) :
        ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->add_ref();
        }
    }


    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            if (cb_) {
                cb_->release_ref();
            }

            ptr_ = other.ptr_;
            cb_ = other.cb_;

            if (cb_) {
                cb_->add_ref();
            }
        }
        return *this;
    }

    SharedPtr(SharedPtr&& other) noexcept
        : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            if (cb_) {
                cb_->release_ref();
            }

            ptr_ = other.ptr_;
            cb_ = other.cb_;

            other.ptr_ = nullptr;
            other.cb_ = nullptr;
        }
        return *this;
    }
    void reset() noexcept {
        if (cb_) {
            cb_->release_ref();
            cb_ = nullptr;
        }
        ptr_ = nullptr;
    }


    template <typename Y>
    void reset(Y* p) {
        reset(p, DefaultDelete<Y>{});
    }

    template <typename Y, typename Deleter>
    void reset(Y* p, Deleter d) {

        if (cb_) {
            cb_->release_ref();
            cb_ = nullptr;
        }
        ptr_ = p;
        if (p) {
            cb_ = new ControlBlockPtr<Y*, Deleter>(p, std::move(d));
        }
    }

    T* get() const noexcept { return ptr_; }
    T& operator*() const noexcept { return *ptr_; }
    T* operator->() const noexcept { return ptr_; }

    std::size_t use_count() const noexcept {
        return cb_ ? cb_->ref_count : 0;
    }

    explicit operator bool() const noexcept {
        return ptr_ != nullptr;
    }

};

template<typename T>
class SharedPtr<T[]> {
private:
    template<typename U>
    friend class SharedPtr;

    T* ptr_ = nullptr;
    ControlBlockBase* cb_ = nullptr;
public:
    SharedPtr() = default;

    explicit SharedPtr(T* p) : ptr_(p) {
        if (p) {
            cb_ = new ControlBlockArray<T>(p);
        }
    }

    ~SharedPtr() {
        if (cb_) {
            cb_->release_ref();
        }
    }

    SharedPtr(const SharedPtr& other)
        : ptr_(other.ptr_), cb_(other.cb_) {
        if (cb_) {
            cb_->add_ref();
        }
    }

    SharedPtr& operator=(const SharedPtr& other)  {
        if (this != &other) {
            if (cb_) {
                cb_->release_ref();
            }
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            if (cb_) {
                cb_->add_ref();
            }
        }
        return *this;
    }

    SharedPtr(SharedPtr&& other) noexcept
        : ptr_(other.ptr_), cb_(other.cb_) {
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this != &other) {
            if (cb_) {
                cb_->release_ref();
            }
            ptr_ = other.ptr_;
            cb_ = other.cb_;
            other.ptr_ = nullptr;
            other.cb_ = nullptr;
        }
        return *this;
    }

    void reset() noexcept {
        if (cb_) {
            cb_->release_ref();
            cb_ = nullptr;
        }
        ptr_ = nullptr;
    }

    void reset(T* p) {
        if (cb_) {
            cb_->release_ref();
            cb_ = nullptr;
        }
        ptr_ = p;
        if (p) {
            cb_ = new ControlBlockArray<T>(p);
        }
    }

    T& operator[] (size_t index) const {
        return ptr_[index];
    }

    T* get() const noexcept { return ptr_; }
    size_t use_count() const noexcept { return cb_ ? cb_->ref_count : 0; }
    explicit operator bool() const noexcept { return ptr_ != nullptr; }
};

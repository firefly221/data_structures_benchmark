#pragma once
#include <cstddef>
#include <new>
#include <utility>
#include <type_traits>
#include <stdexcept>

template <class T, std::size_t N>
class SmallVector {
public:
    SmallVector() noexcept : data_(inline_ptr()), size_(0), cap_(N) {}

    ~SmallVector() {
        destroy_range(0, size_);
        if (!using_inline())
            ::operator delete(static_cast<void*>(data_));
    }

    // copy ctor
    SmallVector(const SmallVector& other) : SmallVector() {
        reserve(other.size_);
        for (std::size_t i = 0; i < other.size_; ++i)
            new (data_ + i) T(other.data_[i]);
        size_ = other.size_;
    }

    // move ctor
    SmallVector(SmallVector&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : SmallVector() {
        if (other.using_inline()) {
            for (std::size_t i = 0; i < other.size_; ++i)
                new (data_ + i) T(std::move(other.data_[i]));
            size_ = other.size_;
            other.clear();
        } else {
            data_ = other.data_;
            size_ = other.size_;
            cap_  = other.cap_;

            other.data_ = other.inline_ptr();
            other.size_ = 0;
            other.cap_  = N;
        }
    }

    SmallVector& operator=(const SmallVector& other) {
        if (this == &other) return *this;
        SmallVector tmp(other);
        swap(tmp);
        return *this;
    }

    SmallVector& operator=(SmallVector&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_move_assignable_v<T>
    ) {
        if (this == &other) return *this;

        destroy_range(0, size_);
        if (!using_inline())
            ::operator delete(static_cast<void*>(data_));

        if (other.using_inline()) {
            data_ = inline_ptr();
            cap_  = N;
            size_ = 0;
            for (std::size_t i = 0; i < other.size_; ++i)
                new (data_ + i) T(std::move(other.data_[i]));
            size_ = other.size_;
            other.clear();
        } else {
            data_ = other.data_;
            size_ = other.size_;
            cap_  = other.cap_;
            other.data_ = other.inline_ptr();
            other.size_ = 0;
            other.cap_  = N;
        }
        return *this;
    }

    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return cap_; }
    bool empty() const noexcept { return size_ == 0; }

    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    T& operator[](std::size_t i) noexcept { return data_[i]; }
    const T& operator[](std::size_t i) const noexcept { return data_[i]; }

    T& at(std::size_t i) {
        if (i >= size_) throw std::out_of_range("SmallVector::at");
        return data_[i];
    }
    const T& at(std::size_t i) const {
        if (i >= size_) throw std::out_of_range("SmallVector::at");
        return data_[i];
    }

    T& back() noexcept { return data_[size_ - 1]; }
    const T& back() const noexcept { return data_[size_ - 1]; }

    T* begin() noexcept { return data_; }
    T* end() noexcept { return data_ + size_; }
    const T* begin() const noexcept { return data_; }
    const T* end() const noexcept { return data_ + size_; }

    void clear() noexcept {
        destroy_range(0, size_);
        size_ = 0;
    }

    void reserve(std::size_t new_cap) {
        if (new_cap <= cap_) return;
        grow_to(new_cap);
    }

    template <class... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == cap_) reserve(next_capacity());
        new (data_ + size_) T(std::forward<Args>(args)...);
        ++size_;
        return data_[size_ - 1];
    }

    void push_back(const T& x) { (void)emplace_back(x); }
    void push_back(T&& x) { (void)emplace_back(std::move(x)); }

    void pop_back() {
        if (size_ == 0) return;
        data_[size_ - 1].~T();
        --size_;
    }

    void swap(SmallVector& other) noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_swappable_v<T>
    ) {
        if (this == &other) return;
        SmallVector tmp(std::move(other));
        other = std::move(*this);
        *this = std::move(tmp);
    }

private:
    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
    Storage inline_[(N > 0 ? N : 1)];

    T* data_;
    std::size_t size_;
    std::size_t cap_;

    T* inline_ptr() noexcept { return reinterpret_cast<T*>(inline_); }
    const T* inline_ptr() const noexcept { return reinterpret_cast<const T*>(inline_); }
    bool using_inline() const noexcept { return data_ == inline_ptr(); }

    std::size_t next_capacity() const noexcept {
        if (cap_ == 0) return 1;
        return cap_ * 2;
    }

    void destroy_range(std::size_t from, std::size_t to) noexcept {
        for (std::size_t i = from; i < to; ++i)
            data_[i].~T();
    }

    void grow_to(std::size_t new_cap) {
        T* new_data = static_cast<T*>(::operator new(sizeof(T) * new_cap));

        for (std::size_t i = 0; i < size_; ++i) {
            if constexpr (std::is_nothrow_move_constructible_v<T> ||
                          !std::is_copy_constructible_v<T>) {
                new (new_data + i) T(std::move(data_[i]));
            } else {
                new (new_data + i) T(data_[i]);
            }
        }

        destroy_range(0, size_);

        if (!using_inline())
            ::operator delete(static_cast<void*>(data_));

        data_ = new_data;
        cap_  = new_cap;
    }
};

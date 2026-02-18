#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <utility>

template <class T>
class RingBuffer {
public:
    explicit RingBuffer(std::size_t capacity)
        : buf_(capacity),
          cap_(capacity),
          head_(0),
          tail_(0),
          size_(0)
    {
        if (cap_ == 0)
            throw std::invalid_argument("RingBuffer capacity must be > 0");
    }

    std::size_t size() const noexcept { return size_; }
    std::size_t capacity() const noexcept { return cap_; }
    bool empty() const noexcept { return size_ == 0; }
    bool full() const noexcept { return size_ == cap_; }

    // --- push ---

    void push(const T& value) {
        if (full())
            throw std::runtime_error("RingBuffer is full");

        buf_[tail_] = value;
        advance_tail_();
    }

    void push(T&& value) {
        if (full())
            throw std::runtime_error("RingBuffer is full");

        buf_[tail_] = std::move(value);
        advance_tail_();
    }

    bool try_push(const T& value) {
        if (full()) return false;
        buf_[tail_] = value;
        advance_tail_();
        return true;
    }

    bool try_push(T&& value) {
        if (full()) return false;
        buf_[tail_] = std::move(value);
        advance_tail_();
        return true;
    }

    // --- pop ---

    void pop() {
        if (empty())
            throw std::runtime_error("RingBuffer is empty");

        advance_head_();
    }

    bool try_pop() {
        if (empty()) return false;
        advance_head_();
        return true;
    }

    // --- access ---

    T& front() {
        if (empty())
            throw std::runtime_error("RingBuffer is empty");
        return buf_[head_];
    }

    const T& front() const {
        if (empty())
            throw std::runtime_error("RingBuffer is empty");
        return buf_[head_];
    }

    T& back() {
        if (empty())
            throw std::runtime_error("RingBuffer is empty");
        std::size_t idx = (tail_ + cap_ - 1) % cap_;
        return buf_[idx];
    }

    const T& back() const {
        if (empty())
            throw std::runtime_error("RingBuffer is empty");
        std::size_t idx = (tail_ + cap_ - 1) % cap_;
        return buf_[idx];
    }

private:
    void advance_tail_() noexcept {
        tail_ = (tail_ + 1) % cap_;
        ++size_;
    }

    void advance_head_() noexcept {
        head_ = (head_ + 1) % cap_;
        --size_;
    }

    std::vector<T> buf_;
    std::size_t cap_;
    std::size_t head_;
    std::size_t tail_;
    std::size_t size_;
};

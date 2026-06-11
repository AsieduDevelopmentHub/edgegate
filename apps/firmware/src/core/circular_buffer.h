#pragma once

#include <stddef.h>
#include <stdint.h>

template<typename T, size_t N>
class CircularBuffer {
public:
    CircularBuffer() : head_(0), tail_(0), count_(0) {}

    bool push(const T& item) {
        if (count_ >= N) return false;
        buffer_[head_] = item;
        head_ = (head_ + 1) % N;
        count_++;
        return true;
    }

    bool pop(T& item) {
        if (count_ == 0) return false;
        item = buffer_[tail_];
        tail_ = (tail_ + 1) % N;
        count_--;
        return true;
    }

    size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }
    bool full() const { return count_ >= N; }
    size_t capacity() const { return N; }

private:
    T buffer_[N];
    size_t head_;
    size_t tail_;
    size_t count_;
};

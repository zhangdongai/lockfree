#pragma once

#include <atomic>
#include <vector>

#include "macros.h"

template <typename T>
class ElemAllocator {
public:
    static T* alloc() {
        return new T;
    }
    static void free(T* t) {
        delete t;
    }
};

static const int32_t gTryTimes = 10;

template <typename T>
class LockFreeQueue {
public:
    class Node {
    public:
        T data_;
        Node* next_ = nullptr;
    };

    LockFreeQueue() {
        Node* node = new Node;
        head_.store(node);
        tail_.store(node);
        size_.store(0);
    }
    ~LockFreeQueue() {
        ElemAllocator<Node>::free(head_.load());
    }

    bool empty() const {
        return (head_ == tail_);
    }

    int32_t size() const {
        return size_.load();
    }

    void push(const T& data) {
        Node* new_node = ElemAllocator<Node>::alloc();
        new_node->data_ = data;

        Node* tail_node = tail_.load(std::memory_order_acquire);
        CHECK_RET((tail_node == nullptr));
        volatile int32_t try_time = 0;
        volatile bool exchange_ret = true;
        while (!tail_.compare_exchange_weak(tail_node, new_node,
                                            std::memory_order_acq_rel,
                                            std::memory_order_relaxed)) {
            if (++try_time >= gTryTimes) {
                exchange_ret = false;
                break;
            }
        }
        if (exchange_ret) {
            tail_node->next_ = new_node;
            size_.fetch_add(1);
        }
    }

    bool pop(T& t) {
        // empty
        CHECK_RET_BOOL(empty());

        Node* head_node = head_.load(std::memory_order_acquire);
        CHECK_RET_BOOL((head_node == nullptr));
        Node* next_node = head_node->next_;
        CHECK_RET_BOOL((next_node == nullptr));
        volatile bool exchange_ret = true;
        volatile int32_t try_time = 0;
        while (!head_.compare_exchange_weak(head_node, head_node->next_,
                                            std::memory_order_acq_rel,
                                            std::memory_order_relaxed)) {
            if (++try_time >= gTryTimes) {
                exchange_ret = false;
                break;
            }

            next_node = head_node->next_;
            CHECK_RET_BOOL((next_node == nullptr));
        }

        CHECK_RET_BOOL(!exchange_ret);
        t = next_node->data_;
        ElemAllocator<Node>::free(head_node);
        size_.fetch_sub(1);
        return true;
    }

private:
    std::atomic<Node*> head_ = {nullptr};
    std::atomic<Node*> tail_ = {nullptr};
    std::atomic<int32_t> size_;
};

#pragma once

#include "ome/order.hpp"
#include "ome/memory_pool.hpp"

namespace ome {

template <std::size_t Capacity>
class OrderQueue {
public:
    explicit OrderQueue(MemoryPool<Order, Capacity>& pool) : pool_(pool) {} 
    
    bool empty() const { return head_ == nullptr; }

    void push_back(const Order& order) {
        Order* node = pool_.allocate();
        *node = order;
        node->next = nullptr;

        if (tail_ == nullptr) {
            head_ = node;
            tail_ = node;
        } else {
            tail_->next = node;
            tail_ = node;
        }
    }

    Order& front() {
        return *head_;
    }

    void pop_front() {
        Order* old_head = head_;
        head_ = head_->next;
        if (head_ == nullptr) {
            tail_ = nullptr;
        }
        pool_.deallocate(old_head);
    }

private:
    MemoryPool<Order, Capacity>& pool_;
    Order* head_ = nullptr;
    Order* tail_ = nullptr;
};

} 
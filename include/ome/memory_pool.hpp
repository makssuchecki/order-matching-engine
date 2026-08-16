#pragma once

#include <array>
#include <cstddef>

namespace ome{


template <typename T, std::size_t BlockCount>
class MemoryPool {
public:
    MemoryPool() {
        for(std::size_t i = 0; i<BlockCount - 1; i++){
            storage_[i].next_free = &storage_[i+1];
        }
        storage_[BlockCount - 1].next_free = nullptr;
        free_list_head_ = &storage_[0];
    }
    T* allocate() {
        if (!free_list_head_) return nullptr;
        Slot* slot = free_list_head_;
        free_list_head_ = slot->next_free;
        return new (slot) T();
    };

    void deallocate(T* ptr){
        ptr->~T();
        Slot* slot = reinterpret_cast<Slot*>(ptr);
        slot->next_free = free_list_head_;    
        free_list_head_ = slot;
    };
private:
    union Slot {
        T value;
        Slot* next_free;
    };

    std::array<Slot, BlockCount> storage_;
    Slot* free_list_head_;
};

}
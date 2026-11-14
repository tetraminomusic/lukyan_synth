#include "arena.h"

void arena_init(MemoryArena *arena, void *raw_memory, size_t capacity) {
    arena->buffer = (uint8_t *)raw_memory;
    arena->capacity = capacity;
    arena->offset = 0;
}

void *arena_alloc(MemoryArena *arena, size_t size) {
    size_t aligned_size = (size + 7) & ~7;
    
    if (arena->offset + aligned_size > arena->capacity) {
        return NULL;
    }

    void *ptr = &arena->buffer[arena->offset];
    arena->offset += aligned_size;
    return ptr;
}

void arena_reset(MemoryArena *arena) {
    arena->offset = 0;
}
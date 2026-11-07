#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *buffer;
    size_t capacity;
    size_t offset;
} MemoryArena;

void arena_init(MemoryArena *arena, void *raw_memory, size_t capacity);
void *arena_alloc(MemoryArena *arena, size_t size);
void arena_reset(MemoryArena *arena);

#endif
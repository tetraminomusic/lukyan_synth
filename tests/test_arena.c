#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "arena.h"

int main(void) {
    uint8_t buffer[1024];
    MemoryArena arena;

    arena_init(&arena, buffer, sizeof(buffer));

    void *ptr1 = arena_alloc(&arena, 13);
    assert(ptr1 != NULL);
    assert((uintptr_t)ptr1 % 8 == 0);

    void *ptr2 = arena_alloc(&arena, 27);
    assert(ptr2 != NULL);
    assert((uintptr_t)ptr2 % 8 == 0);
    assert(ptr2 > ptr1);

    void *too_big = arena_alloc(&arena, 2048);
    assert(too_big == NULL);

    arena_reset(&arena);
    void *ptr3 = arena_alloc(&arena, 16);
    assert(ptr3 == ptr1);

    printf("[PASSED] Arena Allocator unit test: Alignment and boundaries verified\n");
    return 0;
}
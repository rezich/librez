#pragma once

#define TOTAL_MEMORY 1024 * 1024 * 1
char* total_memory;

Mem_Arena mem_arena = 0;

size_t mem_end[MEM_ARENA_COUNT] = { 0 };

void mem_init() {
    total_memory = pd->system->realloc(NULL, TOTAL_MEMORY);
    for (int i = 0; i < TOTAL_MEMORY; ++i) total_memory[i] = 0;
}
inline void* mem_alloc(size_t bytes) {
    void* ptr = total_memory + mem_end[mem_arena];
    mem_end[mem_arena] += bytes;
    return ptr;
}
void mem_reset() {
    for (int i = mem_arena; i < MEM_ARENA_COUNT; ++i) {
        mem_end[i] = (i == 0) ? 0 : mem_end[i - 1];
    }
}
void mem_use(Mem_Arena arena) {
    for (int i = 0; i <= arena; ++i) {
        if (mem_end[i] == 0) mem_end[i] = (i == 0) ? 0 : mem_end[i - 1];
    }
    mem_arena = arena;
    mem_reset();
}

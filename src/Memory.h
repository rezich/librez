#pragma once

#ifndef MEM_ARENA_CHUNK_SIZE
#define MEM_ARENA_CHUNK_SIZE 1024 * 1024 * 2
#endif

char* mem_arena_chunk;
Mem_Arena mem_arena = 0;
size_t mem_end[MEM_ARENA_COUNT] = { 0 };

void _mem_init() {
    mem_arena_chunk = pd->system->realloc(NULL, MEM_ARENA_CHUNK_SIZE);
    for (int i = 0; i < MEM_ARENA_CHUNK_SIZE; ++i) mem_arena_chunk[i] = 0;
}
void* mem_alloc(size_t bytes) {
    void* ptr = mem_arena_chunk + mem_end[mem_arena];
    mem_end[mem_arena] += bytes;
    return ptr;
}
void mem_reset() {
    for (int i = mem_arena; i < MEM_ARENA_COUNT; ++i) mem_end[i] = (i == 0) ? 0 : mem_end[i - 1];
}
void mem_use(Mem_Arena arena) {
    for (int i = 0; i <= arena; ++i) if (mem_end[i] == 0) mem_end[i] = (i == 0) ? 0 : mem_end[i - 1];
    mem_arena = arena;
    mem_reset();
}

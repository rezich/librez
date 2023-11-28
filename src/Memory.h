#pragma once

#ifndef MEM_ARENA_HUNK_SIZE
#define MEM_ARENA_HUNK_SIZE 1024 * 1024 * 2
#endif

char      _mem_arena_hunk[MEM_ARENA_HUNK_SIZE] = { 0 };
Mem_Arena _mem_arena                           =   0;
size_t    _mem_end[MEM_ARENA_COUNT]            = { 0 };
size_t    _mem_temp_begin                      =   0;

void* mem_alloc(size_t bytes) {
    assert(_mem_end[_mem_arena] < MEM_ARENA_HUNK_SIZE - 1 - _mem_temp_begin);
    void* ptr = _mem_arena_hunk + _mem_end[_mem_arena];
    _mem_end[_mem_arena] += bytes;
    return ptr;
}
void* mem_alloc_temp(size_t bytes) {
    if (_mem_arena == MEM_ARENA_COUNT - 1) return mem_alloc(bytes);
    assert(MEM_ARENA_HUNK_SIZE - 1 - (int)_mem_temp_begin - (int)bytes > (int)_mem_end[_mem_arena]);
    _mem_temp_begin += bytes;
    return _mem_arena_hunk + MEM_ARENA_HUNK_SIZE - 1 - _mem_temp_begin;
}
void mem_reset() {
    _mem_temp_begin = 0;
    for (int i = _mem_arena; i < MEM_ARENA_COUNT; ++i) _mem_end[i] = (i == 0) ? 0 : _mem_end[i - 1];
}
void mem_use(Mem_Arena arena) {
    for (int i = 0; i <= arena; ++i) if (_mem_end[i] == 0) _mem_end[i] = (i == 0) ? 0 : _mem_end[i - 1];
    _mem_arena = arena;
    mem_reset();
}

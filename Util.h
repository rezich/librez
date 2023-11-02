#pragma once

#define STB_SPRINTF_IMPLEMENTATION
#ifdef _WINDLL
#pragma warning( push )
#pragma warning( disable : 26451 )
#endif
#include "lib/stb_printf.h"
#ifdef _WINDLL
#pragma warning( pop )
#endif
#ifndef _WINDLL
//#pragma GCC diagnostic ignored "-Wformat-extra-args"
//#pragma GCC diagnostic ignored "-Wformat="

//#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

#undef max
#undef min
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

const uint8_t PATTERN_WHITE[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const uint8_t PATTERN_BLACK[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint8_t BAYER[8][8] = {
    { 0, 32,  8, 40,  2, 34, 10, 42},
    {48, 16, 56, 24, 50, 18, 58, 26},
    {12, 44,  4, 36, 14, 46,  6, 38},
    {60, 28, 52, 20, 62, 30, 54, 22},
    { 3, 35, 11, 43,  1, 33,  9, 41},
    {51, 19, 59, 27, 49, 17, 57, 25},
    {15, 47,  7, 39, 13, 45,  5, 37},
    {63, 31, 55, 23, 61, 29, 53, 21}
};

void set_black(LCDPattern* pattern) {
    memcpy(*pattern, PATTERN_BLACK, sizeof(PATTERN_BLACK));
}
void set_white(LCDPattern* pattern) {
    memcpy(*pattern, PATTERN_WHITE, sizeof(PATTERN_WHITE));
}
void set_alpha(LCDPattern* pattern, float alpha) {
    if (alpha == 1.f) { //TODO: fix?!
        uint8_t* dest = (*pattern) + 8 * sizeof(uint8_t);
        memcpy(dest, PATTERN_WHITE, sizeof(PATTERN_WHITE));
        return;
    }
    const uint8_t threshold = (uint8_t)((1.f - alpha) * 63.f);
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (BAYER[row][col] > threshold)
                (*pattern)[8 + row] |= (1 << col); // set
            else
                (*pattern)[8 + row] &= ~(1 << col); // clear
        }
    }
}

float ease(float x, float target, float speed) {
    if (fabsf(x - target) <= 0.01f) return target;
    return x + (target - x) * speed;
}

float accelerated_change(float x) {
    return 1.f / (0.2f + powf(1.04f, -fabsf(x) + 20.f));
}

typedef unsigned long Hash;
Hash hash_string(const char* str) {
    Hash hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}
Hash hash_int(int n) {
    Hash x = (Hash)n;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}
Hash hash_combine(Hash a, Hash b) {
    return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2));
}
Hash hash_pointer(const void* ptr) {
#ifdef TARGET_SIMULATOR
    const int high = (int)((uintptr_t)ptr >> 32);
    const int low  = (int)((uintptr_t)ptr & 0xFFFFFFFF);
    return hash_combine(hash_int(high), hash_int(low));
#else
    return hash_int((int)ptr);
#endif
}

typedef struct {
    unsigned int s;
    unsigned int ms;
} Timestamp;
typedef struct {
    unsigned int s;
    unsigned int ms;
} Timespan;
void timestamp_now(Timestamp* timestamp) {
    timestamp->s = pd->system->getSecondsSinceEpoch(&timestamp->ms);
}
unsigned int timestamp_diff(Timestamp* earlier, Timestamp* later, Timespan* diff) {
    unsigned int ms_diff = (later->s - earlier->s) * 1000 + later->ms - earlier->ms;
    if (diff) {
        diff->s =  ms_diff / 1000;
        diff->ms = ms_diff % 1000;
    }
    return ms_diff;
}
unsigned int timespan_get_ms(Timespan* timespan) { return timespan->s * 1000 + timespan->ms; }

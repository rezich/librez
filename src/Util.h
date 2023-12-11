#pragma once

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
void set_black(LCDPattern* pattern) { memcpy(*pattern, PATTERN_BLACK, sizeof(PATTERN_BLACK)); }
void set_white(LCDPattern* pattern) { memcpy(*pattern, PATTERN_WHITE, sizeof(PATTERN_WHITE)); }
void set_alpha(LCDPattern* pattern, float alpha) {
    const uint8_t threshold = (uint8_t)((1.f - alpha) * 64.f);
    for (int row = 0; row < 8; ++row) for (int col = 0; col < 8; ++col)
        if (BAYER[row][col] >= threshold) (*pattern)[8 + row] |=  (1 << col); // set
        else                              (*pattern)[8 + row] &= ~(1 << col); // clear
}
bool invert(LCDColor* color) {
    if (*color == kColorBlack) { *color = kColorWhite; return true; }
    if (*color == kColorWhite) { *color = kColorBlack; return true; }
    if (*color >  kColorXOR)   { for (int i = 0; i < 8; ++i) color[i] = ~color[i]; return true; }
    return false;
}
bool patternize(LCDColor color, LCDPattern* pattern) {
    if (color == kColorBlack) { set_black(pattern); set_alpha(pattern, 1.f); return true; }
    if (color == kColorWhite) { set_white(pattern); set_alpha(pattern, 1.f); return true; }
    if (color >  kColorXOR)   { memcpy(*pattern, *((LCDPattern*)color), sizeof(LCDPattern)); set_alpha(pattern, 1.f); return true; }
    return false;
}

float ease(float x, float target, float speed) {
    const float EPSILON = 0.01f;
    if (fabsf(x - target) <= EPSILON) return target;
    return x + (target - x) * speed;
}

float accelerated_change(float x) {
    return 1.f / (0.2f + powf(1.04f, -fabsf(x) + 20.f));
}

float get_temporally_cycling_value(float scale) {
    return fmaxf(fminf(cosf((float)pd->system->getCurrentTimeMilliseconds() / 1000.f * scale) * .6f + .5f, 1.0f), 0.f);
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
    x =  (x >> 16) ^ x;
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


//TODO: clean up the date/time stuff below


#define Timestamp(s, ms) (Timestamp){ (s), (ms) }
typedef struct {
    unsigned int s;
    unsigned int ms;
} Timestamp;

#define Timespan(s, ms) (Timespan){ (s), (ms) }
typedef struct {
    unsigned int s;
    unsigned int ms;
} Timespan;

typedef struct {
    unsigned int year;        // 2000+
    unsigned int month;       // 1–12
    unsigned int day;         // 1–31
    unsigned int day_of_week; // 0—6
    unsigned int hour;        // 0—23
    unsigned int minute;      // 0–59
    unsigned int second;      // 0–59
    unsigned int millisecond; // 0–999
} Moment;

const char* WEEKDAY_SHORT[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

FORCE_INLINE bool year_is_leap_year(unsigned int year) { return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0); }

const unsigned int SECONDS_PER_MINUTE      = 60;
const unsigned int MINUTES_PER_HOUR        = 60;
const unsigned int SECONDS_PER_HOUR        = 3600;
const unsigned int HOURS_PER_DAY           = 24;
const unsigned int DAYS_PER_WEEK           = 7;
const unsigned int DAYS_PER_YEAR           = 365;
const unsigned int DAYS_PER_LEAP_YEAR      = 366;
const unsigned int SECONDS_PER_DAY         = 86400;
const unsigned int EPOCH_YEAR              = 2000;
const unsigned int EPOCH_DAY_OF_WEEK       = 6;
const unsigned int MILLISECONDS_PER_SECOND = 1000;

void timestamp_parse(Timestamp timestamp, Moment* moment) {
    unsigned int total_days = timestamp.s / SECONDS_PER_DAY;
    *moment = (Moment){ .year = EPOCH_YEAR, .month = 1 };
    
    while (total_days >= (year_is_leap_year(moment->year) ? DAYS_PER_LEAP_YEAR : DAYS_PER_YEAR)) {
           total_days -= (year_is_leap_year(moment->year) ? DAYS_PER_LEAP_YEAR : DAYS_PER_YEAR);
        ++moment->year;
    }
    const unsigned int   DAYS_IN_MONTH[] = { 31, 28 + (year_is_leap_year(moment->year) ? 1 : 0), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    while (total_days >= DAYS_IN_MONTH[moment->month - 1]) {
           total_days -= DAYS_IN_MONTH[moment->month - 1];
        ++moment->month;
    }

    moment->day = total_days + 1;
    moment->day_of_week = (EPOCH_DAY_OF_WEEK + timestamp.s / SECONDS_PER_DAY) % DAYS_PER_WEEK;

    unsigned int      remaining_seconds = timestamp.s % SECONDS_PER_DAY;
    moment->hour   =  remaining_seconds / (SECONDS_PER_MINUTE * MINUTES_PER_HOUR);
    moment->minute = (remaining_seconds % (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)) / SECONDS_PER_MINUTE;
    moment->second =  remaining_seconds %  SECONDS_PER_MINUTE;
    moment->millisecond = timestamp.ms;

    assert(moment->millisecond <= 999);
    assert(moment->second      <= 59);
    assert(moment->minute      <= 59);
    assert(moment->hour        <= 23);
    assert(moment->day         <= 31);
    assert(moment->day_of_week <= 6);
    assert(moment->month       <= 12);
}

void timestamp_now(Timestamp* timestamp) {
    timestamp->s = pd->system->getSecondsSinceEpoch(&timestamp->ms);
}

unsigned int timestamp_diff(Timestamp* earlier, Timestamp* later, Timespan* diff) {
    unsigned int ms_diff = (later->s - earlier->s) * MILLISECONDS_PER_SECOND + later->ms - earlier->ms;
    if (diff) {
        diff->s  = ms_diff / MILLISECONDS_PER_SECOND;
        diff->ms = ms_diff % MILLISECONDS_PER_SECOND;
    }
    return ms_diff;
}

void timespan_parse(Timespan timespan, unsigned int* days, unsigned int* hours, unsigned int* minutes, unsigned int* seconds, unsigned int* milliseconds) {
    if (days)         *days         = timespan.s / SECONDS_PER_DAY;
    timespan.s                     %= SECONDS_PER_DAY;
    if (hours)        *hours        = timespan.s / SECONDS_PER_HOUR;
    timespan.s                     %= SECONDS_PER_HOUR;
    if (minutes)      *minutes      = timespan.s / SECONDS_PER_MINUTE;
    timespan.s                     %= SECONDS_PER_MINUTE;
    if (seconds)      *seconds      = timespan.s;
    if (milliseconds) *milliseconds = timespan.ms;
}

void timestamp_add_timespan(Timestamp* timestamp, Timespan timespan) {
    timestamp->ms += timespan.ms;
    timestamp->s  += timespan.s + timestamp->ms / MILLISECONDS_PER_SECOND;
    timestamp->ms %= MILLISECONDS_PER_SECOND;
}


#ifdef USING_TIMESCALE
float _timescale = 1.f;
FORCE_INLINE void timescale_set(float ts) { _timescale = ts; }
#define TSDT _timescale * dt
#endif


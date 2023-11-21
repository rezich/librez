#pragma once

#undef M_PI
#undef M_PI_2
#undef M_PI_4
#define M_PI       3.14159265358979323846f   // pi
#define M_PI_2     1.57079632679489661923f   // pi/2
#define M_PI_4     0.785398163397448309616f  // pi/4
#define M_TAU      M_PI * 2.f

#define Point(x, y) (Point){(x), (y)}
typedef struct {
    int x;
    int y;
} Point;
FORCE_INLINE void swap_points(Point* a, Point* b) {
    Point temp = *a;
    *a = *b;
    *b = temp;
}
FORCE_INLINE bool points_are_equal(Point a, Point b) { return a.x == b.x && a.y == b.y; }

const Point LCD_CENTER = { LCD_COLUMNS / 2 - 1, LCD_ROWS / 2 - 1 };

#define Rect(x, y, w, h) (Rect){(Point){(x), (y)}, (Point){(w), (h)}}
typedef union {
    struct {
        Point top_left;
        Point dimensions;
    };
    struct {
        int x;
        int y;
        int w;
        int h;
    };
} Rect;
Point get_top_right(Rect rect) { return Point(
    rect.x + rect.w - 1,
    rect.y
);}
Point get_bottom_right(Rect rect) { return Point(
    rect.x + rect.w - 1,
    rect.y + rect.h - 1
);}
Point get_bottom_left(Rect rect) { return Point(
    rect.x,
    rect.y + rect.h - 1
);}
Point get_center(Rect rect) { return Point(
    rect.top_left.x + rect.dimensions.y / 2,
    rect.top_left.y + rect.dimensions.y / 2
);}


#define Vec2(x, y) (Vec2){(x), (y)}
typedef struct {
    float x;
    float y;
} Vec2;
FORCE_INLINE bool  vec2s_are_equal(Vec2 a, Vec2 b) { return a.x == b.x && a.y == b.y; }
FORCE_INLINE void  vec2_multiply(Vec2* v, float scalar) {
    v->x *= scalar;
    v->y *= scalar;
}
FORCE_INLINE float vec2_length_squared(Vec2 v) { return v.x * v.x + v.y * v.y; }
FORCE_INLINE float vec2_length(Vec2 v) { return sqrtf(vec2_length_squared(v)); }
FORCE_INLINE Point vec2_to_point(Vec2 v) { return Point((int)roundf(v.x), (int)roundf(v.y)); }
FORCE_INLINE Vec2  point_to_vec2(Point p) { return Vec2((float)p.x, (float)p.y); }
void vec2_normalize(Vec2* v) {
    const float length = vec2_length(*v);
    if (length <= 0.f) *v = Vec2(0, 0);
    else vec2_multiply(v, 1.f / length);
}
FORCE_INLINE Vec2 ease_vec2(Vec2 p, Vec2 target, float speed) { return (Vec2) { ease(p.x, target.x, speed), ease(p.y, target.y, speed) }; }
FORCE_INLINE Vec2 ease_vec2_to_point(Vec2 p, Point target, float speed) { return (Vec2) { ease(p.x, (float)target.x, speed), ease(p.y, (float)target.y, speed) }; }

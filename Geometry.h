#pragma once

#define Point(x, y) (Point) {(x), (y)}
typedef struct {
    int x;
    int y;
} Point;
void swap_points(Point* a, Point* b) {
    Point temp = *a;
    *a = *b;
    *b = temp;
}


#define Rect(x, y, w, h) (Rect) {(Point){(x), (y)}, (Point){(w), (h)}}
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


#define Vec2(x, y) (Vec2) {(x), (y)}
typedef struct {
    float x;
    float y;
} Vec2;
inline void  vec2_multiply(Vec2* v, float scalar) {
    v->x *= scalar;
    v->y *= scalar;
}
inline float vec2_length_squared(Vec2 v) { return v.x * v.x + v.y * v.y; }
inline float vec2_length(Vec2 v) { return sqrtf(vec2_length_squared(v)); }
inline Point vec2_to_point(Vec2 v) { return Point((int)roundf(v.x), (int)roundf(v.y)); }
inline void  vec2_normalize(Vec2* v) {
    const float length = vec2_length(*v);
    if (length <= 0.f) *v = Vec2(0, 0);
    else vec2_multiply(v, 1.f / length);
}

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
#pragma once

#ifdef TARGET_SIMULATOR
#define RETURN_IF_FAKING_DEBUG_RENDERING
#else
#define RETURN_IF_FAKING_DEBUG_RENDERING if (context_faking_debug) return
#endif

#ifdef USING_CUSTOM_RENDERER
typedef struct {
    int width;
    int height;
    int rowbytes;
    uint8_t* mask;
    uint8_t* data;
} Draw_Buffer;

uint8_t* backbuffer;
Draw_Buffer draw_buffer;

#define CONTEXT_STACK_SIZE 100
LCDBitmap* context_stack[CONTEXT_STACK_SIZE];
size_t context_stack_count = 0;
#ifndef TARGET_SIMULATOR
bool context_faking_debug = false;
#endif

Rect clip_rect;
bool clip_rect_active = false;

void _set_draw_buffer_to_backbuffer() {
    draw_buffer.width    = LCD_COLUMNS;
    draw_buffer.height   = LCD_ROWS;
    draw_buffer.rowbytes = LCD_ROWSIZE;
    draw_buffer.mask     = NULL; // ?
    draw_buffer.data     = backbuffer;
}
void _set_draw_buffer_to_end_of_context_stack() {
    assert(context_stack_count > 0);
    pd->graphics->getBitmapData(context_stack[context_stack_count-1], &draw_buffer.width, &draw_buffer.height, &draw_buffer.rowbytes, &draw_buffer.mask, &draw_buffer.data);
}

void _begin_rendering() {
    backbuffer = pd->graphics->getFrame();
    context_stack_count = 0;
    clip_rect_active = false;
#ifndef TARGET_SIMULATOR
    context_faking_debug = false;
#endif
    _set_draw_buffer_to_backbuffer();
}
#endif

inline void renderer_mark_updated_rows(int start, int end) {
    pd->graphics->markUpdatedRows(max(start, 0), min(end, LCD_ROWS - 1));
}

inline LCDBitmap* new_bitmap(int width, int height, LCDColor bgcolor) { return pd->graphics->newBitmap(width, height, bgcolor); }
inline void free_bitmap(LCDBitmap* bitmap) { pd->graphics->freeBitmap(bitmap); }
inline void draw_bitmap(LCDBitmap * bitmap, int x, int y) { pd->graphics->drawBitmap(bitmap, x, y, kBitmapUnflipped); }

inline void clear(LCDColor color) { pd->graphics->clear(color); }

inline void clip_set(Rect r) {
    pd->graphics->setScreenClipRect(r.x, r.y, r.w, r.h);
#ifdef USING_CUSTOM_RENDERER
    clip_rect = r;
    clip_rect_active = true;
#endif
}
inline void clip_clear() {
    pd->graphics->clearClipRect();
#ifdef USING_CUSTOM_RENDERER
    clip_rect_active = false;
#endif
}

inline void target_push(LCDBitmap* target) {
    pd->graphics->pushContext(target);
#ifdef USING_CUSTOM_RENDERER
    context_stack[context_stack_count++] = target;
    if (target) _set_draw_buffer_to_end_of_context_stack();
    else _set_draw_buffer_to_backbuffer();
#endif
}
inline void target_push_debug() {
#ifdef TARGET_SIMULATOR
    target_push(pd->graphics->getDebugBitmap());
#else
    context_faking_debug = true;
#endif
}
inline void target_pop() {
#ifndef TARGET_SIMULATOR
    if (context_faking_debug) { context_faking_debug = false; return; }
#endif
#ifdef USING_CUSTOM_RENDERER
    if (context_stack_count == 0) {
        _set_draw_buffer_to_backbuffer();
        return;
    }
#endif
    pd->graphics->popContext();
#ifdef USING_CUSTOM_RENDERER
    --context_stack_count;
    if (context_stack_count == 0) {
        _set_draw_buffer_to_backbuffer();
        return;
    }
    _set_draw_buffer_to_end_of_context_stack();
#endif
    return;
}

#ifdef USING_CUSTOM_RENDERER
inline void _set_point(bool mask, int x, int y, LCDColor color) {
    if (x < 0 || x >= draw_buffer.width || y < 0 || y >= draw_buffer.height) return;
    if (clip_rect_active) {
        if (x < clip_rect.x || x >= clip_rect.x + clip_rect.w || y < clip_rect.y || y >= clip_rect.y + clip_rect.h) return;
    }
    uint8_t* row = (mask ? draw_buffer.mask : draw_buffer.data) + ((size_t)y * draw_buffer.rowbytes);
    uint8_t* byte = &row[x / 8];
    if (color > kColorXOR) {
        LCDPattern* pattern = (LCDPattern*)color;
        if (!((*pattern)[8 + (y % 8)] & (1 << (x % 8)))) return; // skip if masked
        color = (!((*pattern)[(y % 8)] & (1 << (x % 8)))) ? kColorBlack : kColorWhite;
    }
    const uint8_t pixel = 0x80 >> (x % 8);
    switch ((LCDSolidColor)color) {
    case kColorBlack: *byte &= ~pixel; break;
    case kColorWhite: *byte |=  pixel; break;
    case kColorClear: assert(false); // should never occur
    case kColorXOR:   *byte ^=  pixel; break;
    }
}

inline void _draw_point(int x, int y, LCDColor color) {
    if (color == kColorClear) {
        if (draw_buffer.mask) _set_point(true, x, y, kColorBlack);
        return;
    }
    if (draw_buffer.mask) _set_point(true, x, y, kColorWhite);
    _set_point(false, x, y, color);
}

inline void _draw_hline(int x, int y, int width, LCDColor color) {
    if (color == kColorClear) return;
    pd->graphics->fillRect(x, y, width, 1, color);
}

inline void _draw_vline(int x, int y, int height, LCDColor color) {
    if (color == kColorClear) return;
    pd->graphics->fillRect(x, y, 1, height, color);
}

inline void _draw_line(Point a, Point b, LCDColor color) {
    if (color == kColorClear) return;
    /*     if (a.x == b.x) { _draw_vline(a.x, a.y, b.y - a.y, color); return; }
    else if (a.y == b.y) { _draw_hline(a.x, a.y, b.x - a.x, color); return; }*/
    const int dx = b.x - a.x;
    const int dy = b.y - a.y;
    const int n = max(abs(dx), abs(dy));
    const float div_n = (n == 0) ? 0.f : 1.f / (float)n;
    const float xstep = (float)dx * div_n;
    const float ystep = (float)dy * div_n;
    float fx = (float)a.x;
    float fy = (float)a.y;
    for (int step = 0; step <= n; ++step, fx += xstep, fy += ystep) {
        _draw_point((int)roundf(fx), (int)roundf(fy), color);
    }
}
#endif

inline void draw_point(Point p, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
#ifdef USING_CUSTOM_RENDERER
    _draw_point(p.x, p.y, color);
    renderer_mark_updated_rows(p.y, p.y);
#else
    pd->graphics->drawLine(p.x, p.y, p.x, p.y, 0, color);
#endif
}

inline void draw_line(Point a, Point b, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
#ifdef USING_CUSTOM_RENDERER
    _draw_line(a, b, color);
    renderer_mark_updated_rows(min(a.y, b.y), max(a.y, b.y));
#else
    pd->graphics->drawLine(a.x, a.y, b.x, b.y, 0, color);
#endif
}

inline void draw_rect(Rect r, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    pd->graphics->fillRect(r.x, r.y, r.w, r.h, color);
}
void draw_rect_outline(Rect r, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
#ifdef USING_CUSTOM_RENDERER
    Point top_left = r.top_left;
    Point top_right = get_top_right(r);
    Point bottom_right = get_bottom_right(r);
    Point bottom_left = get_bottom_left(r);
    _draw_line(top_left, top_right, color);
    _draw_line(bottom_left, bottom_right, color);

    // 
    top_left.y     += 1;
    top_right.y    += 1;
    bottom_left.y  -= 1;
    bottom_right.y -= 1;

    _draw_line(top_right, bottom_right, color);
    _draw_line(top_left, bottom_left, color);

    renderer_mark_updated_rows(r.y, r.y + r.h - 1);
#else
    pd->graphics->drawRect(r.x, r.y, r.w, r.h, color);
#endif
}

void draw_triangle(Point a, Point b, Point c, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
//#ifdef USING_CUSTOM_RENDERER
    if (b.y < a.y) swap_points(&b, &a);
    if (c.y < a.y) swap_points(&c, &a);
    if (c.y < b.y) swap_points(&c, &b);
    //TODO (ugh)
//#else
    pd->graphics->fillTriangle(a.x, a.y, b.x, b.y, c.x, c.y, color);
//#endif
}
void draw_triangle_outline(Point a, Point b, Point c, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
#ifdef USING_CUSTOM_RENDERER
    _draw_line(a, b, color);
    _draw_line(b, c, color);
    _draw_line(c, a, color);
    //TODO: don't overdraw the corners!
    renderer_mark_updated_rows(
        min(min(a.y, b.y), c.y),
        max(max(a.y, b.y), c.y)
    );
#else
    pd->graphics->drawLine(a.x, a.y, b.x, b.y, 0, color);
    pd->graphics->drawLine(b.x, b.y, c.x, c.y, 0, color);
    pd->graphics->drawLine(c.x, c.y, a.x, a.y, 0, color);
#endif
}

inline void draw_ellipse(Rect r, float angle_start, float angle_end, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    pd->graphics->fillEllipse(r.x, r.y, r.w, r.h, angle_start, angle_end, color);
}
inline void draw_ellipse_outline(Rect r, float angle_start, float angle_end, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    pd->graphics->drawEllipse(r.x, r.y, r.w, r.h, 0, angle_start, angle_end, color);
}

inline void draw_circle(Point center, int diameter, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    int radius = diameter / 2;
    pd->graphics->fillEllipse(center.x - radius, center.y - radius, diameter, diameter, 0, 360, color);
}
inline void draw_circle_outline(Point center, int diameter, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    int radius = diameter / 2;
    pd->graphics->drawEllipse(center.x - radius, center.y - radius, diameter, diameter, 0, 0, 360, color);
}

void draw_text(Point top_left, const char* text, const Typesetting* typesetting) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    if (!typesetting) typesetting = &DEFAULT_TYPESETTING;

    Point glyph_size = Point(GLYPH_SIZE_MINIMUM.x + typesetting->glyph_size.x * 2, GLYPH_SIZE_MINIMUM.y + typesetting->glyph_size.y * 2);
    //Point unit_scale = Point((glyph_size.x - 1) / 2, (glyph_size.y - 1) / 2);
    Point cursor = Point(top_left.x, top_left.y);

    for (int text_index = 0; text[text_index] != 0; ++text_index) {
        if (text[text_index] == '\n') {
            cursor = Point(top_left.x, cursor.y + typesetting->line_spacing);
            continue;
        }

        Point shiver_offset = { 0,0 };
        if  (typesetting->glyph_shiver > 0) shiver_offset = Point(
            -typesetting->glyph_shiver + (rand() % (typesetting->glyph_shiver * 2)),
            -typesetting->glyph_shiver + (rand() % (typesetting->glyph_shiver * 2))
        );

        const Glyph* glyph = &THE_FONT[text[text_index] - FONT_ASCII_OFFSET];
        int stride = glyph_size.x;
        
        for (int stroke_index = 0; stroke_index < glyph->stroke_count; ++stroke_index) {
            const Stroke* stroke = &glyph->strokes[stroke_index];
            const Point start_point = stroke_point(stroke->start, glyph_size);
            const Point end_point   = stroke_point(stroke->end,   glyph_size);
            draw_line(
                Point(
                    cursor.x + start_point.x + shiver_offset.x,
                    cursor.y + start_point.y + shiver_offset.y
                ),
                Point(
                    cursor.x + end_point.x   + shiver_offset.x,
                    cursor.y + end_point.y   + shiver_offset.y
                ),
                typesetting->color
            );
        }

        cursor.x += stride + typesetting->glyph_spacing;
    }
}

/*
reference for how rendering would work (kinda) if shaders were a thing

static bool pixel_is_black(int x, int y) {
    return true;
}

void renderer_render() {
    uint8_t* buffer = pd->graphics->getFrame();
    for (int y = 0; y < LCD_ROWS; ++y) {
        char b = 0;
        int bitpos = 0x80;
        uint8_t* row_buffer = &buffer[y * LCD_ROWSIZE];
        for (int x = 0; x < LCD_COLUMNS; ++x) {
            if (pixel_is_black(x, y)) {
                b |= bitpos;
            }
            bitpos >>= 1;
            if (bitpos == 0) {
                row_buffer[x / 8] = ~b;
                b = 0;
                bitpos = 0x80;
            }
        }
    }
    pd->graphics->markUpdatedRows(0, LCD_ROWS);
}*/

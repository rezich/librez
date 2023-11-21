#pragma once

#ifdef TARGET_SIMULATOR
#define RETURN_IF_FAKING_DEBUG_RENDERING
#else
#define RETURN_IF_FAKING_DEBUG_RENDERING if (context_faking_debug) return
#endif

#ifdef USING_CUSTOM_RENDERER
typedef struct {
    int      width;
    int      height;
    int      rowbytes;
    uint8_t* mask;
    uint8_t* data;
} Draw_Buffer;

uint8_t* backbuffer;
Draw_Buffer draw_buffer;
LCDBitmap* debug_bitmap = NULL;

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
#ifdef TARGET_SIMULATOR
    debug_bitmap = pd->graphics->getDebugBitmap();
#else
    context_faking_debug = false;
    
#endif
    _set_draw_buffer_to_backbuffer();
}
#endif

FORCE_INLINE void renderer_mark_updated_rows(int start, int end) { pd->graphics->markUpdatedRows(max(start, 0), min(end, LCD_ROWS - 1)); }

FORCE_INLINE LCDBitmap* new_bitmap(Point dimensions, LCDColor bgcolor) { return pd->graphics->newBitmap(dimensions.x, dimensions.y, bgcolor); }
FORCE_INLINE LCDBitmap* copy_bitmap(LCDBitmap* bitmap) { return pd->graphics->copyBitmap(bitmap); }
FORCE_INLINE void free_bitmap(LCDBitmap* bitmap) { pd->graphics->freeBitmap(bitmap); }
FORCE_INLINE void draw_bitmap(LCDBitmap* bitmap, Point position) { pd->graphics->drawBitmap(bitmap, position.x, position.y, kBitmapUnflipped); }
FORCE_INLINE void draw_bitmap_scaled(LCDBitmap* bitmap, Point position, Vec2 scale) { pd->graphics->drawScaledBitmap(bitmap, position.x, position.y, scale.x, scale.y); }
FORCE_INLINE void set_bitmap_mask(LCDBitmap* bitmap, LCDBitmap* mask) { pd->graphics->setBitmapMask(bitmap, mask); }
FORCE_INLINE LCDBitmap* get_bitmap_mask(LCDBitmap* bitmap) { return pd->graphics->getBitmapMask(bitmap); }

FORCE_INLINE void clear(LCDColor color) { pd->graphics->clear(color); }

FORCE_INLINE void clip_set(Rect r) {
    pd->graphics->setScreenClipRect(r.x, r.y, r.w, r.h);
#ifdef USING_CUSTOM_RENDERER
    clip_rect = r;
    clip_rect_active = true;
#endif
}
FORCE_INLINE void clip_clear() {
    pd->graphics->clearClipRect();
#ifdef USING_CUSTOM_RENDERER
    clip_rect_active = false;
#endif
}

FORCE_INLINE void target_push(LCDBitmap* target) {
    pd->graphics->pushContext(target);
#ifdef USING_CUSTOM_RENDERER
    context_stack[context_stack_count++] = target;
    if (target) _set_draw_buffer_to_end_of_context_stack();
    else _set_draw_buffer_to_backbuffer();
#endif
}
FORCE_INLINE void target_push_debug() {
#ifdef TARGET_SIMULATOR
    target_push(debug_bitmap);
#else
    context_faking_debug = true;
#endif
}
FORCE_INLINE void target_pop() {
#ifndef TARGET_SIMULATOR
    if (context_faking_debug) { context_faking_debug = false; return; }
#endif
#ifdef USING_CUSTOM_RENDERER
    if (context_stack_count == 0) { _set_draw_buffer_to_backbuffer(); return; }
#endif
    pd->graphics->popContext();
#ifdef USING_CUSTOM_RENDERER
    --context_stack_count;
    if (context_stack_count == 0) { _set_draw_buffer_to_backbuffer(); return; }
    _set_draw_buffer_to_end_of_context_stack();
#endif
    return;
}

#ifdef USING_CUSTOM_RENDERER
FORCE_INLINE void _set_point(bool mask, int x, int y, LCDColor color) {
    if ((x < 0 || x >= draw_buffer.width   ||
         y < 0 || y >= draw_buffer.height) ||
        (clip_rect_active && (
            x < clip_rect.x || x >= clip_rect.x + clip_rect.w ||
            y < clip_rect.y || y >= clip_rect.y + clip_rect.h))) return;
    uint8_t* row = (mask ? draw_buffer.mask : draw_buffer.data) + ((size_t)y * draw_buffer.rowbytes);
    uint8_t* byte = &row[x / 8];
    if (color > kColorXOR) {
        LCDPattern* pattern = (LCDPattern*)color;
        if      (!((*pattern)[8 + (y % 8)] & (1 << (x % 8)))) return; // skip if masked
        color = (!((*pattern)[    (y % 8)] & (1 << (x % 8)))) ? kColorBlack : kColorWhite;
    }
    const uint8_t pixel = 0x80 >> (x % 8);
    switch ((LCDSolidColor)color) {
    case kColorBlack: *byte &= ~pixel; break;
    case kColorWhite: *byte |=  pixel; break;
    case kColorClear: assert(false); // should never occur
    case kColorXOR:   *byte ^=  pixel; break;
    }
}

FORCE_INLINE void _draw_point(int x, int y, LCDColor color) {
    if (color == kColorClear) {
        if (draw_buffer.mask) _set_point(true, x, y, kColorBlack);
        else assert(false);
        return;
    }
    if (draw_buffer.mask) _set_point(true, x, y, kColorWhite);
    _set_point(false, x, y, color);
}

FORCE_INLINE void _draw_hline(int x, int y, int width, LCDColor color) {
    if (color == kColorClear) return;
    pd->graphics->fillRect(x, y, width, 1, color);
}

FORCE_INLINE void _draw_vline(int x, int y, int height, LCDColor color) {
    if (color == kColorClear) return;
    pd->graphics->fillRect(x, y, 1, height, color);
}

FORCE_INLINE void _draw_line(Point a, Point b, LCDColor color) {
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

FORCE_INLINE void draw_point(Point p, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
#ifdef USING_CUSTOM_RENDERER
    _draw_point(p.x, p.y, color);
    renderer_mark_updated_rows(p.y, p.y);
#else
    pd->graphics->drawLine(p.x, p.y, p.x, p.y, 0, color);
#endif
}

FORCE_INLINE void draw_line(Point a, Point b, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
#ifdef USING_CUSTOM_RENDERER
    _draw_line(a, b, color);
    renderer_mark_updated_rows(min(a.y, b.y), max(a.y, b.y));
#else
    pd->graphics->drawLine(a.x, a.y, b.x, b.y, 0, color);
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

FORCE_INLINE void draw_ellipse(Rect r, float angle_start, float angle_end, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    pd->graphics->fillEllipse(r.x, r.y, r.w, r.h, angle_start, angle_end, color);
}
FORCE_INLINE void draw_ellipse_outline(Rect r, float angle_start, float angle_end, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    pd->graphics->drawEllipse(r.x, r.y, r.w, r.h, 0, angle_start, angle_end, color);
}

FORCE_INLINE void draw_circle(Point center, int diameter, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    int radius = diameter / 2;
    pd->graphics->fillEllipse(center.x - radius, center.y - radius, diameter, diameter, 0, 360, color);
}
FORCE_INLINE void draw_circle_outline(Point center, int diameter, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    int radius = diameter / 2;
    pd->graphics->drawEllipse(center.x - radius, center.y - radius, diameter, diameter, 0, 0, 360, color);
}

FORCE_INLINE void draw_rect(Rect r, int roundness, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    if (roundness == 0) { pd->graphics->fillRect(r.x, r.y, r.w, r.h, color); return; }
    const int roundness2 = roundness * 2;

    const Rect top = Rect(r.x + roundness,        r.y,                    r.w - roundness2, roundness       );
    const Rect mid = Rect(r.x,                    r.y + roundness,        r.w,              r.h - roundness2);
    const Rect bot = Rect(r.x + roundness,        r.y + r.h - roundness,  r.w - roundness2, roundness       );
    const Rect nw  = Rect(r.x + r.w - roundness2, r.y,                    roundness2,       roundness2      );
    const Rect sw  = Rect(r.x + r.w - roundness2, r.y + r.h - roundness2, roundness2,       roundness2      );
    const Rect se  = Rect(r.x,                    r.y + r.h - roundness2, roundness2,       roundness2      );
    const Rect ne  = Rect(r.x,                    r.y,                    roundness2,       roundness2      );

    pd->graphics->fillRect(top.x, top.y, top.w, top.h, color);
    pd->graphics->fillRect(mid.x, mid.y, mid.w, mid.h, color);
    pd->graphics->fillRect(bot.x, bot.y, bot.w, bot.h, color);
    draw_ellipse(nw,   0.f,  90.f, color);
    draw_ellipse(sw,  90.f, 180.f, color);
    draw_ellipse(se, 180.f, 270.f, color);
    draw_ellipse(ne, 270.f, 360.f, color);

}
void draw_rect_outline(Rect r, int roundness, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    if (roundness == 0) {
#ifdef USING_CUSTOM_RENDERER
        Point top_left     = r.top_left;
        Point top_right    = get_top_right(r);
        Point bottom_right = get_bottom_right(r);
        Point bottom_left  = get_bottom_left(r);
        _draw_line(   top_left,    top_right, color);
        _draw_line(bottom_left, bottom_right, color);

        top_left.y     += 1;
        top_right.y    += 1;
        bottom_left.y  -= 1;
        bottom_right.y -= 1;

        _draw_line(top_right, bottom_right, color);
        _draw_line( top_left,  bottom_left, color);

        renderer_mark_updated_rows(r.y, r.y + r.h - 1);
#else
        pd->graphics->drawRect(r.x, r.y, r.w, r.h, color);
#endif
        return;
    }
    const int roundness2 = roundness * 2;
    {
        const int xw1 = r.x + r.w - 1;
        const int yh1 = r.y + r.h - 1;
        const int xr  = r.x + roundness;
        const int yr  = r.y + roundness;

        _draw_line(Point(xr,  r.y), Point(xw1 - roundness, r.y            ), color);
        _draw_line(Point(xr,  yh1), Point(xw1 - roundness, yh1            ), color);
        _draw_line(Point(r.x,  yr), Point(r.x            , yh1 - roundness), color);
        _draw_line(Point(xw1,  yr), Point(xw1            , yh1 - roundness), color);
    }{
        const Rect nw = Rect(r.x + r.w - roundness2, r.y,                    roundness2, roundness2);
        const Rect sw = Rect(r.x + r.w - roundness2, r.y + r.h - roundness2, roundness2, roundness2);
        const Rect se = Rect(r.x,                    r.y + r.h - roundness2, roundness2, roundness2);
        const Rect ne = Rect(r.x,                    r.y,                    roundness2, roundness2);

        draw_ellipse_outline(nw,   0.f,  90.f, color);
        draw_ellipse_outline(sw,  90.f, 180.f, color);
        draw_ellipse_outline(se, 180.f, 270.f, color);
        draw_ellipse_outline(ne, 270.f, 360.f, color);
    }
}

#ifdef USING_CUSTOM_GLYPHS
#define CUSTOM_GLYPH_NAME_MAX_LENGTH 32
//      width                      position glyph_size color
typedef   int (*Custom_Glyph_Proc)(Point,   Point,     LCDColor);

// string -> Custom_Glyph_Proc hashmap
struct { char* key; Custom_Glyph_Proc value; } *custom_glyphs = NULL;

FORCE_INLINE void custom_glyph_set(const char* name, Custom_Glyph_Proc proc) { shput(custom_glyphs, name, proc); }
FORCE_INLINE Custom_Glyph_Proc custom_glyph_get(const char* name) { return shget(custom_glyphs, name); }

#endif

void draw_glyph(Point top_left, char c, Point glyph_size, LCDColor color) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    const Glyph* glyph = &THE_FONT[c - FONT_ASCII_OFFSET];

    for (int stroke_index = 0; stroke_index < glyph->stroke_count; ++stroke_index) {
        const Stroke* stroke = &glyph->strokes[stroke_index];
        const Point start_point = stroke_point(stroke->start, glyph_size);
        const Point end_point = stroke_point(stroke->end, glyph_size);
        draw_line(
            Point(
                top_left.x + start_point.x,
                top_left.y + start_point.y
            ),
            Point(
                top_left.x + end_point.x,
                top_left.y + end_point.y
            ),
            color
        );
    }
}

void draw_text(Point top_left, const char* text, const Typesetting* typesetting) {
    RETURN_IF_FAKING_DEBUG_RENDERING;
    if (!typesetting) typesetting = &DEFAULT_TYPESETTING;

    Point glyph_size = Point(GLYPH_SIZE_MINIMUM.x + typesetting->glyph_size.x * 2, GLYPH_SIZE_MINIMUM.y + typesetting->glyph_size.y * 2);
    //Point unit_scale = Point((glyph_size.x - 1) / 2, (glyph_size.y - 1) / 2);
    Point cursor = Point(top_left.x, top_left.y);

    for (size_t text_index = 0; text[text_index] != 0; ++text_index) {
        if (text[text_index] == '\n') {
            cursor = Point(top_left.x, cursor.y + glyph_size.y + typesetting->line_spacing);
            continue;
        }

        Point shiver_offset = { 0,0 };
        if  (typesetting->glyph_shiver > 0) shiver_offset = Point(
            -typesetting->glyph_shiver + (rand() % (typesetting->glyph_shiver * 2)),
            -typesetting->glyph_shiver + (rand() % (typesetting->glyph_shiver * 2))
        );

        int stride = 0;

        bool markup_parsed = false;
#ifdef USING_CUSTOM_GLYPHS
        if (text[text_index] == '[' && text[text_index + 1] == '[') {
            const char* markup_begin = &text[text_index + 2];
            const char* markup_end = strstr(markup_begin, "]]");
            if (markup_end) {
                char name[CUSTOM_GLYPH_NAME_MAX_LENGTH] = { 0 };
                size_t length = markup_end - markup_begin;
                assert(length < CUSTOM_GLYPH_NAME_MAX_LENGTH);
                strncpy(name, markup_begin, length);

                Custom_Glyph_Proc proc = custom_glyph_get(name);
                if (proc) stride = proc(cursor, glyph_size, typesetting->color);

                text_index += length + 3;
                markup_parsed = true;
            }
        }
#endif

        if (!markup_parsed) {
            stride = glyph_size.x;
            const Point position = { cursor.x + shiver_offset.x, cursor.y + shiver_offset.y };
            draw_glyph(position, text[text_index], glyph_size, typesetting->color);
        }
        cursor.x += stride + (stride != 0 ? typesetting->glyph_spacing : 0);
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

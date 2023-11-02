#pragma once

#define UIID_STACK_SIZE 20
typedef Hash UIID;

UIID   uiid_hot;
bool   ui_hot_is_active;

UIID   uiid_stack[UIID_STACK_SIZE];
size_t uiid_stack_pointer = 0;

typedef enum {
    FROM_INT,
    FROM_STRING,
    FROM_POINTER
} UIID_FROM;

void push_uiid(const void* whatever, UIID from) {
    Hash hash = 0;
    switch (from) {
    case FROM_INT:     hash = hash_int(*(const int*)whatever);     break;
    case FROM_STRING:  hash = hash_string((const char *)whatever); break;
    case FROM_POINTER: hash = hash_pointer(whatever);              break;
    }
    uiid_stack[uiid_stack_pointer++] = hash;
}
void pop_uiid() { --uiid_stack_pointer; }
UIID get_uiid(const void* whatever, UIID_FROM from) {
    push_uiid(whatever, from);
    UIID uiid = 0;
    for (size_t i = 0; i < uiid_stack_pointer; ++i) uiid = hash_combine(uiid, uiid_stack[i]);
    pop_uiid();
    // TEMPORARY (?):
    if (uiid_hot == 0) uiid_hot = uiid;
    return uiid;
}

#define UI_MAX_SELECTABLES 100
#define UI_Selectable(rect, uiid) (UI_Selectable) { (rect), (uiid) };
typedef struct {
    Rect rect;
    UIID uiid;
} UI_Selectable;
UI_Selectable ui_selectables[UI_MAX_SELECTABLES];
size_t        ui_selectables_next = 0;

typedef enum {
    CD_UP,
    CD_RIGHT,
    CD_DOWN,
    CD_LEFT
} Cardinal_Direction;

void _UI_navigate() {
    Point direction;
    if (input_get(IA_UI_UP   )) direction = Point( 0, -1); else
    if (input_get(IA_UI_DOWN )) direction = Point( 0,  1); else 
    if (input_get(IA_UI_LEFT )) direction = Point(-1,  0); else
    if (input_get(IA_UI_RIGHT)) direction = Point( 1,  0); else return;

    Point pos = { 0 };
    for (size_t i = 0; i < ui_selectables_next; ++i) {
        UI_Selectable selectable = ui_selectables[i];
        if (selectable.uiid == uiid_hot) {
            pos = get_center(selectable.rect);
            break;
        }
    }

    UIID to = uiid_hot;
    float closest_distance = FLT_MAX;
    for (size_t i = 0; i < ui_selectables_next; ++i) {
        UI_Selectable* selectable = &ui_selectables[i];
        if (selectable->uiid == uiid_hot) continue;
        Rect rect = selectable->rect;
        float r0x = (float)rect.x;
        float r0y = (float)rect.y;
        float r1x = r0x + (float)rect.w;
        float r1y = r0y + (float)rect.h;
        float px = (float)pos.x;
        float py = (float)pos.y;
        float dx = (float)direction.x;
        float dy = (float)direction.y;
        float tMinX = (r0x - px) / dx;
        float tMaxX = (r1x - px) / dx;
        float tMinY = (r0y - py) / dy;
        float tMaxY = (r1y - py) / dy;
        float tEnter = fmaxf(fminf(tMinX, tMaxX), fminf(tMinY, tMaxY));
        float tExit  = fminf(fmaxf(tMinX, tMaxX), fmaxf(tMinY, tMaxY));

        if (tEnter < tExit && tExit > 0 && tEnter < closest_distance) {
            closest_distance = tEnter;
            to = selectable->uiid;
        }
    }
    uiid_hot = to;
}

void UI_reset() {
    _UI_navigate();
    if (input_get(IA_UI_ACTION)) {
        ui_hot_is_active = true;
    }
    ui_selectables_next = 0;
}



typedef enum {
    UI_CLIP_ALL,
    UI_CLIP_CONTEXT,
    UI_CLIP_NONE
} UI_Clipping;
typedef struct {
    LCDColor fg;
    LCDColor bg;
    Point padding;
    UI_Clipping clipping;
} UI_Style;

#define UI_MAX_COLUMNS 20
typedef struct {
    int    count;
    int    widths[UI_MAX_COLUMNS];
} UI_Columns;

#define UI_Context(rect, style) (UI_Context) { (rect), (style), Point(0, 0), {1, {0}}, 0, 1, 0, false }
typedef struct {
    Rect       rect;
    UI_Style*  style;
    Point      cursor;
    UI_Columns columns;
    int        column_cursor;
    int        colspan;
    int        current_row_max_height;
    bool       interaction_disabled;
} UI_Context;

#define    UI_CONTEXT_STACK_SIZE 20
UI_Context ui_context_stack[UI_CONTEXT_STACK_SIZE];
size_t     ui_context_stack_pointer = 0;
void ui_push_context(UI_Context context) {
    ui_context_stack[ui_context_stack_pointer++] = context;
}
void ui_pop_context() { --ui_context_stack_pointer; }
UI_Context* ui_get_context() {
    assert(ui_context_stack_pointer != 0);
    return &ui_context_stack[ui_context_stack_pointer - 1];
}
void ui_pad_vertical() {
    UI_Context* ctx = ui_get_context();
    int padding = ctx->style->padding.y;
    ctx->cursor.y += padding;
}
void ui_advance_row() {
    UI_Context* ctx = ui_get_context();
    if (ctx->current_row_max_height == 0) return; // don't advance if you don't have to
    ctx->cursor.y += ctx->current_row_max_height;
    ctx->current_row_max_height = 0;
    ui_pad_vertical();
}
Rect ui_widget_begin() {
    UI_Context* ctx = ui_get_context();
    int width = 0;
    int x = ctx->rect.x;
    if (ctx->colspan == 1 && ctx->columns.count == 1) {
        width = ctx->rect.w;
    }
    else {
        for (int i = 0; i < ctx->colspan; ++i) {
            width += ctx->columns.widths[ctx->column_cursor + i] + ctx->style->padding.x;
        }
        for (int i = 0; i < ctx->column_cursor; ++i) x += ctx->columns.widths[i] + ctx->style->padding.x;
        width -= ctx->style->padding.x;
    }

    int y = ctx->rect.y + ctx->cursor.y;
    return Rect(
        x,
        y,
        width,
        ctx->rect.h - y
    );
}
void ui_widget_end(int height) {
    UI_Context* ctx = ui_get_context();
    int previous_column = ctx->column_cursor;
    ctx->column_cursor += ctx->colspan;
    ctx->column_cursor %= ctx->columns.count;

    ctx->current_row_max_height = max(ctx->current_row_max_height, height);
    ctx->colspan = 1;

    if (ctx->column_cursor <= previous_column) {
        ui_advance_row();
    }
}
Rect ui_selectable_widget_begin(UIID uiid) {
    assert(ui_selectables_next + 1 < UI_MAX_SELECTABLES);
    Rect rect = ui_widget_begin();
    if (!(ui_get_context())->interaction_disabled) ui_selectables[ui_selectables_next] = UI_Selectable(rect, uiid);
    return rect;
}
void ui_selectable_widget_end(int height) {
    if (!(ui_get_context())->interaction_disabled) ui_selectables[ui_selectables_next++].rect.h = height;
    ui_widget_end(height);
}


UIID uiid_selected;
void UI_Begin(const char* label, Rect rect, UI_Style* style) {
    assert(style != NULL);
    const UI_Context ctx = UI_Context(rect, style);
    ui_push_context(ctx);
    push_uiid(label, FROM_STRING);
    clip_set(ctx.rect);
}
void UI_End() {
    pop_uiid();
    ui_pop_context();
    if (ui_context_stack_pointer == 0)
        clip_clear();
    else {
        const UI_Context* ctx = ui_get_context();
        clip_set(ctx->rect);
    }
}

void Disable_Interaction() {
    (ui_get_context())->interaction_disabled = true;
}
void Enable_Interaction() {
    (ui_get_context())->interaction_disabled = false;
}

void Columns(int count, ...) {
    UI_Context* ctx = ui_get_context();
    if (count == 1) {
        ctx->columns.widths[0] = ctx->rect.w;
    }
    else {
        int total_column_width = 0;
        va_list args;
        va_start(args, count);
        for (int i = 0; i < count; ++i) {
            int width = va_arg(args, int);
            total_column_width += width;
            ctx->columns.widths[i] = width;
        }
        va_end(args);
        assert(total_column_width == ctx->rect.w);

        int total_padding = (count - 1) * ctx->style->padding.x;
        int remaining_width = ctx->rect.w - total_padding;
        for (int i = 0; i < count; ++i) {
            float proportion = (float)ctx->columns.widths[i] / total_column_width;
            int width = (int)(proportion * (ctx->rect.w - total_padding));
            ctx->columns.widths[i] = width;
            remaining_width -= width;
        }
        int* columns_topped_off = mem_alloc(sizeof(int) * count);
        int  columns_topped_off_next = 0;
        while (remaining_width > 0) {
            int smallest_index = -1;
            int smallest_value = INT_MAX;
            for (int i = 0; i < count; ++i) {
                bool already_topped_off = false;
                for (int j = 0; j < columns_topped_off_next; ++j) {
                    if (i == columns_topped_off[j]) {
                        already_topped_off = true;
                        break;
                    }
                }
                if (already_topped_off) continue;
                if (ctx->columns.widths[i] < smallest_value) {
                    smallest_value = ctx->columns.widths[i];
                    smallest_index = i;
                }
            }
            assert(smallest_index != -1);
            ctx->columns.widths[smallest_index] += 1;
            --remaining_width;
            columns_topped_off[columns_topped_off_next++] = smallest_index;
        }
    }
    ui_advance_row();
    ctx->columns.count = count;
    ctx->column_cursor = 0;
}

void Equal_Columns(int count) {
    UI_Context* ctx = ui_get_context();
    if (count == 1) {
        ctx->columns.widths[0] = ctx->rect.w;
    }
    else {
        int available_width = ctx->rect.w - (count - 1) * ctx->style->padding.x;
        int equal_width = available_width / count;
        int remainder = available_width % count;
        for (int i = 0; i < count; ++i) {
            ctx->columns.widths[i] = equal_width;
            if (i < remainder) ctx->columns.widths[i]++;
        }
    }
    ui_advance_row();
    ctx->columns.count = count;
    ctx->column_cursor = 0;
}

void Colspan(int count) {
    UI_Context* ctx = ui_get_context();
    assert(count <= ctx->columns.count);
    if (ctx->column_cursor + count > ctx->columns.count) {
        ctx->column_cursor = 0;
        ui_advance_row();
    }
    ctx->colspan = count;
}

void Label(const char* label) {
    UI_Context* ctx = ui_get_context();
    const Rect rect = ui_widget_begin();
    const int height = 10;
    const Rect this_rect = Rect(rect.x, rect.y, rect.w, height);
    draw_rect(this_rect, ctx->style->fg);
    ui_widget_end(height);
}
void Horizontal_Rule(int horizontal_padding, int vertical_padding) {
    UI_Context* ctx = ui_get_context();
    const Rect rect = ui_widget_begin();
    const int width = rect.w - 2 * horizontal_padding;
    const int height = (vertical_padding - 1) * 2 + 1;
    const int top = rect.y + vertical_padding - 1;
    const Rect this_rect = Rect(rect.x + horizontal_padding, top, width, 1);
    draw_rect(this_rect, ctx->style->fg);
    ui_widget_end(height);
}
bool Box(int height) {
    UI_Context* ctx = ui_get_context();
    const Rect rect = ui_widget_begin();
    const Rect this_rect = Rect(rect.x, rect.y, rect.w, height);
    draw_rect(this_rect, ctx->style->fg);
    ui_widget_end(height);
    return false;
}
bool Button(const char* label, int height) {
    UI_Context* ctx = ui_get_context();
    const UIID uiid = get_uiid(label, FROM_STRING);
    const Rect rect = ui_selectable_widget_begin(uiid);
    const Rect this_rect = Rect(rect.x, rect.y, rect.w, height);
    bool ret = false;
    if (uiid_hot == uiid) {
        draw_rect(this_rect, ctx->style->fg);
        if (ui_hot_is_active) {
            ret = true;
            ui_hot_is_active = false;
        }
    }
    else {
        draw_rect(this_rect, ctx->style->bg);
        draw_rect_outline(this_rect, ctx->style->fg);
    }
    ui_selectable_widget_end(height);
    return ret;
}
void Checkbox(bool* boolean, int height) {
    UI_Context* ctx = ui_get_context();
    const UIID uiid = get_uiid(boolean, FROM_POINTER);
    const Rect rect = ui_selectable_widget_begin(uiid);
    const Rect this_rect = Rect(rect.x, rect.y, rect.w, height);
    if (uiid_hot == uiid) {
        draw_rect(this_rect, ctx->style->fg);
        if (ui_hot_is_active) {
            *boolean = !*boolean;
            ui_hot_is_active = false;
        }
    }
    else {
        draw_rect(this_rect, ctx->style->bg);
        draw_rect_outline(this_rect, ctx->style->fg);
    }
    
    if (*boolean) {
        const Point padding = Point(3, 3);
        const int x0 = rect.x + padding.x;
        const int y0 = rect.y + padding.y;
        const int x1 = rect.x + rect.w - 1 - padding.x;
        const int y1 = rect.y + height - 1 - padding.y;
        const Point top_left     = Point(x0, y0);
        const Point bottom_left  = Point(x0, y1);
        const Point top_right    = Point(x1, y0);
        const Point bottom_right = Point(x1, y1);
        draw_line(
            (Point){ top_left.x,     top_left.y     },
            (Point){ bottom_right.x, bottom_right.y },
            (uiid_hot == uiid) ? ctx->style->bg : ctx->style->fg
        );
        draw_line(
            (Point){ bottom_left.x, bottom_left.y },
            (Point){ top_right.x,   top_right.y   },
            (uiid_hot == uiid) ? ctx->style->bg : ctx->style->fg
        );
    }
    ui_selectable_widget_end(height);
}

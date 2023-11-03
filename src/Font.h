#pragma once

typedef enum {
    TS_FIXED_WIDTH,
    TS_PROPORTIONAL
} Text_Style;
typedef enum {
    TA_LEFT,
    TA_CENTER,
    TA_RIGHT
} Text_Alignment;
typedef struct {
    Point glyph_size;
    int glyph_spacing;
    int glyph_shiver;
    int line_spacing;
    LCDColor color;
    Text_Alignment alignment;
    Text_Style style;
    int proportional_space_width;
} Typesetting;
const Typesetting DEFAULT_TYPESETTING = {
    .glyph_size = {0,0},
    .glyph_spacing = 1,
    .glyph_shiver = 0,
    .line_spacing = 1,
    .color = kColorBlack,
    .alignment = TA_LEFT,
    .style = TS_PROPORTIONAL,
    .proportional_space_width = 1
};
typedef enum {
    SP_NEGATIVE_ONE,
    SP_NEGATIVE_HALF,
    SP_ZERO,
    SP_POSITIVE_HALF,
    SP_POSITIVE_ONE
} Stroke_Point_Coord;
typedef struct {
    Stroke_Point_Coord x;
    Stroke_Point_Coord y;
} Stroke_Point;
typedef struct {
    Stroke_Point start;
    Stroke_Point end;
} Stroke;
#define MAX_STROKES_PER_GLYPH 8
typedef struct {
    int stroke_count;
    Stroke strokes[MAX_STROKES_PER_GLYPH];
} Glyph;
#define BASIC_ASCII_COUNT 128
#define FONT_ASCII_OFFSET 32
#define MAX_GLYPHS_PER_FONT BASIC_ASCII_COUNT-FONT_ASCII_OFFSET
typedef Glyph Font[MAX_GLYPHS_PER_FONT];

const Point GLYPH_SIZE_MINIMUM = { 9, 9 };
Point stroke_point(Stroke_Point sp, Point glyph_scale) {
    const Point unit = Point(
        (glyph_scale.x - 1) / 2,
        (glyph_scale.y - 1) / 2
    );
    const Point half_unit = Point(
        unit.x / 2,
        unit.y / 2
    );
    Point p = Point(
        unit.x,
        unit.y
    );
    switch (sp.x) {
    case SP_NEGATIVE_ONE:  p.x += -unit.x;      break;
    case SP_NEGATIVE_HALF: p.x += -half_unit.x; break;
    case SP_ZERO:                               break;
    case SP_POSITIVE_HALF: p.x += half_unit.x;  break;
    case SP_POSITIVE_ONE:  p.x += unit.x;       break;
    }
    switch (sp.y) {
    case SP_NEGATIVE_ONE:  p.y += unit.y;       break;
    case SP_NEGATIVE_HALF: p.y += half_unit.y;  break;
    case SP_ZERO:                               break;
    case SP_POSITIVE_HALF: p.y += -half_unit.y; break;
    case SP_POSITIVE_ONE:  p.y += -unit.y;      break;
    }
    return p;
}

const Font THE_FONT = {
    // Space
    { 0, {{{0}}}},
    // Exclamation mark
    { 2, {
        {{SP_ZERO, SP_ZERO}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_NEGATIVE_HALF}, {SP_ZERO, SP_NEGATIVE_ONE}}
    }},
    // Quotation mark
    { 2, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_HALF}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_POSITIVE_HALF, SP_POSITIVE_ONE}}
    }},
    // Number sign
    { 4, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_HALF}, {SP_POSITIVE_ONE, SP_POSITIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_HALF}, {SP_POSITIVE_ONE, SP_NEGATIVE_HALF}}
    }},
    // Dollar sign
    { 6, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_POSITIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_POSITIVE_HALF}, {SP_POSITIVE_HALF, SP_POSITIVE_HALF}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_ZERO, SP_NEGATIVE_ONE}}
    }},
    // Percent sign
    { 3, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_HALF}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_HALF}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Ampersand
    { 6, {
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Apostrophe
    { 1, {
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_ZERO, SP_POSITIVE_ONE}}
    }},
    // Left parenthesis
    { 3, {
        {{SP_POSITIVE_HALF, SP_POSITIVE_ONE}, {SP_NEGATIVE_HALF, SP_POSITIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_POSITIVE_HALF}, {SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Right parenthesis
    { 3, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_POSITIVE_HALF}},
        {{SP_POSITIVE_HALF, SP_POSITIVE_HALF}, {SP_POSITIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_HALF}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Asterisk
    { 3, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_ZERO}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Plus sign
    { 2, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_ZERO, SP_NEGATIVE_HALF}}
    }},
    // Comma
    { 1, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_ZERO, SP_NEGATIVE_HALF}}
    }},
    // Hyphen-minus
    { 1, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Full stop
    { 1, {
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_ZERO, SP_NEGATIVE_HALF}}
    }},
    // Slash
    { 1, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Digit Zero
    { 5, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Digit One
    { 3, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_HALF}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_ZERO, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Digit Two
    { 4, {
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Digit Three
    { 4, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_ONE, SP_ZERO}}
    }},
    // Digit Four
    { 3, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Digit Five
    { 4, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Digit Six
    { 5, {
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Digit Seven
    { 2, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Digit Eight
    { 7, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}}
    }},
    // Digit Nine
    { 5, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_ZERO}}
    }},
    // Colon
    { 2, {
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_ZERO, SP_NEGATIVE_HALF}},
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_ZERO, SP_POSITIVE_ONE}}
    }},
    // Semicolon
    { 2, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_ZERO, SP_NEGATIVE_HALF}},
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_ZERO, SP_POSITIVE_ONE}}
    }},
    // Less-than sign
    { 2, {
        {{SP_POSITIVE_ONE, SP_POSITIVE_HALF}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_NEGATIVE_HALF}}
    }},
    // Equal sign
    { 2, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_HALF}, {SP_POSITIVE_ONE, SP_POSITIVE_HALF}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_HALF}, {SP_POSITIVE_ONE, SP_NEGATIVE_HALF}}
    }},
    // Greater-than sign
    { 2, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_HALF}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_NEGATIVE_HALF}}
    }},
    // Question mark
    { 5, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_HALF}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_HALF}, {SP_ZERO, SP_POSITIVE_HALF}},
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_ZERO, SP_ZERO}},
        {{SP_ZERO, SP_NEGATIVE_HALF}, {SP_ZERO, SP_NEGATIVE_ONE}}
    }},
    // At sign
    { 8, {
        {{SP_POSITIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_POSITIVE_HALF}},
        {{SP_POSITIVE_HALF, SP_POSITIVE_HALF}, {SP_NEGATIVE_HALF, SP_POSITIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_POSITIVE_HALF}, {SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_ONE, SP_NEGATIVE_HALF}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_HALF}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter A
    { 3, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Capital letter B
    { 6, {
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Capital letter C
    { 3, {
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter D
    { 4, {
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}}
    }},
    // Latin Capital letter E
    { 4, {
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Capital letter F
    { 3, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Capital letter G
    { 5, {
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_ZERO, SP_ZERO}}
    }},
    // Latin Capital letter H
    { 3, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter I
    { 1, {
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_ZERO, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter J
    { 3, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_HALF}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Latin Capital letter K
    { 3, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter L
    { 2, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter M
    { 4, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_ZERO, SP_ZERO}},
        {{SP_ZERO, SP_ZERO}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter N
    { 3, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Latin Capital letter O
    { 4, {
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter P
    { 4, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_ZERO}}
    }},
    // Latin Capital letter Q
    { 5, {
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_ZERO, SP_ZERO}}
    }},
    // Latin Capital letter R
    { 5, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter S
    { 5, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Latin Capital letter T
    { 2, {
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Latin Capital letter U
    { 3, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Latin Capital letter V
    { 2, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_ZERO, SP_NEGATIVE_ONE}},
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Latin Capital letter W
    { 4, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_ZERO, SP_ZERO}},
        {{SP_ZERO, SP_ZERO}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Latin Capital letter X
    { 2, {
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Latin Capital letter Y
    { 3, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_ZERO, SP_ZERO}},
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_ZERO, SP_ZERO}},
        {{SP_ZERO, SP_ZERO}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}}
    }},
    // Latin Capital letter Z
    { 3, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_POSITIVE_ONE}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Left square bracket
    { 3, {
        {{SP_POSITIVE_HALF, SP_POSITIVE_ONE}, {SP_NEGATIVE_HALF, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Backslash
    { 1, {
        {{SP_NEGATIVE_ONE, SP_POSITIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Right square bracket
    { 3, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_POSITIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Circumflex accent (caret)
    { 2, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_HALF}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_POSITIVE_HALF, SP_POSITIVE_HALF}}
    }},
    // Low line (underscore)
    { 1, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}}
    }},
    // Grave accent (backtick)
    { 1, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_ZERO, SP_POSITIVE_HALF}}
    }},
    // Latin Small Letter A
    { 5, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_NEGATIVE_HALF}}
    }},
    // Latin Small Letter B
    { 4, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter C
    { 3, {
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter D
    { 4, {
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_POSITIVE_ONE}}
    }},
    // Latin Small Letter E
    { 5, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter F
    { 3, {
        {{SP_POSITIVE_ONE, SP_POSITIVE_HALF}, {SP_ZERO, SP_POSITIVE_HALF}},
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_ZERO, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter G
    { 5, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_NEGATIVE_HALF}}
    }},
    // Latin Small Letter H
    { 3, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_POSITIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter I
    { 2, {
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_ZERO, SP_ZERO}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_ZERO, SP_POSITIVE_HALF}}
    }},
    // Latin Small Letter J
    { 3, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_ZERO, SP_NEGATIVE_ONE}},
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_ZERO, SP_ZERO}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_ZERO, SP_POSITIVE_HALF}}
    }},
    // Latin Small Letter K
    { 3, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter L
    { 2, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_ZERO, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter M
    { 4, {
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_NEGATIVE_ONE, SP_ZERO}},
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_POSITIVE_ONE, SP_ZERO}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_ZERO, SP_ZERO}, {SP_ZERO, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter N
    { 3, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter O
    { 4, {
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter P
    { 4, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_HALF}, {SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}}
    }},
    // Latin Small Letter Q
    { 4, {
        {{SP_POSITIVE_HALF, SP_NEGATIVE_HALF}, {SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter R
    { 2, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter S
    { 5, {
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_HALF}, {SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_HALF}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter T
    { 3, {
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_ZERO, SP_NEGATIVE_ONE}},
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter U
    { 3, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter V
    { 2, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_ZERO, SP_NEGATIVE_ONE}},
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter W
    { 4, {
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_NEGATIVE_ONE}},
        {{SP_POSITIVE_ONE, SP_NEGATIVE_ONE}, {SP_POSITIVE_ONE, SP_ZERO}},
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_ZERO, SP_ZERO}}
    }},
    // Latin Small Letter X
    { 2, {
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Latin Small Letter Y
    { 2, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_ZERO, SP_NEGATIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_ZERO}}
    }},
    // Latin Small Letter Z
    { 3, {
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}},
        {{SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Left curly bracket
    { 6, {
        {{SP_POSITIVE_HALF, SP_POSITIVE_ONE}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_ZERO, SP_POSITIVE_HALF}},
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_NEGATIVE_HALF, SP_ZERO}},
        {{SP_NEGATIVE_HALF, SP_ZERO}, {SP_ZERO, SP_NEGATIVE_HALF}},
        {{SP_ZERO, SP_NEGATIVE_HALF}, {SP_ZERO, SP_NEGATIVE_ONE}},
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_POSITIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Vertical bar
    { 1, {
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_ZERO, SP_NEGATIVE_HALF}}
    }},
    // Right curly bracket
    { 6, {
        {{SP_NEGATIVE_HALF, SP_POSITIVE_ONE}, {SP_ZERO, SP_POSITIVE_ONE}},
        {{SP_ZERO, SP_POSITIVE_ONE}, {SP_ZERO, SP_POSITIVE_HALF}},
        {{SP_ZERO, SP_POSITIVE_HALF}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_ZERO, SP_NEGATIVE_HALF}},
        {{SP_ZERO, SP_NEGATIVE_HALF}, {SP_ZERO, SP_NEGATIVE_ONE}},
        {{SP_ZERO, SP_NEGATIVE_ONE}, {SP_NEGATIVE_HALF, SP_NEGATIVE_ONE}}
    }},
    // Tilde
    { 3, {
        {{SP_NEGATIVE_ONE, SP_ZERO}, {SP_NEGATIVE_HALF, SP_POSITIVE_HALF}},
        {{SP_NEGATIVE_HALF, SP_POSITIVE_HALF}, {SP_POSITIVE_HALF, SP_ZERO}},
        {{SP_POSITIVE_HALF, SP_ZERO}, {SP_POSITIVE_ONE, SP_POSITIVE_HALF}}
    }},
    // Non-breaking space
    { 0, {{{0}}} }
};
This is a sort of library or framework or whatever to facilitate the creation of Playdate games using the Playdate C API.

***Note that this only works with Visual Studio for now, as that's what I'm using, and I don't know what `#define`s I should look for for MacOS and Linux.***

# Examples

## Basic

This is a basic example that is barely interactive at all, but shows the overall structure of a project using this library/framework/thing.

```c
// define your memory arenas
typedef enum {
    ARENA_GAME,    // stuff that your whole game uses goes in here
    //ARENA_LEVEL, // if the game has levels, we could add this arena and put level stuff in there
    ARENA_FRAME,   // per-frame temporary stuff goes in here
    MEM_ARENA_COUNT
} Mem_Arena;

// define any additional features you wish to use
#define USING_CUSTOM_RENDERER

// define your input actions
typedef enum {
    IA_ACTION, // action button, aka "A"
    INPUT_ACTION_COUNT
} Input_Action;

// include the library
#include "../../librez/librez.h"

// called once when the game starts
static void init() {
    //
    // your init logic goes here
    //
    mem_use(ARENA_FRAME); // set the memory allocator to use the frame arena
}

// input handler
static void input(PDButtons current, PDButtons pushed, PDButtons released) {
    if (current & kButtonA) input_set(IA_ACTION);
}

static int update(float dt, void* userdata) {
    // just so there's any interactivity in this example at all
    clear(kColorWhite);
    if (input_get(IA_ACTION)) draw_text(Point(0, 0), "Hello, world!", NULL);

    //
    // your game logic goes here
    //
    
    mem_reset(); // reset the frame arena at the end of the frame
    return 1;
}
```

## Real-time game with suspend/resume and autosave

This is an example of the suspend/resume system and the autosave system. It's a zero-player game where the score goes up by one each frame -- but what makes it interesting is, if you put the Playdate to sleep or pause or quit the game, the game will autosave, and then after you awaken the Playdate or unpause or relaunch the game, a screen will appear showing a progress bar and estimated time remaining, while the game runs its simulation loop as fast as it can. Once this process is done, the game resumes as normal, but, aside from the loading screen, it's as though the game was running in the background and progressing in real time. Also, it autosaves every thirty seconds, just in case.

```c
typedef enum {
    ARENA_GAME,
    ARENA_FRAME,
    MEM_ARENA_COUNT
} Mem_Arena;

typedef enum {
    IA_PLACEHOLDER, // no inputs in this example
    INPUT_ACTION_COUNT
} Input_Action;

struct {
    int score;
} state;

#define USING_FPS_COUNTER
#define USING_CUSTOM_RENDERER
#define USING_AUTOSAVE
    #define AUTOSAVE_DATA state
    #define AUTOSAVE_PERIOD 30
#define USING_SUSPEND_RESUME

#include "../../librez/librez.h"

static void init(bool autosave_loaded) {
    mem_use(ARENA_FRAME);
}
static void input(PDButtons current, PDButtons pushed, PDButtons released) {}

// run more iterations of the bullshit loop when testing on PC vs on the Playdate
#ifdef TARGET_SIMULATOR
#define BULLSHIT_LOOP_COUNT 70000
#else
#define BULLSHIT_LOOP_COUNT 5000
#endif

// when USING_SUSPEND_RESUME is enabled, you must define a simulate(dt) function like this one. this
// function is called automatically when the game determines that it needs to "catch up" the
// simulation after the game has been resumed after being suspended.
static void simulate(float dt) {
    state.score += 1;
    // this is just to give the CPU something to crunch on so we can see the catch-up process
    if (suspend_resume.frames_left_to_simulate > 0) for (int i = 0; i < BULLSHIT_LOOP_COUNT; ++i) {
        state.score += (i % 2) ? 1 : -1;
    }
}

// clear the background once instead of every frame, for performance during suspend/resume catch-up
static void resume_begin(unsigned int seconds) { clear(kColorBlack); }
static int  resume_update(unsigned int frames_simulated, unsigned int total_frames_to_simulate, float fps, Timespan estimated_time_remaining) {
    if (fps == 0) return 0;

    const int BAR_HEIGHT = 20;
    const int BAR_HPADDING = 1;
    const int bar_width = (int)((float)frames_simulated / (float)total_frames_to_simulate * (float)(LCD_COLUMNS - 1)) - BAR_HPADDING * 2;
    const Rect r = Rect(BAR_HPADDING, LCD_ROWS / 2 - BAR_HEIGHT / 2, bar_width, BAR_HEIGHT);
    draw_rect(r, kColorWhite);

    Typesetting typesetting = DEFAULT_TYPESETTING;
    typesetting.color = kColorWhite;
    draw_text(Point(BAR_HPADDING, LCD_ROWS / 2 - BAR_HEIGHT / 2 - 10), "SIMULATING TIMESKIP...", &typesetting);

    // draw a black box over the bottom text because we're not clearing the whole screen each frame
    Point bottom_text_pos = Point(BAR_HPADDING, LCD_ROWS / 2 + BAR_HEIGHT / 2 + 1);
    draw_rect(Rect(bottom_text_pos.x, bottom_text_pos.y, LCD_COLUMNS - 1 - BAR_HPADDING * 2, 9), kColorBlack);

    // here's an example of the memory arena system at work. we need a buffer to store our formatted
    // string in, so we ask the arena stack for 80 bytes. we don't need to free this memory, because
    // mem_reset() at the end of the frame will reset the arena pointer back to the start of the
    // current arena (which is ARENA_FRAME) for us!
    char* bottom_text = mem_alloc(80);
    unsigned int hours, minutes, seconds, milliseconds;
    timespan_parse(estimated_time_remaining, NULL, &hours, &minutes, &seconds, &milliseconds);
    format_string(&bottom_text, "ETA %02u:%02u:%02u.%03u", hours, minutes, seconds, milliseconds);
    draw_text(bottom_text_pos, bottom_text, &typesetting);

    mem_reset();
    return 1;
}
static void resume_end() {} // we don't need to do anything but get back to the game after resuming

static int update(float dt, void* userdata) {
    simulate(dt); // run the simulation normally

    clear(kColorWhite);

    char* score_string = mem_alloc(80);
    format_string(&score_string, "SCORE: %u", state.score);
    draw_text(Point(1, 20), score_string, NULL);

    mem_reset();
    return 1;
}
```


# API Reference

## Features
The following features are enabled or otherwise set using `#define`.

### `USING_CUSTOM_RENDERER`
Uses a custom rendering backend as much as possible, instead of relying on the Playdate SDK's currently-busted line rendering functionality. Once this is fixed in the SDK, we should be able to get rid of this, or at least deprecate it.

### `USING_UI`
Enables the immediate-mode graphical user interface system. If defined, requires that the `Input_Action` enum defines the following values:
- IA_UI_ACTION
- IA_UI_CANCEL
- IA_UI_UP
- IA_UI_DOWN
- IA_UI_LEFT
- IA_UI_RIGHT

### `USING_FPS_COUNTER`
Includes an FPS counter in the upper-left corner of the display, using the Playdate SDK's font rendering.

### `USING_AUTOSAVE`
Provides an autosave system for games that are meant to be played in short bursts and automatically saved every so often, as well as when the game is suspended (if `USING_SUSPEND_RESUME` is also defined -- see below).

When enabled, the `static void init()` function declaration is replaced with `static void init(bool autosave_loaded)`, where `autosave_loaded` is passed as `true` if the system found and successfully loaded an autosave, or `false` if it did not (i.e. if this is the first time the game is run).

Additionally, this feature declares `autosave()` and `autoload()` functions. If `AUTOSAVE_DATA` is defined, then these are filled out for you -- otherwise, you must implement them on your own. If `USING_SUSPEND_RESUME` is defined, then its data gets autosaved/loaded automatically, too.

#### `AUTOSAVE_FILENAME`
Defines the filename for the autosave file. `autosave` by default if not specified.

#### `AUTOSAVE_DATA`
Defines the variable that is automatically saved to disk when autosaving occurs.

#### `AUTOSAVE_PERIOD`
Defines the period, in seconds, between automatic saves. If left undefined, then automatic saving is not automatic, and must be handled by the user.

#### `AUTOSAVE_RESET`
If defined, causes the autosave system to delete the autosave file instead of autoloading it. Useful for when things break during development.

### `USING_SUSPEND_RESUME`
Provides functionality for automatically "catching up" a simulation when the game is suspended/resumed. Suspension occurs when the game is closed, the game is paused, or the device is put to sleep. Resuming occurs when more than one second has passed since the system last checked the time.

This is useful when you want to create some kind of game that appears to persistently run in real-time "in the background", when the game is actually suspended.

To make use of this system, you must define the following functions:

- `static void simulate(float dt)` The simulation function.
- `static void suspend()` Called when the game is suspended. **Automatically defined if `USING_AUTOSAVE` is also enabled.**
- `static void resume_begin(unsigned int seconds)` Called when the game is resumed -- `seconds` is how many seconds are needed to simulate until the simulation is "caught up".
- `static int  resume_update(unsigned int frames_simulated, unsigned int total_frames_to_simulate, float fps, Timespan eta)` Called during the catch-up process. Use the parameters to display a loading bar or something. `fps` is the current average catch-up "framerate". Return value is just like the normal `update()` function -- whether or not to update the display.
- `static void resume_end()` Called once the simulation is "caught up".

#### `SUSPEND_RESUME_SIMULATION_REFRESH_RATE`
This is the "framerate" that `simulate()` will be run at when "catching up" the simulation to the current time when the game is resumed. Must be a `float`. Defaults to `30.f`.

#### `SUSPEND_RESUME_DISPLAY_INTERVAL_MILLISECONDS`
The interval at which the display is updated while simulating the suspend/resume, in milliseconds. Must be less than 10000, or else the hardware will think your game has hung and will terminate it. Must be an `unsigned int`. Defaults to `((unsigned int)(1.f / SUSPEND_RESUME_SIMULATION_REFRESH_RATE * 1000.f))`.

#### `SUSPEND_RESUME_SIMULATE_SECONDS`
Used for testing out long suspend/resume simulation catch-ups. Causes the game to pretend that the given number of seconds has passed since the last time the game was run, when the game launches.

#### `SUSPEND_RESUME_ETA_BUFFER_SIZE`
The size of the buffer used for calculating the ETA of the suspend/resume catch-up process. Each element of the buffer is 8 bytes. Defaults to `60`.

### `USING_PD_FORMAT_STRING`
Have `string_format()` wrap `pd->system->formatString()`, instead of `stbsp_sprintf()` (which it does by default).


## Memory
A memory allocation system is provided, using a stack of arena allocators in a shared chunk of memory. Typically, you want at least two memory arenas: one for stuff that is allocated once and will remain allocated for the duration of your game's execution, and another for per-frame temporary allocations. Depending on the complexity of your game, you may wish to include one or more additional arenas between these two: for example, you could have an arena for level-specific memory, that is reset and reused when transitioning from one level to the next.

### `MEM_ARENA_CHUNK_SIZE`
The amount of memory, in bytes, that will be set aside for use in the arena stack. Defaults to `1024 * 1024 * 2` (2MB).

### `void* mem_alloc(size_t bytes)`
Allocates `bytes` bytes from the current arena.

### `void mem_reset()`
Resets all arenas in the arena stack after (and including) the current arena.

### `void mem_use(Mem_Arena arena)`
Set the current active arena to `arena`. Automatically calls `mem_reset()`.



## Util
Various utility functions that provide things such as better handling of colors/patterns, easing, and hashing.

### `void set_black(LCDPattern* pattern)`
Sets the color portion of `pattern` to be entirely black.

### `void set_white(LCDPattern* pattern)`
Sets the color portion of `pattern` to be entirely white.

### `void set_alpha(LCDPattern* pattern, float alpha)`
Sets the "alpha" of `pattern` using a Bayer matrix and the provided `alpha` value.

### `float ease(float x, float target, float speed)`
Easy ease function.

### `float accelerated_change(float x)`
Possibly useful for crank motion.

### `Hash hash_string(const char* str)`
Hash the given string.

### `Hash hash_int(int n)`
Hash the given int.

### `Hash hash_combine(Hash a, Hash b)`
Combine the given `Hash`es.

### `Hash hash_pointer(const void* ptr)`
Hash the given pointer.

### `typedef struct { ... } Timestamp`
Represents a point in time.

### `typedef struct { ... } Timespan`
Represents a difference between points in time.

### `void timestamp_now(Timestamp* timestamp)`
Sets the given `Timestamp` to represent the current time.

### `unsigned int timestamp_diff(Timestamp* earlier, Timestamp* later, Timespan* diff)`
Returns the number of milliseconds between the two `Timestamp`s. If `diff` is not `NULL`, it will be set to the `Timespan` representing the difference.

### `void timespan_parse(Timespan timespan, unsigned int* days, unsigned int* hours, unsigned int* minutes, unsigned int* seconds, unsigned int* milliseconds)`
Sets `days`, `hours`, `minutes`, `seconds`, and `milliseconds` based on `timespan`. `NULL` parameters are ignored.

### `format_string(char **ret, const char *format, ...)`
Wrapper for `stbsp_sprintf()`, unless `USING_PD_FORMAT_STRING` is defined, then it's a wrapper for `pd->system->formatString()`. Suppresses `double` promotion warnings when building for the device.



## Geometry
`Point`s and `Rect`s are structs that are used throughout this library -- this is where they are defined.

### `typedef struct { ... } Point`
Describes an integer-coordinate point in two-dimensional space (i.e. on the screen).

### `#define Point(x, y)  ...`
Shorthand method of defining a `Point`.

### `typedef union { ... } Rect`
Describes an integer-coordinate rectangle in two-dimensional space. Defined as a `union` of two points (`top_left` and `dimensions`) and four `int`s (`x`, `y`, `w`, `h`).

### `Point get_top_right(Rect rect)`
Gets the top-right point of the `Rect`.

### `Point get_bottom_right(Rect rect)`
Gets the bottom-right point of the `Rect`.

### `Point get_bottom_left(Rect rect)`
Gets the bottom-left point of the `Rect`.

### `Point get_center(Rect rect)`
Gets the center point of the `Rect`.



## Font
(TODO)



## Renderer
The current Playdate SDK has inaccurate one-pixel line rendering, so an alternative renderer is provided (see `USE_CUSTOM_RENDERER` above).

### `LCDBitmap* new_bitmap(int width, int height, LCDColor bgcolor)`
Wrapper for `pd->graphics->newBitmap()`.

### `void free_bitmap(LCDBitmap* bitmap)`
Wrapper for `pd->graphics->freeBitmap()`.

### `void clear(LCDColor color)`
Wrapper for `pd->graphics->clear()`.

### `void clip_set(Rect r)`
Wrapper for `pd->graphics->setClipRect()`. Necessary for custom renderer.

### `void clip_clear()`
Wrapper for `pd->graphics->clearClipRect()`. Necessary for custom renderer.

### `void target_push(LCDBitmap* target)`
Wrapper for `pd->graphics->pushContext()`. Necessary for custom renderer.

### `void target_push_debug()`
Sort of a wrapper for `pd->graphics->pushContext(pd->graphics->getDebugBitmap())`, but it automatically dummies out when building for the device, to avoid the null pointer dereference caused by invoking `pd->graphics->getDebugBitmap()` (which is `NULL` when compiled for the device).

### `void target_pop()`
Wrapper for `pd->graphics->popContext()`. Necessary for custom renderer.

### `void draw_point(Point p, LCDColor color)`
Draw a single pixel.

### `void draw_line(Point a, Point b, LCDColor color)`
Draw a one-pixel line. Should be a wrapper for `pd->graphics->drawLine()`, except that's busted -- hence, the impetus for the custom renderer.

### `void draw_rect(Rect r, int roundness, LCDColor color)`
Wrapper for `pd->graphics->fillRect()`, with support for `roundness`.

### `void draw_rect_outline(Rect r, int roundness, LCDColor color)`
Wrapper for `pd->graphics->drawRect()`, with support for `roundness`.

### `void draw_triangle(Point a, Point b, Point c, LCDColor color)`
Wrapper for `pd->graphics->fillTriangle()`. ***Currently unavailable when custom renderer is enabled.***

### `void draw_triangle_outline(Point a, Point b, Point c, LCDColor color)`
Draw a one-pixel-outlined triangle.

### `void draw_ellipse(Rect r, float angle_start, float angle_end, LCDColor color)`
Wrapper for `pd->grpahics->fillEllipse()`.

### `void draw_ellipse_outline(Rect r, float angle_start, float angle_end, LCDColor color)`
Wrapper for `pd->graphics->drawEllipse()`.

### `void draw_circle(Point center, int diameter, LCDColor color)`
Draw a circle. Use an odd `diameter` to make sure it's centered on `center`.

### `void draw_circle_outline(Point center, int diameter, LCDColor color)`
Draw a one-pixel-outlined circle. Use an odd `diameter` to make sure it's centered on `center`.

### `void draw_text(Point top_left, const char* text, const Typesetting* typesetting)`
Draw a string of text using vector text rendering.



## Input
Instead of sampling inputs every time you want to check them, why not have a system of `Input_Action`s that get checked once per frame?

### `bool input_get(Input_Action action)`
Gets whether the given `Input_Action` was activated this frame.

### `void input_set(Input_Action action)`
Sets the given `Input_Action` to be activated this frame. For use in `input()`.



## System
(TODO)



## UI
If `USING_UI` is defined, an immediate-mode UI system is provided.

### `void UI_reset()`
To be called at the end of the frame. Handles UI navigation through input, and resets per-frame UI data.

### `void UI_Begin(const char* label, Rect rect, UI_Style* style)`
Begins a new UI context.

### `void UI_End()`
Ends the current UI context.

### `void Disable_Interaction()`
Temporarily disable UI interaction until reenabled. Does not persist across frames.

### `void Enable_Interaction()`
Reenable temporarily-disabled UI interaction.

### `void Columns(int count, ...)`
Sets the current UI context to use columns for layout. The column `count` must be specified, followed by that number of `int` column widths.

### `void Equal_Columns(int count)`
Sets the current UI context to use columns for layout. The column widths will be equal, or as equal as possible given the size of the current UI context.

### `void Colspan(int count)`
Causes the next UI element to span `count` columns instead of one.

### `void Label(const char* label)`
Produces a text label. ***Currently nonfunctional.***

### `void Horizontal_Rule(int horizontal_spacing, int vertical_padding)`
Produces a horizontal rule (line), with the provided spacing parameters.

### `bool Button(const char* label, int height)`
Produces a button with the specified `height` and `label` text. Returns `true` if the button was active and activated.

### `void Checkbox(bool* boolean, int height)`
Produces a checkbox with the specified `height`. The `bool` pointed to by `boolean` determines whether the box is checked or not, and will be toggled if the button was active and activated.

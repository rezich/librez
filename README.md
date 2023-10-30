This is a sort of library or framework or whatever to facilitate the creation of Playdate games using the Playdate C API.

To use it, structure your main.c as follows:

    // define your memory arenas
    typedef enum {
        MA_GAME,
        //MA_LEVEL,
        MA_FRAME,
        MEM_ARENA_COUNT
    } Mem_Arena;
    
    // define any additional features you wish to use
    //#define USING_FPS_COUNTER
    #define USING_CUSTOM_RENDERER
    
    // define your input actions (NOTE: for now, the first six are required)
    typedef enum {
        IA_UI_ACTION,
        IA_UI_CANCEL,
        IA_UI_UP,
        IA_UI_DOWN,
        IA_UI_LEFT,
        IA_UI_RIGHT,
        IA_META_TOGGLE,
        IA_HOLD_B,
        INPUT_ACTION_COUNT
    } Input_Action;
    
    // include the library
    #include "../../librez/librez.h"
    
    // called once when the game starts
    static void init() {
        mem_use(MA_FRAME); // set the memory allocator to use the frame arena
    }
    
    // input handler
    static void input(PDButtons current, PDButtons pushed, PDButtons released) {
        if (pushed & kButtonA)     input_set(IA_UI_ACTION);
        if (pushed & kButtonB)     input_set(IA_UI_CANCEL);
        if (pushed & kButtonRight) input_set(IA_UI_RIGHT);
        if (pushed & kButtonLeft)  input_set(IA_UI_LEFT);
        if (pushed & kButtonUp)    input_set(IA_UI_UP);
        if (pushed & kButtonDown)  input_set(IA_UI_DOWN);
    }
    
    static int update(void* userdata) {
        float dt = pd->system->getElapsedTime();
        pd->system->resetElapsedTime();
        
        // your game logic goes here

        UI_Reset();  // if you're using the UI system
        mem_reset(); // reset the frame arena
        return 1;
    }

There's a bunch of additional features that will be documented at some point.
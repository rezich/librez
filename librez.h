#pragma once

#include "pd_api.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <limits.h>
#include <float.h>
#include <math.h>


PlaydateAPI* pd = NULL;
#include "Memory.h"
#include "Util.h"
#include "Geometry.h"
#include "Font.h"
#include "Renderer.h"
#include "Input.h"
#ifdef USING_UI
#include "UI.h"
#endif

#ifdef USING_AUTOSAVE
static void init(bool autosave_loaded);
#else
static void init();
#endif
static int  update(float dt, void* userdata);

#ifdef USING_SUSPEND_RESUME
    static void simulate(float dt);
    #ifndef SIMULATION_REFRESH_RATE
        #define SIMULATION_REFRESH_RATE 30.f
    #endif
    static bool is_resuming = false;
    static unsigned int last_second_simulated = 0;
    static void suspend();
    static void resume_begin(unsigned int seconds);
    static void resume_end();
#endif
#ifdef USING_AUTOSAVE
    static unsigned int last_second_autosaved = 0;
#ifndef AUTOSAVE_FILENAME
#define AUTOSAVE_FILENAME "autosave"
#endif
    static void autosave();
    static bool autoload();
#endif
#if defined(USING_SUSPEND_RESUME) && defined(USING_AUTOSAVE) && defined(AUTOSAVE_STRUCT)
    static void autosave() {
        SDFile* file = pd->file->open(AUTOSAVE_FILENAME, kFileWrite);
        if (!file) {
            pd->system->error("Could not open autosave file to write!");
            assert(false);
        }
        if (pd->file->write(file, &last_second_simulated, sizeof(last_second_simulated)) == -1) {
            pd->system->error("Failed to write last_second_simulated to autosave file!");
            assert(false);
        }
        if (pd->file->write(file, &AUTOSAVE_STRUCT, sizeof(AUTOSAVE_STRUCT)) == -1) {
            pd->system->error("Failed to write save data to autosave file!");
            assert(false);
        }
        else {
            last_second_autosaved = pd->system->getSecondsSinceEpoch(NULL);
        }
        if (pd->file->close(file) != 0) {
            pd->system->error("Failed to close autosave file!");
            assert(false);
        }
    }
    static bool autoload() {
#ifdef AUTOSAVE_RESET
        if (pd->file->unlink(AUTOSAVE_FILENAME, false) == -1) pd->system->logToConsole("Could not delete save file (probably because it doesn't exist.");
#ifdef USING_SUSPEND_RESUME
#ifdef SUSPEND_RESUME_SIMULATE_RUNNING_FOR_A_LONG_TIME
        last_second_simulated = pd->system->getSecondsSinceEpoch(NULL) - 1000000;
#else
        last_second_simulated = pd->system->getSecondsSinceEpoch(NULL);
#endif
#endif
        return false;
#endif
        SDFile* file = pd->file->open(AUTOSAVE_FILENAME, kFileReadData);
        if (!file) {
            pd->system->logToConsole("Could not open autosave file to read! (probably no file exists)");
#ifdef USING_SUSPEND_RESUME
            last_second_simulated = pd->system->getSecondsSinceEpoch(NULL);
#endif
            return false;
        }
        if (pd->file->read(file, &last_second_simulated, sizeof(last_second_simulated)) == -1) {
            pd->system->error("Failed to read last_second_simulated from autosave file!");
            assert(false);
        }
        if (pd->file->read(file, &state, sizeof(State)) == -1) {
            pd->system->error("Failed to read save data from autosave file!");
            assert(false);
        }
        if (pd->file->close(file) != 0) {
            pd->system->error("Failed to close autosave file!");
            assert(false);
        }
        return true;
    }
    static void suspend() { autosave(); }
#endif

#ifdef USING_FPS_COUNTER
const char* fpsfontpath = "/System/Fonts/Asheville-Sans-14-Bold.pft";
LCDFont* fpsfont = NULL;
#endif

static int _update(void* userdata) {
#ifdef USING_CUSTOM_RENDERER
    _begin_rendering();
#endif
#ifdef USING_SUSPEND_RESUME
    {
        unsigned int current_second = pd->system->getSecondsSinceEpoch(NULL);
        unsigned int seconds_to_simulate = current_second - last_second_simulated;
        if (seconds_to_simulate > 1) {
            if (is_resuming) {
                unsigned int frames_to_simulate = (unsigned int)((float)seconds_to_simulate * SIMULATION_REFRESH_RATE);
                pd->system->logToConsole("Simulating %d frames to catch up %d seconds!", frames_to_simulate, seconds_to_simulate);
                float dt = 1.f / SIMULATION_REFRESH_RATE;
                for (unsigned int i = 0; i < frames_to_simulate; ++i) simulate(dt);
                resume_end();
                is_resuming = false;
            }
            else {
                is_resuming = true;
                resume_begin(seconds_to_simulate);
                return 1;
            }
        }
        last_second_simulated = current_second;
    }
#endif
    _input_update();
    float dt = pd->system->getElapsedTime();
    pd->system->resetElapsedTime();
    int update_display = update(dt, userdata);
#if defined(USING_AUTOSAVE) && defined(AUTOSAVE_PERIOD)
    {
        unsigned int current_second = pd->system->getSecondsSinceEpoch(NULL);
        if (current_second - last_second_autosaved >= AUTOSAVE_PERIOD) {
            pd->system->logToConsole("Autosaving...");
            autosave();
            last_second_autosaved = current_second;
        }
    }
#endif
#ifdef USING_FPS_COUNTER
    pd->graphics->setFont(fpsfont);
    pd->system->drawFPS(0, 0);
#endif
    return update_display;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
    (void)arg; // arg is currently only used for event = kEventKeyPressed
    switch (event) {
    case kEventInit:
        pd = playdate;
#ifdef USING_FPS_COUNTER
        const char* err;
        fpsfont = pd->graphics->loadFont(fpsfontpath, &err);
        if (!fpsfont) pd->system->error("%s:%i Couldn't load font %s: %s", __FILE__, __LINE__, fpsfontpath, err);
#endif
        pd->system->setUpdateCallback(_update, NULL);
        srand(pd->system->getSecondsSinceEpoch(NULL));
        _mem_init();
#ifdef USING_AUTOSAVE
        init(autoload());
#else
        init();
#endif
        break;
    case kEventInitLua:
        break;
    case kEventLock:
#ifdef USING_SUSPEND_RESUME
        suspend();
#endif
        break;
    case kEventUnlock:
        break;
    case kEventPause:
#ifdef USING_SUSPEND_RESUME
        suspend();
#endif
        break;
    case kEventResume:
        break;
    case kEventTerminate:
#ifdef USING_SUSPEND_RESUME
        suspend();
#endif
        break;
    case kEventKeyPressed:
        break;
    case kEventKeyReleased:
        break;
    case kEventLowPower:
        break;
    }
    return 0;
}
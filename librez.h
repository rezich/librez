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

#ifdef _WINDLL
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE __attribute__((always_inline)) inline
#endif

PlaydateAPI* pd = NULL;
#include "src/System.h"
#include "src/Memory.h"
#include "src/Util.h"
#include "src/Geometry.h"
#include "src/Font.h"
#include "src/Renderer.h"
#include "src/Input.h"
#ifdef USING_UI
#include "src/UI.h"
#endif

#ifdef USING_AUTOSAVE
static void init(bool autosave_loaded);
#else
static void init();
#endif
static int  update(float dt, void* userdata);

#ifdef USING_SUSPEND_RESUME
    static void simulate(float dt);
    #ifndef SUSPEND_RESUME_SIMULATION_REFRESH_RATE
        #define SUSPEND_RESUME_SIMULATION_REFRESH_RATE 30.f
    #endif
    #ifndef SUSPEND_RESUME_DISPLAY_INTERVAL_MILLISECONDS
        #define SUSPEND_RESUME_DISPLAY_INTERVAL_MILLISECONDS ((unsigned int)(1.f / SUSPEND_RESUME_SIMULATION_REFRESH_RATE * 1000.f))
    #endif
    static struct {
        unsigned int last_second_simulated_or_queued;
        unsigned int frames_left_to_simulate;
        unsigned int total_frames_to_simulate;
    } suspend_resume;
    static bool _is_resuming = false;
    #ifndef SUSPEND_RESUME_ETA_BUFFER_SIZE
        #define SUSPEND_RESUME_ETA_BUFFER_SIZE 60
    #endif
    static struct {
        unsigned int ms;
        unsigned int frames;
    } _suspend_resume_eta_buffer[SUSPEND_RESUME_ETA_BUFFER_SIZE];
    static size_t _suspend_resume_eta_buffer_count   = 0;
    static size_t _suspend_resume_eta_buffer_pointer = 0;
    static void suspend();
    static void resume_begin(unsigned int seconds);
    static int  resume_update(unsigned int frames_simulated, unsigned int total_frames_to_simulate, float fps, Timespan estimated_time_remaining);
    static void resume_end();
    #ifndef USING_AUTOSAVE
        #error "USING_AUTOSAVE is necessary for USING_SUSPEND_RESUME"
    #endif
#endif
#ifdef USING_AUTOSAVE
    static unsigned int last_second_autosaved = 0;
    static bool _autosave_disabled = false;
    #ifndef AUTOSAVE_FILENAME
        #define AUTOSAVE_FILENAME "autosave"
    #endif
    #if !defined(AUTOSAVE_DATA)
        static void autosave();
        static bool autoload();
    #endif
#endif
#if defined(USING_AUTOSAVE) && defined(AUTOSAVE_DATA)
    static void autosave() {
        if (_autosave_disabled) return;
        SDFile* file = pd->file->open(AUTOSAVE_FILENAME, kFileWrite);
        if (!file) {
            pd->system->error("Could not open autosave file to write!");
            assert(false);
        }
#ifdef USING_SUSPEND_RESUME
        if (pd->file->write(file, &suspend_resume, sizeof(suspend_resume)) == -1) {
            pd->system->error("Failed to write suspend_resume to autosave file!");
            assert(false);
        }
#endif
        if (pd->file->write(file, &AUTOSAVE_DATA, sizeof(AUTOSAVE_DATA)) == -1) {
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
        if (pd->file->unlink(AUTOSAVE_FILENAME, false) == -1) LOG("Could not delete save file (probably because it doesn't exist.");
#ifdef USING_SUSPEND_RESUME
#ifdef SUSPEND_RESUME_SIMULATE_SECONDS
        suspend_resume.last_second_simulated_or_queued = pd->system->getSecondsSinceEpoch(NULL) - SUSPEND_RESUME_SIMULATE_SECONDS;
#else
        suspend_resume.last_second_simulated_or_queued = pd->system->getSecondsSinceEpoch(NULL);
#endif
#endif
        return false;
#endif
        SDFile* file = pd->file->open(AUTOSAVE_FILENAME, kFileReadData);
        if (!file) {
            LOG("Could not open autosave file to read! (probably no file exists)");
#ifdef USING_SUSPEND_RESUME
            suspend_resume.last_second_simulated_or_queued = pd->system->getSecondsSinceEpoch(NULL);
#endif
            return false;
        }
#ifdef USING_SUSPEND_RESUME
        if (pd->file->read(file, &suspend_resume, sizeof(suspend_resume)) == -1) {
            pd->system->error("Failed to read last_second_simulated_or_queued from autosave file!");
            assert(false);
        }
#endif
        if (pd->file->read(file, &state, sizeof(AUTOSAVE_DATA)) == -1) {
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
    bool do_update = true;
    int update_display = 0;
#ifdef USING_CUSTOM_RENDERER
    _begin_rendering();
#endif
#ifdef USING_SUSPEND_RESUME
    {
        const unsigned int current_second = pd->system->getSecondsSinceEpoch(NULL);
        const unsigned int seconds_to_simulate = current_second - suspend_resume.last_second_simulated_or_queued;
        if (seconds_to_simulate > 1) {
            const unsigned int more_frames = (unsigned int)((float)seconds_to_simulate * SUSPEND_RESUME_SIMULATION_REFRESH_RATE);
            LOG("Adding more frames: %u", more_frames);
            suspend_resume.total_frames_to_simulate += more_frames;
            suspend_resume.frames_left_to_simulate  += more_frames;
            if (!_is_resuming) resume_begin(seconds_to_simulate);
        }
        suspend_resume.last_second_simulated_or_queued = current_second;

        if (suspend_resume.frames_left_to_simulate > 0) {
            Timestamp start;
            timestamp_now(&start);
            unsigned int frames_simulated_this_iteration = 0;
            const float dt = 1.f / SUSPEND_RESUME_SIMULATION_REFRESH_RATE;
            while (suspend_resume.frames_left_to_simulate > 0) {
                simulate(dt);
                --suspend_resume.frames_left_to_simulate;
                ++frames_simulated_this_iteration;
                Timestamp now;
                timestamp_now(&now);
                Timespan time_simulated;
                int milliseconds_simulated = timestamp_diff(&start, &now, &time_simulated);
                if (milliseconds_simulated >= SUSPEND_RESUME_DISPLAY_INTERVAL_MILLISECONDS) {
                    _suspend_resume_eta_buffer_count = min(SUSPEND_RESUME_ETA_BUFFER_SIZE, _suspend_resume_eta_buffer_count + 1);
                    _suspend_resume_eta_buffer[_suspend_resume_eta_buffer_pointer].frames = frames_simulated_this_iteration;
                    _suspend_resume_eta_buffer[_suspend_resume_eta_buffer_pointer].ms = milliseconds_simulated;
                    _suspend_resume_eta_buffer_pointer = (_suspend_resume_eta_buffer_pointer + 1) % SUSPEND_RESUME_ETA_BUFFER_SIZE;
                    break;
                }
            }
            if (suspend_resume.frames_left_to_simulate == 0) {
                suspend_resume.total_frames_to_simulate = 0;
                _is_resuming = false;
                resume_end();
            }
            else {
                unsigned int total_milliseconds = 0;
                unsigned int total_frames = 0;
                for (int i = 0; i < _suspend_resume_eta_buffer_count; ++i) {
                    total_milliseconds += _suspend_resume_eta_buffer[i].ms;
                    total_frames += _suspend_resume_eta_buffer[i].frames;
                }
                total_milliseconds /= (unsigned int)_suspend_resume_eta_buffer_count;
                total_frames       /= (unsigned int)_suspend_resume_eta_buffer_count;
                const float fpms = (float)total_milliseconds / (float)total_frames;
#ifdef _WINDLL
#pragma warning( push )
#pragma warning( disable : 26451 )
#endif
                const unsigned long long eta_ms = (unsigned long long)((float)suspend_resume.frames_left_to_simulate * fpms);
#ifdef _WINDLL
#pragma warning( pop )
#endif
                Timespan eta = Timespan((unsigned int)(eta_ms / 1000), (unsigned int)(eta_ms % 1000));
                update_display = resume_update(suspend_resume.total_frames_to_simulate - suspend_resume.frames_left_to_simulate, suspend_resume.total_frames_to_simulate, fpms * 1000.f, eta);
                do_update = false;
            }
        }
    }
#endif
    if (do_update) {
        _input_update();
        float dt = pd->system->getElapsedTime();
        pd->system->resetElapsedTime();
        update_display = update(dt, userdata);
#if defined(USING_AUTOSAVE) && defined(AUTOSAVE_PERIOD)
        unsigned int current_second = pd->system->getSecondsSinceEpoch(NULL);
        if (current_second - last_second_autosaved >= AUTOSAVE_PERIOD) {
            LOG("Autosaving...");
            autosave();
            last_second_autosaved = current_second;
        }
#endif
    }
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
#ifdef USING_SUSPEND_RESUME
        suspend_resume.last_second_simulated_or_queued = 0;
        suspend_resume.frames_left_to_simulate         = 0;
        suspend_resume.total_frames_to_simulate        = 0;
#endif
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

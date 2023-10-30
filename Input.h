#pragma once

bool input_actions[INPUT_ACTION_COUNT];
const bool DEFAULT_INPUT_ACTIONS[INPUT_ACTION_COUNT] = { 0 };
inline bool input_get(Input_Action action) { return input_actions[action]; }
inline void input_set(Input_Action action) { input_actions[action] = true; }
static void input(PDButtons current, PDButtons pushed, PDButtons released);
void _input_update() {
    memcpy(&input_actions, DEFAULT_INPUT_ACTIONS, sizeof(DEFAULT_INPUT_ACTIONS));
    PDButtons current, pushed, released;
    pd->system->getButtonState(&current, &pushed, &released);
    input(current, pushed, released);
}

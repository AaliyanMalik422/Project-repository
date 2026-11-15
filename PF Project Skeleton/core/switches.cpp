#include "switches.h"
#include "simulation_state.h"

int total_switches = 0;

void toggleSwitch(int switchIndex) {
    if (switchIndex < 0 || switchIndex >= MAX_SWITCHES) return;

    // Switch state is represented consistently as 0 = straight, 1 = turn.
    switch_state[switchIndex] = switch_state[switchIndex] == 0 ? 1 : 0;
}

void initializeSwitches() {
    total_switches = 0;

    for (int i = 0; i < MAX_SWITCHES; ++i) {
        if (switch_active[i]) {
            ++total_switches;
        } else {
            switch_flip_queued[i] = false;
        }
    }
}

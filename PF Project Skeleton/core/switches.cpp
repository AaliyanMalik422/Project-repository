#include "switches.h"
#include "simulation_state.h"
#include "grid.h"
#include "io.h"
#include <iostream>

using namespace std;

// ============================================================================
// SWITCHES.CPP - Switch management
// ============================================================================

// ----------------------------------------------------------------------------
// UPDATE SWITCH COUNTERS
// ----------------------------------------------------------------------------
// Logic: Loop through all trains. If a train is standing ON a switch,
// decrement the counter for that train's direction.
// ----------------------------------------------------------------------------
void updateSwitchCounters() {
    for (int t = 0; t < total_trains; t++) {
        // Skip inactive trains
        if (!train_active[t]) continue;

        // Check against all switches
        for (int s = 0; s < MAX_SWITCHES; s++) {
            if (!switch_active[s]) continue;

            // Is the train exactly on the switch coordinates?
            if (train_x[t] == switch_x[s] && train_y[t] == switch_y[s]) {
                
                // Get the direction the train is facing (0=UP, 1=RIGHT, etc.)
                int dir = train_direction[t];

                // Decrement the counter for this specific switch and direction
                // (Assuming we count down from K to 0)
                if (switch_counters[s][dir] > 0) {
                    switch_counters[s][dir]--;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// QUEUE SWITCH FLIPS
// ----------------------------------------------------------------------------
// Logic: Check all switches. If any counter hits 0, queue a flip
// and reset the counter back to K.
// ----------------------------------------------------------------------------
void queueSwitchFlips() {
    for (int s = 0; s < MAX_SWITCHES; s++) {
        if (!switch_active[s]) continue;

        // Check all 4 directions for this switch
        for (int dir = 0; dir < 4; dir++) {
            if (switch_counters[s][dir] <= 0) {
                // Trigger condition met!
                switch_flip_queued[s] = true;

                // Reset the counter back to the original K value
                switch_counters[s][dir] = switch_k_values[s][dir];
            }
        }
    }
}

// ----------------------------------------------------------------------------
// APPLY DEFERRED FLIPS
// ----------------------------------------------------------------------------
// Logic: If a switch is queued to flip, toggle its state (0->1 or 1->0)
// and clear the queue flag.
// ----------------------------------------------------------------------------
void applyDeferredFlips() {
    for (int s = 0; s < MAX_SWITCHES; s++) {
        if (switch_active[s] && switch_flip_queued[s]) {
            // Toggle state: If 0 becomes 1, if 1 becomes 0
            switch_state[s] = !switch_state[s];
            
            // Clear the flag
            switch_flip_queued[s] = false;
            
            // Optional: Debug output
            // cout << "Switch " << (char)('A' + s) << " flipped to " << switch_state[s] << endl;
        }
    }
}

// ----------------------------------------------------------------------------
// UPDATE SIGNAL LIGHTS
// ----------------------------------------------------------------------------
// Placeholder: If you need to change grid colors based on switch state
// ----------------------------------------------------------------------------
void updateSignalLights() {
    // This function can be left empty for now unless you have specific 
    // requirements to change the map colors based on Traffic Lights.
}

// ----------------------------------------------------------------------------
// TOGGLE SWITCH STATE (Manual)
// ----------------------------------------------------------------------------
// Manually toggle a switch state.
// ----------------------------------------------------------------------------
void toggleSwitchState(int switchIndex) {
    if (switchIndex >= 0 && switchIndex < MAX_SWITCHES && switch_active[switchIndex]) {
        switch_state[switchIndex] = !switch_state[switchIndex];
    }
}

// ----------------------------------------------------------------------------
// GET SWITCH STATE FOR DIRECTION
// ----------------------------------------------------------------------------
// Returns the state (0 or 1). Currently returns the global state,
// but can be modified if your switches have different states per direction.
// ----------------------------------------------------------------------------
int getSwitchStateForDirection(int switchIndex, int direction) {
    if (switchIndex >= 0 && switchIndex < MAX_SWITCHES) {
        return switch_state[switchIndex];
    }
    return 0; // Default error
}
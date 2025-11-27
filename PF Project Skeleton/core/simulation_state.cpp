#include "simulation_state.h"

// Implementation of the initialization helper
void initializeSimulation(int t_active[], int t_count, int s_count, int& rows, int& cols) {
    // 1. Reset Dimensions
    rows = 0;
    cols = 0;
    
    // 2. Reset Counts
    // These track how many items are actually in the arrays
    // t_count = 0; // Usually set by loadLevel, but good practice to reset
    // s_count = 0;

    // 3. Clear Active Status of Trains
    // This ensures no "ghost trains" exist from a previous run
    for (int i = 0; i < MAX_TRAINS; i++) {
        t_active[i] = 0; // 0 = Inactive (Dead)
    }
}
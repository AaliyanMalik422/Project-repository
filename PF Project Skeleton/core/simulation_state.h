#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

// ============================================================================
// SIMULATION_STATE.H - Global constants and state
// ============================================================================

// ----------------------------------------------------------------------------
// CONSTANTS (Fixed limits based on PDF)
// ----------------------------------------------------------------------------
const int MAX_ROWS = 20;
const int MAX_COLS = 60; // Increased for "Hard Level" (PDF says 60 cols)
const int MAX_TRAINS = 50;
const int MAX_SWITCHES = 26; // A-Z

// Direction Enums (0=UP, 1=RIGHT, 2=DOWN, 3=LEFT)
const int DIR_UP = 0;
const int DIR_RIGHT = 1;
const int DIR_DOWN = 2;
const int DIR_LEFT = 3;

// ----------------------------------------------------------------------------
// GLOBAL STATE: GRID
// ----------------------------------------------------------------------------
// The actual 2D map of characters
extern char grid[MAX_ROWS][MAX_COLS];
extern int grid_rows;
extern int grid_cols;

// ----------------------------------------------------------------------------
// GLOBAL STATE: TRAINS (Parallel Arrays)
// ----------------------------------------------------------------------------
extern int train_id[MAX_TRAINS];
extern int train_x[MAX_TRAINS];
extern int train_y[MAX_TRAINS];
extern int train_direction[MAX_TRAINS];
extern int train_color[MAX_TRAINS];
extern int train_spawn_tick[MAX_TRAINS];
extern bool train_active[MAX_TRAINS];    // Is it currently on the map?
extern bool train_finished[MAX_TRAINS];  // Did it reach destination?

// For Collision Logic (Member B needs these)
extern int train_next_x[MAX_TRAINS];
extern int train_next_y[MAX_TRAINS];
extern int train_dest_x[MAX_TRAINS];     // Destination D
extern int train_dest_y[MAX_TRAINS];

extern int total_trains; // How many loaded from file

// ----------------------------------------------------------------------------
// GLOBAL STATE: SWITCHES (A-Z mapped to 0-25)
// ----------------------------------------------------------------------------
extern int switch_x[MAX_SWITCHES];
extern int switch_y[MAX_SWITCHES];
extern int switch_state[MAX_SWITCHES];   // 0 or 1
extern bool switch_active[MAX_SWITCHES]; // Does this switch exist?
extern bool switch_is_global[MAX_SWITCHES]; // GLOBAL vs PER_DIR

// K-Values and Counters [SwitchID][Direction 0-3]
extern int switch_k_values[MAX_SWITCHES][4];
extern int switch_counters[MAX_SWITCHES][4];
extern bool switch_flip_queued[MAX_SWITCHES]; // Pending flip for next tick

// ----------------------------------------------------------------------------
// GLOBAL STATE: SIMULATION
// ----------------------------------------------------------------------------
extern int current_tick;
extern int simulation_seed;

// ----------------------------------------------------------------------------
// INITIALIZATION FUNCTION
// ----------------------------------------------------------------------------
void initializeSimulationState();

#endif
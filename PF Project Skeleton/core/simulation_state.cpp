#include "simulation_state.h"

char grid[MAX_ROWS][MAX_COLS];
int grid_rows = 0;
int grid_cols = 0;

// TRAINS
int train_count = 0;  // FIXED: Added missing variable (required by app.cpp)

int train_id[MAX_TRAINS];
int train_x[MAX_TRAINS];
int train_y[MAX_TRAINS];
int train_direction[MAX_TRAINS];
int train_color[MAX_TRAINS];
int train_spawn_tick[MAX_TRAINS];
bool train_active[MAX_TRAINS];
bool train_finished[MAX_TRAINS];
int train_arrival_tick[MAX_TRAINS];

int train_next_x[MAX_TRAINS];
int train_next_y[MAX_TRAINS];
int train_dest_x[MAX_TRAINS];
int train_dest_y[MAX_TRAINS];
int train_prev_x[MAX_TRAINS];
int train_prev_y[MAX_TRAINS];

int total_trains = 0;

// SWITCHES
int switch_x[MAX_SWITCHES];
int switch_y[MAX_SWITCHES];
int switch_state[MAX_SWITCHES];
bool switch_active[MAX_SWITCHES];
bool switch_is_global[MAX_SWITCHES];

int switch_k_values[MAX_SWITCHES][4];
int switch_counters[MAX_SWITCHES][4];
bool switch_flip_queued[MAX_SWITCHES];

// SIMULATION
int current_tick = 0;
int simulation_seed = 0;

void initializeSimulationState() {
    grid_rows = 0;
    grid_cols = 0;

    // Initialize grid with dots
    for (int r = 0; r < MAX_ROWS; r++) {
        for (int c = 0; c < MAX_COLS; c++) {
            grid[r][c] = '.';
        }
    }

    total_trains = 0;
    train_count = 0;  // FIXED: Initialize train_count

    for (int i = 0; i < MAX_TRAINS; i++) {
        train_id[i] = -1;
        train_active[i] = false;
        train_finished[i] = false;

        train_x[i] = -1;
        train_y[i] = -1;

        train_next_x[i] = -1;
        train_next_y[i] = -1;

        train_prev_x[i] = -1;
        train_prev_y[i] = -1;

        train_dest_x[i] = -1;
        train_dest_y[i] = -1;

        train_direction[i] = 0;
        train_color[i] = 0;
        train_spawn_tick[i] = 0;
    }

    for (int i = 0; i < MAX_SWITCHES; i++) {
        switch_active[i] = false;
        switch_flip_queued[i] = false;

        switch_state[i] = 0;
        switch_x[i] = -1;
        switch_y[i] = -1;

        switch_is_global[i] = false;

        for (int d = 0; d < 4; d++) {
            switch_k_values[i][d] = 0;
            switch_counters[i][d] = 0;
        }
    }

    current_tick = 0;
    simulation_seed = 0;
}
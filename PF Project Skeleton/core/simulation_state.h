#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

const int MAX_ROWS = 100;
const int MAX_COLS = 100;
const int MAX_TRAINS = 50;
const int MAX_SWITCHES = 26;

const int DIR_UP = 0;
const int DIR_RIGHT = 1;
const int DIR_DOWN = 2;
const int DIR_LEFT = 3;

extern char grid[MAX_ROWS][MAX_COLS];
extern int grid_rows;
extern int grid_cols;

extern int train_count;

extern int train_id[MAX_TRAINS];
extern int train_x[MAX_TRAINS];
extern int train_y[MAX_TRAINS];
extern int train_direction[MAX_TRAINS];
extern int train_color[MAX_TRAINS];
extern int train_spawn_tick[MAX_TRAINS];
extern bool train_active[MAX_TRAINS];
extern bool train_finished[MAX_TRAINS];
extern int train_arrival_tick[MAX_TRAINS];

extern int train_next_x[MAX_TRAINS];
extern int train_next_y[MAX_TRAINS];
extern int train_dest_x[MAX_TRAINS];
extern int train_dest_y[MAX_TRAINS];

extern int total_trains;

extern int train_prev_x[MAX_TRAINS];
extern int train_prev_y[MAX_TRAINS];

extern int switch_x[MAX_SWITCHES];
extern int switch_y[MAX_SWITCHES];
extern int switch_state[MAX_SWITCHES];
extern bool switch_active[MAX_SWITCHES];
extern bool switch_is_global[MAX_SWITCHES];

extern int switch_k_values[MAX_SWITCHES][4];
extern int switch_counters[MAX_SWITCHES][4];
extern bool switch_flip_queued[MAX_SWITCHES];

extern int current_tick;
extern int simulation_seed;

void initializeSimulationState();

#endif
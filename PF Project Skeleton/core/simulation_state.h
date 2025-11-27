#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

// ==========================================
// 1. CONSTANTS (The Rules of the World)
// ==========================================

// Directions (0-3 for easy math: Clockwise)
const int UP = 0;
const int RIGHT = 1;
const int DOWN = 2;
const int LEFT = 3;
const int NONE = 4;

// Limits (The maximum sizes of your arrays - PDF Page 6/11)
const int MAX_ROWS = 20;
const int MAX_COLS = 30;
const int MAX_TRAINS = 50;
const int MAX_SWITCHES = 26; // A-Z

// ==========================================
// 2. FUNCTION PROTOTYPES (The Promises)
// ==========================================

// HELPER: Resets all data before loading a new level
void initializeSimulation(int t_active[], int t_count, int s_count, int& rows, int& cols);

// MEMBER A (Input/Output)
// Reads the .lvl file and fills all these arrays
void loadLevel(const char* filename, 
               char grid[MAX_ROWS][MAX_COLS], int& rows, int& cols,
               int t_id[], int t_x[], int t_y[], int t_dir[], int t_dest_x[], int t_dest_y[], 
               int t_spawn[], int t_color[], int t_active[], int& t_count,
               char s_id[], int s_x[], int s_y[], int s_type[], int s_k[][4], int s_counter[][4], int& s_count);

// Prints the grid to the terminal (Phase 1 Visualization)
void printGrid(char grid[MAX_ROWS][MAX_COLS], int rows, int cols, 
               int t_x[], int t_y[], int t_id[], int t_active[], int t_count);

// MEMBER B (Logic)
// Moves trains and calculates the "Next" position
void moveTrains(int t_x[], int t_y[], int t_next_x[], int t_next_y[], 
                int t_dir[], int t_dest_x[], int t_dest_y[], int t_active[], int t_count, 
                char grid[MAX_ROWS][MAX_COLS]);

// Updates switch counters and queues flips
void updateSwitches(int t_x[], int t_y[], int t_active[], int t_count,
                    int s_x[], int s_y[], int s_counter[][4], int s_k[][4], int s_type[], int s_count);

// Detects collisions using Manhattan Distance Priority
void detectCollisions(int t_x[], int t_y[], int t_next_x[], int t_next_y[],
                      int t_dest_x[], int t_dest_y[], int t_active[], int t_count);

#endif
#include "trains.h"
#include "simulation_state.h"
#include "grid.h"
#include "switches.h" // Needed to check switch states
#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace std;

// ============================================================================
// TRAINS.CPP - Train logic
// ============================================================================

// ----------------------------------------------------------------------------
// HELPER: Manhattan Distance
// ----------------------------------------------------------------------------
int getManhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// ----------------------------------------------------------------------------
// SPAWN TRAINS FOR CURRENT TICK
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// SPAWN TRAINS FOR CURRENT TICK (FIXED)
// ----------------------------------------------------------------------------
void spawnTrainsForTick() {
    for (int i = 0; i < total_trains; i++) {

        if (!train_active[i] && !train_finished[i] &&
            train_spawn_tick[i] == current_tick) {

            int sx = train_x[i];
            int sy = train_y[i];

            // Check if spawn is blocked
            bool blocked = false;
            for(int j=0; j<total_trains; j++) {
                if (train_active[j] &&
                    train_x[j] == sx && train_y[j] == sy) {
                    blocked = true;
                    break;
                }
            }

            if (!blocked) {
                train_active[i] = true;
                // train_x & train_y already correct 
            }
        }
    }
}


// ----------------------------------------------------------------------------
// HELPER: GET NEXT DIRECTION
// ----------------------------------------------------------------------------
int getNextDirection(int trainIdx) {
    int cx = train_x[trainIdx];
    int cy = train_y[trainIdx];
    int cdir = train_direction[trainIdx];
    
    // 1. Switches
    if (isSwitchTile(cx, cy)) {
        int swIdx = getSwitchIndex(cx, cy);
        if (swIdx != -1) {
            // If state is 0 (Straight), keep direction (usually)
            // If state is 1 (Turn), change direction
            if (switch_state[swIdx] == 1) {
                // Logic depends on entry direction. 
                // Simplified: If moving Right, Turn Down
                if(cdir == DIR_RIGHT) return DIR_DOWN;
                if(cdir == DIR_DOWN) return DIR_RIGHT; 
                if(cdir == DIR_LEFT) return DIR_UP;
                if(cdir == DIR_UP) return DIR_LEFT;
            }
        }
    }
    
    // 2. Curves (Simple logic based on character)
    char t = grid[cy][cx];
    if (t == '/') {
        if (cdir == DIR_RIGHT) return DIR_UP;
        if (cdir == DIR_DOWN) return DIR_LEFT;
        if (cdir == DIR_LEFT) return DIR_DOWN;
        if (cdir == DIR_UP) return DIR_RIGHT;
    }
    else if (t == '\\') {
        if (cdir == DIR_RIGHT) return DIR_DOWN;
        if (cdir == DIR_UP) return DIR_LEFT;
        if (cdir == DIR_LEFT) return DIR_UP;
        if (cdir == DIR_DOWN) return DIR_RIGHT;
    }
    
    return cdir;
}

// ----------------------------------------------------------------------------
// DETERMINE ALL ROUTES (Phase 2)
// ----------------------------------------------------------------------------
void determineAllRoutes() {
    for (int i = 0; i < total_trains; i++) {
        if (!train_active[i]) continue;
        
        // 1. Update Direction based on current tile (Switch/Curve)
        train_direction[i] = getNextDirection(i);

        // 2. Calculate Proposed Next Coordinate
        int dx = 0, dy = 0;
        if (train_direction[i] == DIR_UP) dy = -1;
        else if (train_direction[i] == DIR_DOWN) dy = 1;
        else if (train_direction[i] == DIR_LEFT) dx = -1;
        else if (train_direction[i] == DIR_RIGHT) dx = 1;
        
        train_next_x[i] = train_x[i] + dx;
        train_next_y[i] = train_y[i] + dy;
    }
}

// ----------------------------------------------------------------------------
// COLLISION DETECTION (Phase 5)
// ----------------------------------------------------------------------------
void detectCollisions() {
    for (int i = 0; i < total_trains; i++) {
        if (!train_active[i]) continue;

        for (int j = i + 1; j < total_trains; j++) {
            if (!train_active[j]) continue;

            // Check if they want to enter the same tile
            if (train_next_x[i] == train_next_x[j] && train_next_y[i] == train_next_y[j]) {
                
                // Manhattan Distance Logic
                int distI = getManhattanDistance(train_x[i], train_y[i], train_dest_x[i], train_dest_y[i]);
                int distJ = getManhattanDistance(train_x[j], train_y[j], train_dest_x[j], train_dest_y[j]);

                if (distI > distJ) {
                    // I is further, I moves. J waits.
                    train_next_x[j] = train_x[j];
                    train_next_y[j] = train_y[j];
                } else {
                    // J is further (or equal), J moves. I waits.
                    train_next_x[i] = train_x[i];
                    train_next_y[i] = train_y[i];
                }
            }
            // Check Head-on Swap (I goes to J's spot, J goes to I's spot)
            else if (train_next_x[i] == train_x[j] && train_next_y[i] == train_y[j] &&
                     train_next_x[j] == train_x[i] && train_next_y[j] == train_y[i]) {
                         
                int distI = getManhattanDistance(train_x[i], train_y[i], train_dest_x[i], train_dest_y[i]);
                int distJ = getManhattanDistance(train_x[j], train_y[j], train_dest_x[j], train_dest_y[j]);
                
                if (distI > distJ) {
                    train_next_x[j] = train_x[j]; // J waits
                    train_next_y[j] = train_y[j];
                } else {
                    train_next_x[i] = train_x[i]; // I waits
                    train_next_y[i] = train_y[i];
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// MOVE ALL TRAINS
// ----------------------------------------------------------------------------
void moveAllTrains() {
    for (int i = 0; i < total_trains; i++) {
        if (train_active[i]) {
            // Only move if the next tile is valid and in bounds
            if (isInBounds(train_next_x[i], train_next_y[i])) {
                int nx = train_next_x[i];
                int ny = train_next_y[i];

            if (nx < 0 || nx >= grid_cols || ny < 0 || ny >= grid_rows ||
                grid[ny][nx] == '.' || grid[ny][nx] == ' ')
            {
                // Off map OR landing on empty cell => train is considered delivered/removed
                train_active[i] = false;
                train_finished[i] = true;
                continue;
            }

                train_prev_x[i] = train_x[i];
                train_prev_y[i] = train_y[i];
                train_x[i] = train_next_x[i];
                train_y[i] = train_next_y[i];

                
            }
        }
    }
}

// ----------------------------------------------------------------------------
// CHECK ARRIVALS
// ----------------------------------------------------------------------------
void checkArrivals() {
    for (int i = 0; i < total_trains; i++) {
        if (train_active[i]) {
            if (isDestinationPoint(train_x[i], train_y[i])) {
                train_active[i] = false;
                train_finished[i] = true;
                // Ideally log this event
            }
        }
    }
}

/* Stubs for currently unused helpers
bool determineNextPosition(int trainIdx) { return true; }
int getSmartDirectionAtCrossing(int trainIdx) { return 0; }
void applyEmergencyHalt() {}
void updateEmergencyHalt() {} */
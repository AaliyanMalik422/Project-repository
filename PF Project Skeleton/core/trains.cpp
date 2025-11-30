#include "trains.h"
#include "simulation_state.h"
#include "grid.h"
#include "switches.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

using namespace std;

int getManhattanDistance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

void spawnTrainsForTick() {
    for (int i = 0; i < total_trains; i++) {

        if (!train_active[i] && !train_finished[i] &&
            train_spawn_tick[i] <= current_tick) {

            // Find 'S' tile closest to this train's destination
            int sx = -1, sy = -1;
            int minDist = 99999;
            
            for (int r = 0; r < grid_rows; r++) {
                for (int c = 0; c < grid_cols; c++) {
                    if (grid[r][c] == 'S') {
                        int dist = abs(r - train_dest_y[i]) + abs(c - train_dest_x[i]);
                        if (dist < minDist) {
                            minDist = dist;
                            sx = c;
                            sy = r;
                        }
                    }
                }
            }
            
            if (sx == -1) continue; // No 'S' found

            // Check if spawn location is blocked
            bool blocked = false;
            for(int j = 0; j < total_trains; j++) {
                if (train_active[j] &&
                    train_x[j] == sx && train_y[j] == sy) {
                    blocked = true;
                    break;
                }
            }

            if (!blocked) {
                train_x[i] = sx;
                train_y[i] = sy;
                train_direction[i] = 0; // Start East (will be corrected by getNextDirection)
                train_active[i] = true;
            }
        }
    }
}





int getNextDirection(int trainIdx) {
    int cx = train_x[trainIdx];
    int cy = train_y[trainIdx];
    int cdir = train_direction[trainIdx];
    
    // Check bounds
    if (cy < 0 || cy >= grid_rows || cx < 0 || cx >= grid_cols) {
        return cdir; // Keep current direction if out of bounds
    }
    
    char t = grid[cy][cx];
    
    // Handle switches FIRST (they override track behavior)
    if (isSwitchTile(cx, cy)) {
        int swIdx = getSwitchIndex(cx, cy);
        if (swIdx != -1) {
            if (switch_state[swIdx] == 1) { // TURN state
                if(cdir == DIR_RIGHT) return DIR_DOWN;
                if(cdir == DIR_DOWN) return DIR_RIGHT; 
                if(cdir == DIR_LEFT) return DIR_UP;
                if(cdir == DIR_UP) return DIR_LEFT;
            }
            // If switch is STRAIGHT (state 0), fall through to normal track handling
        }
    }
    
    // Handle curved tracks
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
    // Handle straight tracks - they maintain direction
    else if (t == '=' || t == '-' || t == '|' || t == '+' || 
             t == 'S' || t == 'D' || t == 'A' || t == 'B' || 
             t == 'C' || t == 'E' || t == 'F' || t == 'G' ||
             t == 'H' || t == 'I' || t == 'J' || t == 'K' || t == 'L') {
        // Straight track or junction - keep moving in current direction
        return cdir;
    }
    
    // Default: keep current direction
    return cdir;
}

void determineAllRoutes() {
    for (int i = 0; i < total_trains; i++) {
        if (!train_active[i]) continue;
        
        train_direction[i] = getNextDirection(i);

        int dx = 0, dy = 0;
        if (train_direction[i] == DIR_UP) dy = -1;
        else if (train_direction[i] == DIR_DOWN) dy = 1;
        else if (train_direction[i] == DIR_LEFT) dx = -1;
        else if (train_direction[i] == DIR_RIGHT) dx = 1;
        
        train_next_x[i] = train_x[i] + dx;
        train_next_y[i] = train_y[i] + dy;
    }
}

void detectCollisions() {
    for (int i = 0; i < total_trains; i++) {
        if (!train_active[i]) continue;

        for (int j = i + 1; j < total_trains; j++) {
            if (!train_active[j]) continue;

            if (train_next_x[i] == train_next_x[j] && train_next_y[i] == train_next_y[j]) {
                
                int distI = getManhattanDistance(train_x[i], train_y[i], train_dest_x[i], train_dest_y[i]);
                int distJ = getManhattanDistance(train_x[j], train_y[j], train_dest_x[j], train_dest_y[j]);

                if (distI > distJ) {
                    train_next_x[j] = train_x[j];
                    train_next_y[j] = train_y[j];
                } else {
                    train_next_x[i] = train_x[i];
                    train_next_y[i] = train_y[i];
                }
            }
            else if (train_next_x[i] == train_x[j] && train_next_y[i] == train_y[j] &&
                     train_next_x[j] == train_x[i] && train_next_y[j] == train_y[i]) {
                
                int distI = getManhattanDistance(train_x[i], train_y[i], train_dest_x[i], train_dest_y[i]);
                int distJ = getManhattanDistance(train_x[j], train_y[j], train_dest_x[j], train_dest_y[j]);

                if (distI > distJ) {
                    train_next_x[j] = train_x[j];
                    train_next_y[j] = train_y[j];
                } else {
                    train_next_x[i] = train_x[i];
                    train_next_y[i] = train_y[i];
                }
            }
        }
    }
}

void moveAllTrains() {
    for (int i = 0; i < total_trains; i++) {
        if (!train_active[i]) continue;
        
        // CHECK IF ALREADY AT DESTINATION BEFORE MOVING
        char currentTile = grid[train_y[i]][train_x[i]];
        if (currentTile == 'D') {  // ✅ Just check for 'D'!
            train_finished[i] = true;
            train_active[i] = false;
            train_arrival_tick[i] = current_tick;
            continue;
        }
        
        // UPDATE DIRECTION based on current tile
        train_direction[i] = getNextDirection(i);
        
        // THEN calculate next position
        int nextX = train_x[i];
        int nextY = train_y[i];
        
        if (train_direction[i] == 0) nextX++; // East
        else if (train_direction[i] == 1) nextY++; // South
        else if (train_direction[i] == 2) nextX--; // West
        else if (train_direction[i] == 3) nextY--; // North
        
        // Bounds check
        if (nextX < 0 || nextX >= grid_cols || nextY < 0 || nextY >= grid_rows) {
            continue;
        }
        
        // Update position
        train_x[i] = nextX;
        train_y[i] = nextY;
        
        // Check if JUST ARRIVED at destination
        char nextTile = grid[nextY][nextX];
        if (nextTile == 'D') {  // ✅ Just check for 'D'!
            train_finished[i] = true;
            train_active[i] = false;
            train_arrival_tick[i] = current_tick;
        }
    }
}





void checkArrivals() {
    for (int i = 0; i < total_trains; i++) {
        if (!train_active[i] || train_finished[i]) continue;

        if (train_x[i] == train_dest_x[i] && train_y[i] == train_dest_y[i]) {
            train_finished[i] = true;
            train_active[i] = false;
            train_arrival_tick[i] = current_tick;
        }
    }
}


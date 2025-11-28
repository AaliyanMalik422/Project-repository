#include<iostream>
#include "grid.h"
#include "simulation_state.h"

using namespace std;

// ACCESS THE SHARED GLOBAL STATE
extern SimulationState state;
// ============================================================================
// GRID.CPP - Grid utilities
// ============================================================================

// ----------------------------------------------------------------------------
// Check if a position is inside the grid.
// ----------------------------------------------------------------------------
// Returns true if x,y are within bounds.
// ----------------------------------------------------------------------------
bool isInBounds(int x, int y) {
    return (x >= 0 && x < state.cols && y >= 0 && y < state.rows);
}

// ----------------------------------------------------------------------------
// Check if a tile is a track tile.
// ----------------------------------------------------------------------------
// Returns true if the tile can be traversed by trains.
// ----------------------------------------------------------------------------
bool isTrackTile(int x , int y) {
    if (!isInBounds(x, y)) return false;
    
    char c = state.grid[y][x];
    // Assuming space ' ' or dot '.' represents empty grass/void
    return (c != ' ' && c != '.');
}

// ----------------------------------------------------------------------------
// Check if a tile is a switch.
// ----------------------------------------------------------------------------
// Returns true if the tile is 'A'..'Z'.
// ----------------------------------------------------------------------------
bool isSwitchTile(int x , int y) {
    if (!isInBounds(x, y)) return false;
    
    char c = state.grid[y][x];
    return (c >= 'A' && c <= 'Z');
}

// ----------------------------------------------------------------------------
// Get switch index from character.
// ----------------------------------------------------------------------------
// Maps 'A'..'Z' to 0..25, else -1.
// ----------------------------------------------------------------------------
int getSwitchIndex(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    }
    return -1; // Not a switch
}

// ----------------------------------------------------------------------------
// Check if a position is a spawn point.
// ----------------------------------------------------------------------------
// Returns true if x,y is a spawn.
// ----------------------------------------------------------------------------
bool isSpawnPoint(int x , int y) {
    if (!isInBounds(x, y)) return false;
    return (state.grid[y][x] == 'S');
}

// ----------------------------------------------------------------------------
// Check if a position is a destination.
// ----------------------------------------------------------------------------
// Returns true if x,y is a destination.
// ----------------------------------------------------------------------------
bool isDestinationPoint(int x , int y) {
    if (!isInBounds(x, y)) return false;
    return (state.grid[y][x] == 'D'); // Assuming 'D' is Destination
}

// ----------------------------------------------------------------------------
// Toggle a safety tile.
// ----------------------------------------------------------------------------
// Returns true if toggled successfully.
// ----------------------------------------------------------------------------
bool toggleSafetyTile(int x , int y) {
    if (!isSwitchTile(x, y)) return false;

    // Find the switch at this location and flip its state
    for (int i = 0; i < state.switchCount; i++) {
        if (state.switches[i].x == x && state.switches[i].y == y) {
            // Toggle 0 -> 1 or 1 -> 0
            if (state.switches[i].state == 0) state.switches[i].state = 1;
            else state.switches[i].state = 0;
            return true;
        }
    }
    return false;
}

// CRITICAL FIX 3: THE VISUALIZATION
// You must have this to see the map!
void printGrid() {
    cout << "Tick: " << state.currentTick << endl;

    for (int i = 0; i < state.rows; i++) {
        for (int j = 0; j < state.cols; j++) {
            
            bool trainHere = false;
            
            // Check for trains at this exact spot
            for (int t = 0; t < state.trainCount; t++) {
                if (state.trains[t].active && 
                    state.trains[t].x == j && 
                    state.trains[t].y == i) {
                    
                    cout << state.trains[t].id; 
                    trainHere = true;
                    break; 
                }
            }

            // If no train, print the map char
            if (!trainHere) {
                cout << state.grid[i][j];
            }
        }
        cout << endl;
    }
    cout << "--------------------------------" << endl;
}

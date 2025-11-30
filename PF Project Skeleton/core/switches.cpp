#include "switches.h"
#include "simulation_state.h"
#include <iostream>

using namespace std;

int total_switches=0; 
// Check if a tile is a switch
bool isSwitchTile(int x, int y) {
    if (x < 0 || x >= grid_cols || y < 0 || y >= grid_rows) return false;
    char tile = grid[y][x];
    return (tile == 'A' || tile == 'B');
}

// Get the switch index at position (x, y), return -1 if not a switch
int getSwitchIndex(int x, int y) {
    for (int i = 0; i < total_switches; ++i) {
        if (switch_x[i] == x && switch_y[i] == y) {
            return i;
        }
    }
    return -1;
}

// Toggle a switch between state A and B
void toggleSwitch(int switchIndex) {
    if (switchIndex < 0 || switchIndex >= total_switches) return;
    
    // Toggle: 'A' <-> 'B'
    if (switch_state[switchIndex] == 'A') {
        switch_state[switchIndex] = 'B';
    } else {
        switch_state[switchIndex] = 'A';
    }
    
    cout << "Switch " << switchIndex << " toggled to state " << switch_state[switchIndex] << endl;
}

// Initialize switches (called once at simulation start)
void initializeSwitches() {
    for (int i = 0; i < total_switches; ++i) {
        // Switches start in state 'A' by default
        switch_state[i] = 'A';
    }
    cout << "Switches initialized: " << total_switches << " total\n";
}
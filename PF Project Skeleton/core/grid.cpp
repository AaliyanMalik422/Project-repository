#include "grid.h"
#include "simulation_state.h"

// ============================================================================
// GRID.CPP - Grid utilities
// ============================================================================

// ----------------------------------------------------------------------------
// Check if a position is inside the grid.
// ----------------------------------------------------------------------------
bool isInBounds(int x, int y) {
    // Uses global grid_cols/rows from simulation_state.h
    return (x >= 0 && x < grid_cols && y >= 0 && y < grid_rows);
}

// ----------------------------------------------------------------------------
// Check if a tile is a track tile.
// ----------------------------------------------------------------------------
// Returns true if the tile is any valid rail component
bool isTrackTile(int x, int y) {
    if (!isInBounds(x, y)) return false;
    char t = grid[y][x];
    // Includes standard rails, curves, crossings, spawn, dest, safety, and switches
    return (t == '-' || t == '|' || t == '/' || t == '\\' || 
            t == '+' || t == 'S' || t == 'D' || t == '=' || 
            (t >= 'A' && t <= 'Z'));
}

// ----------------------------------------------------------------------------
// Check if a tile is a switch.
// ----------------------------------------------------------------------------
bool isSwitchTile(int x, int y) {
    if (!isInBounds(x, y)) return false;
    char t = grid[y][x];
    return (t >= 'A' && t <= 'Z');
}

// ----------------------------------------------------------------------------
// Get switch index from character.
// ----------------------------------------------------------------------------
int getSwitchIndex(int x, int y) {
    if (!isSwitchTile(x, y)) return -1;
    return grid[y][x] - 'A';
}

// ----------------------------------------------------------------------------
// Check if a position is a spawn point.
// ----------------------------------------------------------------------------
bool isSpawnPoint(int x, int y) {
    if (!isInBounds(x, y)) return false;
    return grid[y][x] == 'S';
}

// ----------------------------------------------------------------------------
// Check if a position is a destination.
// ----------------------------------------------------------------------------
bool isDestinationPoint(int x, int y) {
    if (!isInBounds(x, y)) return false;
    return grid[y][x] == 'D';
}

// ----------------------------------------------------------------------------
// Toggle a safety tile.
// ----------------------------------------------------------------------------
bool toggleSafetyTile(int x, int y) {
    if (!isInBounds(x, y)) return false;
    
    // Can only place safety on straight tracks or remove existing safety
    if (grid[y][x] == '-') {
        grid[y][x] = '=';
        return true;
    } else if (grid[y][x] == '=') {
        grid[y][x] = '-';
        return true;
    }
    return false;
}
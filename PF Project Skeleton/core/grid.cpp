#include "grid.h"
#include "simulation_state.h"

bool isInBounds(int x, int y) {
    return x >= 0 && x < grid_cols && y >= 0 && y < grid_rows;
}

bool isSwitchTile(int x, int y) {
    if (!isInBounds(x, y)) return false;

    const char tile = grid[y][x];
    if (tile == 'S') {
        const int sourceOrSwitch = 'S' - 'A';
        return switch_active[sourceOrSwitch] &&
               switch_x[sourceOrSwitch] == x &&
               switch_y[sourceOrSwitch] == y;
    }
    return tile >= 'A' && tile <= 'Z' && tile != 'D';
}

int getSwitchIndex(int x, int y) {
    if (!isSwitchTile(x, y)) return -1;
    return grid[y][x] - 'A';
}

bool isTrackTile(int x, int y) {
    if (!isInBounds(x, y)) return false;

    const char tile = grid[y][x];
    return tile == '-' || tile == '=' || tile == '|' ||
           tile == '/' || tile == '\\' || tile == '+' ||
           tile == 'S' || tile == 'D' || isSwitchTile(x, y);
}

bool isSpawnPoint(int x, int y) {
    return isInBounds(x, y) && grid[y][x] == 'S' && !isSwitchTile(x, y);
}

bool isDestinationPoint(int x, int y) {
    return isInBounds(x, y) && grid[y][x] == 'D';
}

bool toggleSafetyTile(int x, int y) {
    if (!isInBounds(x, y)) return false;

    if (grid[y][x] == '-') {
        grid[y][x] = '=';
        return true;
    }

    if (grid[y][x] == '=') {
        grid[y][x] = '-';
        return true;
    }

    return false;
}

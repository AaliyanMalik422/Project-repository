#include "trains.h"
#include "simulation_state.h"
#include "grid.h"
#include "switches.h"

#include <cstdlib>

int getManhattanDistance(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}

static void directionDelta(int direction, int &dx, int &dy) {
    dx = 0;
    dy = 0;

    if (direction == DIR_UP) dy = -1;
    else if (direction == DIR_RIGHT) dx = 1;
    else if (direction == DIR_DOWN) dy = 1;
    else if (direction == DIR_LEFT) dx = -1;
}

static int oppositeDirection(int direction) {
    return (direction + 2) % 4;
}

static int chooseInitialDirection(int x, int y, int destinationX, int destinationY) {
    const int candidates[4] = {DIR_RIGHT, DIR_DOWN, DIR_LEFT, DIR_UP};
    int bestDirection = DIR_RIGHT;
    int bestDistance = 1000000;

    for (int i = 0; i < 4; ++i) {
        int dx, dy;
        directionDelta(candidates[i], dx, dy);
        const int nx = x + dx;
        const int ny = y + dy;

        if (!isTrackTile(nx, ny)) continue;

        const int distance = getManhattanDistance(nx, ny, destinationX, destinationY);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestDirection = candidates[i];
        }
    }

    return bestDirection;
}

void spawnTrainsForTick() {
    for (int i = 0; i < total_trains; ++i) {
        if (train_active[i] || train_finished[i] || train_crashed[i] ||
            train_spawn_tick[i] > current_tick) {
            continue;
        }

        const int sx = train_spawn_x[i];
        const int sy = train_spawn_y[i];
        if (!isSpawnPoint(sx, sy)) continue;

        bool occupied = false;
        for (int j = 0; j < total_trains; ++j) {
            if (i != j && train_active[j] && train_x[j] == sx && train_y[j] == sy) {
                occupied = true;
                break;
            }
        }

        if (occupied) continue;

        train_x[i] = sx;
        train_y[i] = sy;
        train_direction[i] = chooseInitialDirection(sx, sy, train_dest_x[i], train_dest_y[i]);
        train_next_x[i] = sx;
        train_next_y[i] = sy;
        train_active[i] = true;
    }
}

int getSmartDirectionAtCrossing(int trainIdx) {
    const int x = train_x[trainIdx];
    const int y = train_y[trainIdx];
    const int currentDirection = train_direction[trainIdx];
    const int candidates[4] = {DIR_UP, DIR_RIGHT, DIR_DOWN, DIR_LEFT};

    int bestDirection = currentDirection;
    int bestDistance = getManhattanDistance(
        x, y, train_dest_x[trainIdx], train_dest_y[trainIdx]);

    for (int i = 0; i < 4; ++i) {
        const int direction = candidates[i];
        if (direction == oppositeDirection(currentDirection)) continue;

        int dx, dy;
        directionDelta(direction, dx, dy);
        const int nx = x + dx;
        const int ny = y + dy;
        if (!isTrackTile(nx, ny)) continue;

        const int distance = getManhattanDistance(
            nx, ny, train_dest_x[trainIdx], train_dest_y[trainIdx]);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestDirection = direction;
        }
    }

    return bestDirection;
}

int getNextDirection(int trainIdx) {
    if (trainIdx < 0 || trainIdx >= total_trains) return DIR_RIGHT;

    const int x = train_x[trainIdx];
    const int y = train_y[trainIdx];
    if (!isInBounds(x, y)) return train_direction[trainIdx];

    const int currentDirection = train_direction[trainIdx];
    const char tile = grid[y][x];

    if (isSwitchTile(x, y)) {
        const int switchIndex = getSwitchIndex(x, y);
        if (switchIndex >= 0 && switch_state[switchIndex] == 1) {
            int turnDirection = currentDirection;

            if (currentDirection == DIR_RIGHT) turnDirection = DIR_DOWN;
            else if (currentDirection == DIR_DOWN) turnDirection = DIR_RIGHT;
            else if (currentDirection == DIR_LEFT) turnDirection = DIR_UP;
            else if (currentDirection == DIR_UP) turnDirection = DIR_LEFT;

            int dx, dy;
            directionDelta(turnDirection, dx, dy);
            if (isTrackTile(x + dx, y + dy)) return turnDirection;
        }
        return currentDirection;
    }

    if (tile == '/') {
        if (currentDirection == DIR_RIGHT) return DIR_UP;
        if (currentDirection == DIR_DOWN) return DIR_LEFT;
        if (currentDirection == DIR_LEFT) return DIR_DOWN;
        if (currentDirection == DIR_UP) return DIR_RIGHT;
    } else if (tile == '\\') {
        if (currentDirection == DIR_RIGHT) return DIR_DOWN;
        if (currentDirection == DIR_UP) return DIR_LEFT;
        if (currentDirection == DIR_LEFT) return DIR_UP;
        if (currentDirection == DIR_DOWN) return DIR_RIGHT;
    } else if (tile == '+') {
        return getSmartDirectionAtCrossing(trainIdx);
    }

    return currentDirection;
}

bool determineNextPosition(int trainIdx) {
    if (trainIdx < 0 || trainIdx >= total_trains || !train_active[trainIdx]) {
        return false;
    }

    const int direction = getNextDirection(trainIdx);
    int dx, dy;
    directionDelta(direction, dx, dy);

    const int nx = train_x[trainIdx] + dx;
    const int ny = train_y[trainIdx] + dy;

    if (!isTrackTile(nx, ny)) {
        train_next_x[trainIdx] = train_x[trainIdx];
        train_next_y[trainIdx] = train_y[trainIdx];
        return false;
    }

    train_direction[trainIdx] = direction;
    train_next_x[trainIdx] = nx;
    train_next_y[trainIdx] = ny;
    return true;
}

void determineAllRoutes() {
    for (int i = 0; i < total_trains; ++i) {
        if (!train_active[i]) continue;
        determineNextPosition(i);
    }
}

void detectCollisions() {
    bool wait[MAX_TRAINS] = {};
    bool crash[MAX_TRAINS] = {};

    for (int i = 0; i < total_trains; ++i) {
        if (!train_active[i]) continue;

        for (int j = i + 1; j < total_trains; ++j) {
            if (!train_active[j]) continue;

            const bool sameDestination =
                train_next_x[i] == train_next_x[j] &&
                train_next_y[i] == train_next_y[j];

            const bool headOnSwap =
                train_next_x[i] == train_x[j] &&
                train_next_y[i] == train_y[j] &&
                train_next_x[j] == train_x[i] &&
                train_next_y[j] == train_y[i];

            if (!sameDestination && !headOnSwap) continue;

            const int distanceI = getManhattanDistance(
                train_x[i], train_y[i], train_dest_x[i], train_dest_y[i]);
            const int distanceJ = getManhattanDistance(
                train_x[j], train_y[j], train_dest_x[j], train_dest_y[j]);

            if (distanceI == distanceJ) {
                crash[i] = true;
                crash[j] = true;
            } else if (headOnSwap) {
                // A train cannot safely enter a tile occupied by a train that
                // has been told to wait. Hold both until the route is clear.
                wait[i] = true;
                wait[j] = true;
            } else if (distanceI > distanceJ) {
                wait[j] = true;
            } else {
                wait[i] = true;
            }
        }
    }

    // Prevent a moving train from entering a tile occupied by a train that
    // is stationary (including one held by another collision).
    for (int pass = 0; pass < total_trains; ++pass) {
        bool changed = false;
        for (int i = 0; i < total_trains; ++i) {
            if (!train_active[i] || wait[i] || crash[i]) continue;

            for (int j = 0; j < total_trains; ++j) {
                if (i == j || !train_active[j] || crash[j]) continue;

                const int jNextX = wait[j] ? train_x[j] : train_next_x[j];
                const int jNextY = wait[j] ? train_y[j] : train_next_y[j];

                if (train_next_x[i] == train_x[j] &&
                    train_next_y[i] == train_y[j] &&
                    jNextX == train_x[j] && jNextY == train_y[j]) {
                    wait[i] = true;
                    changed = true;
                    break;
                }
            }
        }
        if (!changed) break;
    }

    for (int i = 0; i < total_trains; ++i) {
        if (!train_active[i]) continue;

        if (crash[i]) {
            train_active[i] = false;
            train_crashed[i] = true;
            train_next_x[i] = train_x[i];
            train_next_y[i] = train_y[i];
        } else if (wait[i]) {
            train_next_x[i] = train_x[i];
            train_next_y[i] = train_y[i];
        }
    }
}

void moveAllTrains() {
    for (int i = 0; i < total_trains; ++i) {
        if (!train_active[i]) continue;

        const int nextX = train_next_x[i];
        const int nextY = train_next_y[i];

        if (!isInBounds(nextX, nextY) || !isTrackTile(nextX, nextY)) continue;
        if (nextX == train_x[i] && nextY == train_y[i]) continue;

        train_prev_x[i] = train_x[i];
        train_prev_y[i] = train_y[i];
        train_x[i] = nextX;
        train_y[i] = nextY;

        if (isDestinationPoint(nextX, nextY)) {
            train_finished[i] = true;
            train_active[i] = false;
            train_arrival_tick[i] = current_tick;
        }
    }
}

void checkArrivals() {
    for (int i = 0; i < total_trains; ++i) {
        if (!train_active[i]) continue;

        if (isDestinationPoint(train_x[i], train_y[i])) {
            train_finished[i] = true;
            train_active[i] = false;
            train_arrival_tick[i] = current_tick;
        }
    }
}

void applyEmergencyHalt() {
    // Emergency halt is not configured by the current level-file format.
}

void updateEmergencyHalt() {
    // Reserved for the emergency-halt feature when its level rules are added.
}

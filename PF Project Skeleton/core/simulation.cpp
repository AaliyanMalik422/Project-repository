#include "simulation.h"
#include "simulation_state.h"
#include "trains.h"
#include "switches.h"

#include <cstdlib>

void initializeSimulation() {
    if (simulation_seed != 0) {
        std::srand(static_cast<unsigned int>(simulation_seed));
    }
    initializeSwitches();
}

void simulateOneTick() {
    ++current_tick;

    // Keep this order deterministic: spawn, route, resolve conflicts,
    // move, then mark arrivals. The caller owns output/logging.
    spawnTrainsForTick();
    determineAllRoutes();
    detectCollisions();
    moveAllTrains();
    checkArrivals();
}

bool isSimulationComplete() {
    for (int i = 0; i < total_trains; ++i) {
        if (train_active[i]) return false;
        if (!train_finished[i] && !train_crashed[i]) return false;
    }
    return true;
}

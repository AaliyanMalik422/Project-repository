#include "simulation.h"
#include "simulation_state.h"
#include "trains.h"
#include "switches.h"
#include "io.h"
#include "grid.h"
#include <iostream>
#include <cstdlib>

// ============================================================================
// SIMULATION.CPP - Implementation of main simulation logic
// ============================================================================

// ----------------------------------------------------------------------------
// INITIALIZE SIMULATION
// ----------------------------------------------------------------------------
void initializeSimulation() {
    // If specific setup beyond state reset is needed (e.g. seeding)
    if (simulation_seed != 0) {
        srand(simulation_seed);
    }
 
}

// ----------------------------------------------------------------------------
// SIMULATE ONE TICK
// ----------------------------------------------------------------------------
// Follows the "Tick Timing" order from PDF Page 3
// ----------------------------------------------------------------------------
void simulateOneTick() {
    current_tick++;
    std::cout << "simulateOneTick(): advancing to tick " << current_tick << std::endl;

    // 1. Spawn: Align trains scheduled for this tick
    spawnTrainsForTick();

    // 2. Route Determination: Compute next tile for every train
    determineAllRoutes();


    // 5. Collision Detection & Movement
    // Detect conflicts (Manhattan priority) and update positions
    detectCollisions();
    moveAllTrains();



    // 7. Arrivals: Check if trains reached destination
    checkArrivals();



    // 9. Logging & Output
    // PDF: "At each tick, print grid state to terminal"
    // Also log to CSV files
    logTrainTrace();
    logSwitchState();
    
    // Optional: Print ASCII grid to console (Member A requirement)
    // We can call a helper function from io.h or do it here. 
    // Usually handled by the main loop or io module.
}

// ----------------------------------------------------------------------------
// CHECK IF SIMULATION IS COMPLETE
// ----------------------------------------------------------------------------
bool isSimulationComplete() {
    // Simulation is done if:
    // 1. No trains are currently active on the map
    // 2. All trains from the file have passed their spawn time
    
    for (int i = 0; i < total_trains; i++) {
        // If a train is active, we are not done
        if (train_active[i]) return false;

        // If a train hasn't spawned yet (and hasn't finished), we are not done
        if (!train_finished[i] && train_spawn_tick[i] > current_tick) return false;
    }

    return true;
}
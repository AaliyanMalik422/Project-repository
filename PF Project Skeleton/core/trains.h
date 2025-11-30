#ifndef TRAINS_H
#define TRAINS_H

extern int getManhattanDistance(int x1, int y1, int x2, int y2);
// ============================================================================
// TRAINS.H - Train logic
// ============================================================================

// ----------------------------------------------------------------------------
// TRAIN SPAWNING
// ----------------------------------------------------------------------------
// Spawn trains scheduled for the current tick.
void spawnTrainsForTick();

// ----------------------------------------------------------------------------
// TRAIN ROUTING
// ----------------------------------------------------------------------------
// Compute routes for all active trains (Phase 2).
void determineAllRoutes();

// Helper: Compute next position/direction for a specific train.
// Returns true if a valid move exists.
bool determineNextPosition(int trainIdx);

// Helper: Get next direction when entering a tile (curves, switches).
int getNextDirection(int trainIdx);

// Helper: Choose best direction at a crossing '+' to get closer to D.
int getSmartDirectionAtCrossing(int trainIdx);

// ----------------------------------------------------------------------------
// TRAIN MOVEMENT
// ----------------------------------------------------------------------------
// Move trains and handle collisions (Phase 5).
void moveAllTrains();

// ----------------------------------------------------------------------------
// COLLISION DETECTION
// ----------------------------------------------------------------------------
// Detect trains targeting the same tile/swap/crossing and apply Priority.
void detectCollisions();

// ----------------------------------------------------------------------------
// ARRIVALS
// ----------------------------------------------------------------------------
// Mark trains that reached destinations.
void checkArrivals();

// ----------------------------------------------------------------------------
// EMERGENCY HALT
// ----------------------------------------------------------------------------
// Apply emergency halt in active zone.
void applyEmergencyHalt();

// Update emergency halt timer.
void updateEmergencyHalt();

#endif
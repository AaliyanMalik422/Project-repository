#ifndef SWITCHES_H
#define SWITCHES_H

// ============================================================================
// SWITCHES.H - Switch logic (Updated signatures)
// ============================================================================

// ----------------------------------------------------------------------------
// SWITCH COUNTER UPDATE
// ----------------------------------------------------------------------------
// Increment/Decrement counters when trains enter switches.
void updateSwitchCounters();

// ----------------------------------------------------------------------------
// FLIP QUEUE
// ----------------------------------------------------------------------------
// Queue flips when counters reach 0.
void queueSwitchFlips();

// ----------------------------------------------------------------------------
// DEFERRED FLIP
// ----------------------------------------------------------------------------
// Apply queued flips after movement.
void applyDeferredFlips();

// ----------------------------------------------------------------------------
// SIGNAL CALCULATION
// ----------------------------------------------------------------------------
// Update switch signal colors.
void updateSignalLights();

// ----------------------------------------------------------------------------
// SWITCH TOGGLE
// ----------------------------------------------------------------------------
// Manually toggle a switch state.
void toggleSwitchState(int switchIndex);

// ----------------------------------------------------------------------------
// HELPER FUNCTIONS
// ----------------------------------------------------------------------------
// Get state for a given direction.
int getSwitchStateForDirection(int switchIndex);

#endif
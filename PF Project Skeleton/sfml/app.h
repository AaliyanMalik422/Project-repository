#ifndef APP_H
#define APP_H

bool initializeApp();
void runApp();
void cleanupApp();

// Simulation functions
void initializeSimulation();
void simulateOneTick();
bool isSimulationComplete();
void logTrainTrace();
void logSwitchState();

// Train functions
void spawnTrains();
void moveTrains();
void checkDestinations();
void checkCollisions();
void updateTrainCounters();
void checkSafetyViolations();

// Switch helper functions
bool isSwitchTile(int x, int y);
int getSwitchIndex(int x, int y);
void toggleSwitch(int switchIndex);
void initializeSwitches();
#endif


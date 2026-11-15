#ifndef IO_H
#define IO_H

#include <string>
#include "simulation_state.h"

extern int train_wait[MAX_TRAINS];
extern int train_priority[MAX_TRAINS];

bool loadLevelFile(std::string filepath);
void initializeLogFiles();
void logTrainTrace();
void logSwitchState();
void writeMetrics();

#endif

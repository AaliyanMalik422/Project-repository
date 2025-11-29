#ifndef IO_H
#define IO_H

#include <string>

// Load a .lvl file from path
bool loadLevelFile(std::string filepath);

// Create/clear log files
void initializeLogFiles();

// Append train movement to trace.csv
void logTrainTrace();

// Append switch state to switches.csv
void logSwitchState();

// Write final metrics to metrics.txt
void writeMetrics();

#endif

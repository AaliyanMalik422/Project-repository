#ifndef IO_H
#define IO_H

#include <string>

// Loads a .lvl level file and populates global arrays.
// Returns true on success.
bool loadLevelFile(std::string filepath);

// Initializes all CSV/TXT log files (trace.csv, switches.csv, metrics.txt)
void initializeLogFiles();

// Logs train movement to trace.csv (APPEND MODE)
void logTrainTrace();

// Logs switch state changes to switches.csv (APPEND MODE)
void logSwitchState();

// Writes summary metrics (total trains, delivered trains)
void writeMetrics();

#endif

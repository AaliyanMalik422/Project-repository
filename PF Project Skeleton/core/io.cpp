#include<iostream>
#include "io.h"
#include "simulation_state.h"
#include "grid.h"
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

using namespace std;

// ============================================================================
// IO.CPP - Level I/O and logging
// ============================================================================

// ----------------------------------------------------------------------------
// LOAD LEVEL FILE
// ----------------------------------------------------------------------------
// Load a .lvl file into global state.
// ----------------------------------------------------------------------------
bool loadLevelFile(string filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cout << "Error: Could not open level file " << filepath << endl;
        return false;
    }
    // 1. Reset Global State
    state.trainCount = 0;
    state.switchCount = 0;
    state.currentTick = 0;
    string line;
    string section = "HEADER";
    int mapRow = 0;
    // 2. Parse the File Line-by-Line
    while (file >> line) {
        // --- SECTION DETECTION ---
        if (line == "MAP:") {
            section = "MAP";
            continue;
        } else if (line == "SWITCHES:") {
            section = "SWITCHES";
            continue;
        } else if (line == "TRAINS:") {
            section = "TRAINS";
            continue;
        }

        // --- DATA PARSING ---
        if (section == "HEADER") {
            if (line == "ROWS:") file >> state.rows;
            else if (line == "COLS:") file >> state.cols;
        }
        else if (section == "MAP") {
            // "line" contains the map row string (e.g. S===A)
            if (mapRow < state.rows) {
                for (int c = 0; c < line.length() && c < 30; c++) {
                    state.grid[mapRow][c] = line[c];
                }
                mapRow++;
            }
        }
        else if (section == "SWITCHES") {
            // Safety: Don't exceed array limit
            if (state.switchCount >= 20) continue; 

            Switch* s = &state.switches[state.switchCount];
            s->type = line[0]; // The 'A', 'B' etc.
            
            string modeStr;
            file >> modeStr >> s->state; // Read "PER_DIR" and Init State (0/1)
            
            // Read 4 counters
            for(int i=0; i<4; i++) file >> s->kValues[i];

            // Find X/Y on grid
            for(int r=0; r<state.rows; r++) {
                for(int c=0; c<state.cols; c++) {
                    if(state.grid[r][c] == s->type) {
                        s->x = c;
                        s->y = r;
                    }
                }
            }
            string dummy; getline(file, dummy); // Skip rest of line
            state.switchCount++;
        }
        else if (section == "TRAINS") {
            // Safety: Don't exceed array limit
            if (state.trainCount >= 50) continue;

            Train* t = &state.trains[state.trainCount];
            try {
                t->spawnTick = stoi(line);
                file >> t->x >> t->y >> t->direction >> t->color;
                
                // Initialize default values
                t->id = state.trainCount;
                t->active = false;
                t->finished = false;
                
                state.trainCount++;
            } catch (...) { }
        }
    }
    file.close();
    cout << "Level loaded: " << filepath << " (" << state.rows << "x" << state.cols << ")" << endl;
    return true;
}


// ----------------------------------------------------------------------------
// INITIALIZE LOG FILES
// ----------------------------------------------------------------------------
// Create/clear CSV logs with headers.
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    ofstream trace("out/trace.csv");
    trace << "Tick,TrainID,X,Y,Direction,State" << endl;
    trace.close();

    ofstream switches("out/switches.csv");
    switches << "Tick,Switch,Mode,State" << endl;
    switches.close();
    
    ofstream signals("out/signals.csv");
    signals << "Tick,Switch,Signal" << endl;
    signals.close();
    
    ofstream metrics("out/metrics.txt");
    metrics.close();
}

// ----------------------------------------------------------------------------
// LOG TRAIN TRACE
// ----------------------------------------------------------------------------
// Append tick, train id, position, direction, state to trace.csv.
// ----------------------------------------------------------------------------
void logTrainTrace() {
    ofstream file("out/trace.csv", ios::app); // Open in append mode
    for(int i = 0; i < state.trainCount; i++) {
        if(state.trains[i].active) {
            // State 0 = Moving (Default)
            file << state.currentTick << "," 
                 << state.trains[i].id << "," 
                 << state.trains[i].x << "," 
                 << state.trains[i].y << "," 
                 << state.trains[i].direction << ",0" << endl;
        }
    }
}

// ----------------------------------------------------------------------------
// LOG SWITCH STATE
// ----------------------------------------------------------------------------
// Append tick, switch id/mode/state to switches.csv.
// ----------------------------------------------------------------------------
void logSwitchState() {
    ofstream file("out/switches.csv", ios::app);
    for(int i = 0; i < state.switchCount; i++) {
        file << state.currentTick << "," 
             << state.switches[i].type << ","
             << "GLOBAL," // Simplified mode for logging
             << state.switches[i].state << endl;
    }
}

// ----------------------------------------------------------------------------
// LOG SIGNAL STATE
// ----------------------------------------------------------------------------
// Append tick, switch id, signal color to signals.csv.
// ----------------------------------------------------------------------------
void logSignalState() {

    // Placeholder: Implement if you handle traffic lights later

}

// ----------------------------------------------------------------------------
// WRITE FINAL METRICS
// ----------------------------------------------------------------------------
// Write summary metrics to metrics.txt.
// ----------------------------------------------------------------------------
void writeMetrics() {
    ofstream file("out/metrics.txt");
    int delivered = 0;
    
    for(int i = 0; i < state.trainCount; i++) {
        if(state.trains[i].finished) delivered++;
    }

    file << "Simulation Report" << endl;
    file << "Total Trains: " << state.trainCount << endl;
    file << "Delivered: " << delivered << endl;
}

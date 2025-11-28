#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "simulation_state.h"
#include "io.h"

using namespace std;

// ============================================================================
// IO.CPP - Level I/O and logging (Parallel Arrays Version)
// ============================================================================

// ----------------------------------------------------------------------------
// LOAD LEVEL FILE
// ----------------------------------------------------------------------------
bool loadLevelFile(string filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cout << "Error: Could not open level file " << filepath << endl;
        return false;
    }

    // 1. Reset Global Counters
    total_trains = 0;
    current_tick = 0;
    
    // Initialize switch active states to false
    for(int i=0; i<MAX_SWITCHES; i++) switch_active[i] = false;

    string line;
    string section = "HEADER";
    int mapRow = 0;

    // 2. Parse the File
    while (file >> line) {
        if (line == "MAP:") { section = "MAP"; continue; } 
        else if (line == "SWITCHES:") { section = "SWITCHES"; continue; } 
        else if (line == "TRAINS:") { section = "TRAINS"; continue; }

        if (section == "HEADER") {
            if (line == "ROWS:") file >> grid_rows;
            else if (line == "COLS:") file >> grid_cols;
        }
        else if (section == "MAP") {
            if (mapRow < grid_rows) {
                for (int c = 0; c < line.length() && c < MAX_COLS; c++) {
                    grid[mapRow][c] = line[c];
                }
                mapRow++;
            }
        }
        else if (section == "SWITCHES") {
            // Line format: A PER_DIR 0 5 5 5 5
            char type = line[0]; 
            int idx = type - 'A'; // Convert 'A' -> 0, 'B' -> 1
            
            if (idx >= 0 && idx < MAX_SWITCHES) {
                switch_active[idx] = true;
                string modeStr;
                file >> modeStr >> switch_state[idx]; 
                
                // Determine logic based on string (Simplified logic)
                // In your header, you have separate arrays for k-values
                for(int i=0; i<4; i++) {
                    file >> switch_k_values[idx][i];
                    switch_counters[idx][i] = switch_k_values[idx][i]; // Init counters
                }

                // Find X/Y on grid for this switch
                for(int r=0; r<grid_rows; r++) {
                    for(int c=0; c<grid_cols; c++) {
                        if(grid[r][c] == type) {
                            switch_x[idx] = c;
                            switch_y[idx] = r;
                        }
                    }
                }
            }
            string dummy; getline(file, dummy); // Skip rest
        }
        else if (section == "TRAINS") {
            if (total_trains >= MAX_TRAINS) continue;
            
            int idx = total_trains;
            try {
                // Line format: SpawnTick X Y Dir Color
                train_spawn_tick[idx] = stoi(line);
                file >> train_x[idx] >> train_y[idx] 
                     >> train_direction[idx] >> train_color[idx];
                
                // Set defaults
                train_id[idx] = idx;
                train_active[idx] = false;
                train_finished[idx] = false;
                
                total_trains++;
            } catch (...) { }
        }
    }
    file.close();
    cout << "Level loaded: " << filepath << endl;
    return true;
}

// ----------------------------------------------------------------------------
// INITIALIZE LOG FILES
// ----------------------------------------------------------------------------
void initializeLogFiles() {
    ofstream trace("out/trace.csv");
    trace << "Tick,TrainID,X,Y,Direction,State" << endl;
    trace.close();

    ofstream switches("out/switches.csv");
    switches << "Tick,Switch,State" << endl;
    switches.close();
    
    ofstream metrics("out/metrics.txt");
    metrics.close();
}

// ----------------------------------------------------------------------------
// LOG TRAIN TRACE
// ----------------------------------------------------------------------------
void logTrainTrace() {
    ofstream file("out/trace.csv", ios::app);
    for(int i = 0; i < total_trains; i++) {
        // Access global arrays directly
        if(train_active[i]) { 
            file << current_tick << "," 
                 << train_id[i] << "," 
                 << train_x[i] << "," 
                 << train_y[i] << "," 
                 << train_direction[i] << ",0" << endl;
        }
    }
}

// ----------------------------------------------------------------------------
// LOG SWITCH STATE
// ----------------------------------------------------------------------------
void logSwitchState() {
    ofstream file("out/switches.csv", ios::app);
    for(int i = 0; i < MAX_SWITCHES; i++) {
        if(switch_active[i]) {
            char name = 'A' + i; // Convert index 0 back to 'A'
            file << current_tick << "," 
                 << name << ","
                 << switch_state[i] << endl;
        }
    }
}

// ----------------------------------------------------------------------------
// WRITE METRICS
// ----------------------------------------------------------------------------
void writeMetrics() {
    ofstream file("out/metrics.txt");
    int delivered = 0;
    
    for(int i = 0; i < total_trains; i++) {
        if(train_finished[i]) delivered++;
    }

    file << "Simulation Report" << endl;
    file << "Total Trains: " << total_trains << endl;
    file << "Delivered: " << delivered << endl;
}
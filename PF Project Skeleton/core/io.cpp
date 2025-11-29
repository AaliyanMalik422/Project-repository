#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include "simulation_state.h"
#include "io.h"

using namespace std;

// ---------------------------------------------------------------------------
// Helper: trim whitespace
// ---------------------------------------------------------------------------
static string trim(const string &s) {
    int a = 0, b = (int)s.size() - 1;
    while (a <= b && isspace((unsigned char)s[a])) a++;
    while (b >= a && isspace((unsigned char)s[b])) b--;
    if (b < a) return "";
    return s.substr(a, b - a + 1);
}

// ============================================================================
// LOAD LEVEL FILE
// ============================================================================
bool loadLevelFile(string filepath) {

    ifstream file(filepath);
    if (!file.is_open()) {
        cout << "Error: Cannot open level file " << filepath << "\n";
        return false;
    }

    // Reset globals
    total_trains = 0;
    current_tick = 0;

    for (int i = 0; i < MAX_SWITCHES; i++)
        switch_active[i] = false;

    string rawLine;
    string section = "NONE";
    int mapRow = 0;

    while (getline(file, rawLine)) {

        string line = trim(rawLine);
        if (line.empty()) continue;

        // Detect section headers
        if (line == "ROWS:")       { section = "ROWS"; continue; }
        if (line == "COLS:")       { section = "COLS"; continue; }
        if (line == "SEED:")       { section = "SEED"; continue; }
        if (line == "WEATHER:")    { section = "WEATHER"; continue; }
        if (line == "MAP:")        { section = "MAP"; mapRow = 0; continue; }
        if (line == "SWITCHES:")   { section = "SWITCHES"; continue; }
        if (line == "TRAINS:")     { section = "TRAINS"; continue; }

        // --------------------------
        // ROWS
        // --------------------------
        if (section == "ROWS") {
            grid_rows = atoi(line.c_str());
            continue;
        }

        // --------------------------
        // COLS
        // --------------------------
        if (section == "COLS") {
            grid_cols = atoi(line.c_str());
            continue;
        }

        // --------------------------
        // SEED
        // --------------------------
        if (section == "SEED") {
            simulation_seed = atoi(line.c_str());
            continue;
        }

        // WEATHER ignored
        if (section == "WEATHER") continue;

        // --------------------------
        // MAP — FIXED VERSION
        // --------------------------
        if (section == "MAP") {

            if (mapRow < grid_rows) {

                string rowLine = rawLine;

                // pad if short
                if ((int)rowLine.size() < grid_cols) {
                    rowLine.append(grid_cols - rowLine.size(), ' ');
                }

                // Convert spaces → '.' so trains can move
                for (int c = 0; c < grid_cols && c < MAX_COLS; c++) {
                    char ch = (c < (int)rowLine.size() ? rowLine[c] : ' ');
                    if (ch == ' ') ch = '.';
                    grid[mapRow][c] = ch;
                }

                mapRow++;
            }
            continue;
        }

        // --------------------------
        // SWITCHES
        // --------------------------
        if (section == "SWITCHES") {
            istringstream ss(line);
            char swChar;
            if (!(ss >> swChar)) continue;
            int idx = swChar - 'A';
            if (idx < 0 || idx >= MAX_SWITCHES) continue;

            switch_active[idx] = true;

            string modeStr;
            int state = 0;
            int k1 = 0, k2 = 0, k3 = 0, k4 = 0;

            ss >> modeStr >> state >> k1 >> k2 >> k3 >> k4;

            switch_state[idx] = state;
            switch_k_values[idx][0] = k1;
            switch_k_values[idx][1] = k2;
            switch_k_values[idx][2] = k3;
            switch_k_values[idx][3] = k4;

            for (int d = 0; d < 4; d++)
                switch_counters[idx][d] = switch_k_values[idx][d];

            // find switch XY
            for (int r = 0; r < grid_rows; r++)
                for (int c = 0; c < grid_cols; c++)
                    if (grid[r][c] == swChar) {
                        switch_x[idx] = c;
                        switch_y[idx] = r;
                    }

            continue;
        }

        // --------------------------
        // TRAINS — FIXED SINGLE-LINE FORMAT
        // --------------------------
        if (section == "TRAINS") {

            if (total_trains >= MAX_TRAINS) continue;

            int spawn, x, y, dir, color;
            istringstream ss(line);

            if (!(ss >> spawn >> x >> y >> dir >> color)) continue;

            int i = total_trains;

            train_spawn_tick[i] = spawn;
            train_x[i] = x;
            train_y[i] = y;
            train_direction[i] = dir;
            train_color[i] = color;

            train_id[i] = i;
            train_active[i] = false;
            train_finished[i] = false;

            total_trains++;
            continue;
        }
    }

    file.close();

    cout << "Loaded level: " << filepath << "\n";
    cout << "Rows=" << grid_rows << " Cols=" << grid_cols << "\n";
    cout << "Trains loaded=" << total_trains << "\n";

    return true;
}

// ============================================================================
// LOG FILES
// ============================================================================
void initializeLogFiles() {
    ofstream trace("out/trace.csv");
    trace << "Tick,TrainID,X,Y,Direction,State\n";

    ofstream sw("out/switches.csv");
    sw << "Tick,Switch,State\n";

    ofstream m("out/metrics.txt");
}

// ============================================================================
// LOG TRAIN TRACE
// ============================================================================
void logTrainTrace() {
    ofstream file("out/trace.csv", ios::app);
    for (int i = 0; i < total_trains; i++) {
        if (train_active[i]) {
            file << current_tick << "," << i << ","
                 << train_x[i] << "," << train_y[i] << ","
                 << train_direction[i] << ",0\n";
        }
    }
}

// ============================================================================
// LOG SWITCH STATE
// ============================================================================
void logSwitchState() {
    ofstream file("out/switches.csv", ios::app);
    for (int s = 0; s < MAX_SWITCHES; s++) {
        if (switch_active[s]) {
            file << current_tick << "," << (char)('A' + s)
                 << "," << switch_state[s] << "\n";
        }
    }
}

// ============================================================================
// METRICS
// ============================================================================
void writeMetrics() {
    ofstream file("out/metrics.txt");
    int delivered = 0;

    for (int i = 0; i < total_trains; i++)
        if (train_finished[i]) delivered++;

    file << "Simulation Report\n";
    file << "Total trains: " << total_trains << "\n";
    file << "Delivered: " << delivered << "\n";
}

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include "simulation_state.h"
#include "io.h"

using namespace std;

static string trim(const string &s) {
    int a = 0, b = (int)s.size() - 1;
    while (a <= b && isspace((unsigned char)s[a])) a++;
    while (b >= a && isspace((unsigned char)s[b])) b--;
    if (b < a) return "";
    return s.substr(a, b - a + 1);
}

bool loadLevelFile(string filepath) {

    ifstream file(filepath);
    if (!file.is_open()) {
        cout << "Error: Cannot open level file " << filepath << "\n";
        return false;
    }

    total_trains = 0;
    train_count = 0;  // FIXED: Initialize train_count
    current_tick = 0;

    for (int i = 0; i < MAX_SWITCHES; i++)
        switch_active[i] = false;

    string rawLine;
    string section = "NONE";
    int mapRow = 0;

    while (getline(file, rawLine)) {

        string line = trim(rawLine);
        if (line.empty()) continue;

        if (line == "ROWS:")       { section = "ROWS"; continue; }
        if (line == "COLS:")       { section = "COLS"; continue; }
        if (line == "SEED:")       { section = "SEED"; continue; }
        if (line == "WEATHER:")    { section = "WEATHER"; continue; }
        if (line == "MAP:")        { section = "MAP"; mapRow = 0; continue; }
        if (line == "SWITCHES:")   { section = "SWITCHES"; continue; }
        if (line == "TRAINS:")     { section = "TRAINS"; continue; }

        if (section == "ROWS") {
            grid_rows = atoi(line.c_str());
            // FIXED: Clamp grid_rows to MAX_ROWS
            if (grid_rows > MAX_ROWS) {
                cout << "Warning: grid_rows=" << grid_rows << " exceeds MAX_ROWS=" << MAX_ROWS << ", clamping.\n";
                grid_rows = MAX_ROWS;
            }
            continue;
        }

        if (section == "COLS") {
            grid_cols = atoi(line.c_str());
            // FIXED: Clamp grid_cols to MAX_COLS
            if (grid_cols > MAX_COLS) {
                cout << "Warning: grid_cols=" << grid_cols << " exceeds MAX_COLS=" << MAX_COLS << ", clamping.\n";
                grid_cols = MAX_COLS;
            }
            continue;
        }

        if (section == "SEED") {
            simulation_seed = atoi(line.c_str());
            continue;
        }

        if (section == "WEATHER") continue;

        if (section == "MAP") {
            // FIXED: Check BOTH grid_rows AND MAX_ROWS
            if (mapRow < grid_rows && mapRow < MAX_ROWS) {

                string rowLine = rawLine;

                if ((int)rowLine.size() < grid_cols) {
                    rowLine.append(grid_cols - rowLine.size(), ' ');
                }

                // FIXED: Ensure we don't exceed MAX_COLS
                int colLimit = grid_cols;
                if (colLimit > MAX_COLS) colLimit = MAX_COLS;

                for (int c = 0; c < colLimit; c++) {
                    char ch = (c < (int)rowLine.size() ? rowLine[c] : ' ');
                    if (ch == ' ') ch = '.';
                    grid[mapRow][c] = ch;
                }

                mapRow++;
            }
            continue;
        }

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

            // FIXED: Bounds check grid access
            for (int r = 0; r < grid_rows && r < MAX_ROWS; r++)
                for (int c = 0; c < grid_cols && c < MAX_COLS; c++)
                    if (grid[r][c] == swChar) {
                        switch_x[idx] = c;
                        switch_y[idx] = r;
                    }

            continue;
        }

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

    train_count = total_trains;  // FIXED: Set train_count

    cout << "Loaded level: " << filepath << "\n";
    cout << "Rows=" << grid_rows << " Cols=" << grid_cols << "\n";
    cout << "Trains loaded=" << total_trains << "\n";

    return true;
}

void initializeLogFiles() {
    ofstream trace("out/trace.csv");
    trace << "Tick,TrainID,X,Y,Direction,State\n";
    trace.close();  // FIXED: Close file

    ofstream sw("out/switches.csv");
    sw << "Tick,Switch,State\n";
    sw.close();  // FIXED: Close file

    ofstream m("out/metrics.txt");
    m.close();  // FIXED: Close file
}

void logTrainTrace() {
    ofstream file("out/trace.csv", ios::app);
    for (int i = 0; i < total_trains; i++) {
        if (train_active[i]) {
            file << current_tick << "," << i << ","
                 << train_x[i] << "," << train_y[i] << ","
                 << train_direction[i] << ",0\n";
        }
    }
    file.close();  // FIXED: Close file
}

void logSwitchState() {
    ofstream file("out/switches.csv", ios::app);
    for (int s = 0; s < MAX_SWITCHES; s++) {
        if (switch_active[s]) {
            file << current_tick << "," << (char)('A' + s)
                 << "," << switch_state[s] << "\n";
        }
    }
    file.close();  // FIXED: Close file
}

void writeMetrics() {
    ofstream file("out/metrics.txt");
    int delivered = 0;

    for (int i = 0; i < total_trains; i++)
        if (train_finished[i]) delivered++;

    file << "Simulation Report\n";
    file << "Total trains: " << total_trains << "\n";
    file << "Delivered: " << delivered << "\n";
    file.close();  // FIXED: Close file
}
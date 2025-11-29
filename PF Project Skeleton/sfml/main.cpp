// main.cpp
#include <iostream>
#include <string>
#include <cstdlib>
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/io.h"
#include "app.h" // sfml viewer functions: initializeApp(), runApp(), cleanupApp()

using namespace std;

// Helper: print simple ASCII grid from global grid[][]
static void printAsciiGrid() {
    cout << "Tick: " << current_tick << "\n";
    for (int r = 0; r < grid_rows; ++r) {
        for (int c = 0; c < grid_cols; ++c) {
            char ch = grid[r][c];

            // If a train occupies this cell, print train id
            bool printedTrain = false;
            for (int t = 0; t < total_trains; ++t) {
                if (train_active[t] && train_x[t] == c && train_y[t] == r) {
                    // print single digit id or '*' if >9
                    if (train_id[t] >= 0 && train_id[t] < 10) cout << (char)('0' + train_id[t]);
                    else cout << '*';
                    printedTrain = true;
                    break;
                }
            }
            if (printedTrain) continue;

            // Otherwise print the tile character or '.' for empty
            if (ch == '\0') cout << '.';
            else cout << ch;
        }
        cout << "\n";
    }

    // Print trains status summary
    for (int t = 0; t < total_trains; ++t) {
        cout << "Train " << t << " id=" << train_id[t]
             << " active=" << (train_active[t] ? "Y" : "N")
             << " pos=(" << train_x[t] << "," << train_y[t] << ")"
             << " spawn=" << train_spawn_tick[t] << "\n";
    }
    cout << flush;
}

static void printUsage(const char* prog) {
    cout << "Usage: " << prog << " <level_file.lvl> [--view] [maxTicks]\n";
    cout << " Example: " << prog << " data/levels/easy_level.lvl --view 1000\n";
}

// MAIN
int main(int argc, char** argv) {
        if (argc < 2) {
        printUsage(argv[0]);
        return 0;
    }

    string levelPath = "data/levels/easy_level.lvl";
    bool viewMode = false;
    int maxTicks = -1; // -1 = no explicit max (run until complete)

    if (argc >= 2) levelPath = argv[1];
    if (argc >= 3) {
        string a2 = argv[2];
        if (a2 == "--view") viewMode = true;
        else maxTicks = atoi(argv[2]);
    }
    if (argc >= 4) {
        string a3 = argv[3];
        if (a3 == "--view") viewMode = true;
        else maxTicks = atoi(argv[3]);
    }

    cout << "Switchback Rails - starting with level: " << levelPath << endl;

    // 1) Reset core state
    initializeSimulationState();

    // 2) Load level file (populates grid, grid_rows, grid_cols, switches, trains spawn data)
    if (!loadLevelFile(levelPath)) {
        cerr << "Failed to load level: " << levelPath << endl;
        return 1;
    }

    // Optional: print quick debug of loads
    cout << "Level loaded: grid " << grid_rows << "x" << grid_cols
         << " total_trains=" << total_trains << "\n";

    for (int i = 0; i < total_trains; ++i) {
        cout << "Train " << i << " spawnTick=" << train_spawn_tick[i]
             << " pos=(" << train_x[i] << "," << train_y[i] << ") dir=" << train_direction[i]
             << " color=" << train_color[i] << "\n";
    }

    // 3) Initialize logs (trace.csv, switches.csv, metrics.txt)
    initializeLogFiles();

    // 4) Initialize simulation internals (seeding, initial signals, etc.)
    initializeSimulation();

    // 5) Run either headless simulation (recommended for grading) or GUI
    if (!viewMode) {
        cout << "Running headless simulation...\n";

        int tickCount = 0;
        // Print initial grid (tick 0)
        printAsciiGrid();

        // main headless loop
        while (true) {
            if (maxTicks >= 0 && tickCount >= maxTicks) {
                cout << "Reached maxTicks=" << maxTicks << ", stopping.\n";
                break;
            }

            // Spawn + route + counters + queue + move + arrival + logging done inside simulateOneTick()
            simulateOneTick();
            ++tickCount;

            // Terminal output each tick
            printAsciiGrid();

            // Stop if complete
            if (isSimulationComplete()) {
                cout << "Simulation complete at tick " << current_tick << "\n";
                break;
            }
        }

        // Write final metrics and close logs
        writeMetrics();             // io.cpp function
        cout << "Metrics written. Exiting.\n";
        return 0;
    }

    // If view mode: start SFML viewer (assumes initializeApp uses global state and textures)
    cout << "Starting SFML viewer...\n";
    if (!initializeApp()) {
        cerr << "Failed to initialize SFML app.\n";
        return 1;
    }

    runApp();    // SFML loop internally calls simulateOneTick() at ticks
    cleanupApp();

    // Final logs + metrics
    writeMetrics();
    cout << "Viewer closed. Metrics written. Exiting.\n";
    return 0;
}

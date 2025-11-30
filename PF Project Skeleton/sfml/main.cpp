#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/io.h"
#include "app.h" 

using namespace std;

static void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

static void sleepMs(int milliseconds) {
    clock_t start = clock();
    clock_t end = start + (milliseconds * CLOCKS_PER_SEC / 1000);
    while (clock() < end) {
        // Busy wait
    }
}

static char getTrainSymbol(int direction) {
    switch(direction) {
        case DIR_UP: return '^';
        case DIR_DOWN: return 'v';
        case DIR_LEFT: return '<';
        case DIR_RIGHT: return '>';
        default: return '*';
    }
}

static void printAsciiGrid() {
    clearScreen();
    
    cout << "========================================\n";
    cout << "Tick: " << current_tick << "\n";
    cout << "Grid: " << grid_rows << " rows x " << grid_cols << " cols\n";
    cout << "========================================\n";
    
    // Count active trains
    int activeCount = 0;
    int finishedCount = 0;
    for (int t = 0; t < total_trains; ++t) {
        if (train_active[t]) activeCount++;
        if (train_finished[t]) finishedCount++;
    }
    cout << "Active: " << activeCount << " | Finished: " << finishedCount << " / " << total_trains << "\n\n";
    
    for (int r = 0; r < grid_rows; ++r) {
        for (int c = 0; c < grid_cols; ++c) {
            char ch = grid[r][c];

            bool printedTrain = false;
            for (int t = 0; t < total_trains; ++t) {
                if (train_active[t] && train_x[t] == c && train_y[t] == r) {
                    cout << getTrainSymbol(train_direction[t]);
                    printedTrain = true;
                    break;
                }
            }
            if (printedTrain) continue;

            if (ch == '\0' || ch == ' ' || ch == '.') cout << ' ';  // ✅ Spaces instead of dots
            else cout << ch;
        }
        cout << "\n";
    }

    cout << "\n--- Train Details ---\n";
    for (int t = 0; t < total_trains; ++t) {
        cout << "Train " << t << ": ";
        if (train_finished[t]) {
            cout << "FINISHED at tick " << train_arrival_tick[t];
        } else if (train_active[t]) {
            // ✅ 1-indexed positions (add +1)
            cout << "ACTIVE pos=(" << (train_x[t] + 1) << "," << (train_y[t] + 1) << ")"
                 << " dir=" << getTrainSymbol(train_direction[t])
                 << " dest=(" << (train_dest_x[t] + 1) << "," << (train_dest_y[t] + 1) << ")";
        } else {
            cout << "INACTIVE (spawns at tick " << train_spawn_tick[t] << ")";
        }
        cout << "\n";
    }
    
    cout << flush;
}

static void printUsage(const char* prog) {
    cout << "Usage: " << prog << " <level_file.lvl> [--view] [maxTicks]\n";
    cout << " Example: " << prog << " data/levels/easy_level.lvl --view 1000\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 0;
    }

    string levelPath = "data/levels/easy_level.lvl";
    bool viewMode = false;
    int maxTicks = -1; 

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

    initializeSimulationState();

    if (!loadLevelFile(levelPath)) {
        cerr << "Failed to load level: " << levelPath << endl;
        return 1;
    }

    cout << "Level loaded: grid " << grid_rows << "x" << grid_cols
         << " total_trains=" << total_trains << "\n";

    for (int i = 0; i < total_trains; ++i) {
        cout << "Train " << i << " spawnTick=" << train_spawn_tick[i]
             << " pos=(" << train_x[i] << "," << train_y[i] << ") dir=" << train_direction[i]
             << " dest=(" << train_dest_x[i] << "," << train_dest_y[i] << ")"
             << " color=" << train_color[i] << "\n";
    }

    // ✅ Initialize output files BEFORE simulation starts
    initializeLogFiles();

    initializeSimulation();

    if (!viewMode) {
        cout << "Running headless simulation...\n";
        cout << "Press Ctrl+C to stop...\n\n";
        sleepMs(2000);

        int tickCount = 0;
        printAsciiGrid();

        while (true) {
            if (maxTicks >= 0 && tickCount >= maxTicks) {
                cout << "Reached maxTicks=" << maxTicks << ", stopping.\n";
                break;
            }

            sleepMs(500);

            simulateOneTick();
            ++tickCount;

            // ✅ Log every tick's data
            logTrainTrace();
            logSwitchState();

            printAsciiGrid();

            if (isSimulationComplete()) {
                cout << "\n*** SIMULATION COMPLETE at tick " << current_tick << " ***\n";
                break;
            }
        }

        // ✅ Write final metrics
        writeMetrics();             
        cout << "Metrics written to out/ directory. Exiting.\n";
        return 0;
    }

    cout << "Starting SFML viewer...\n";
    if (!initializeApp()) {
        cerr << "Failed to initialize SFML app.\n";
        return 1;
    }

    runApp();    
    cleanupApp();

    // ✅ Write metrics after viewer closes
    writeMetrics();
    cout << "Viewer closed. Metrics written to out/ directory. Exiting.\n";
    return 0;
}
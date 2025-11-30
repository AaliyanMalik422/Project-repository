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

static void printAsciiGrid() {
    clearScreen();
    
    cout << "Tick: " << current_tick << "\n";
    for (int r = 0; r < grid_rows; ++r) {
        for (int c = 0; c < grid_cols; ++c) {
            char ch = grid[r][c];

            bool printedTrain = false;
            for (int t = 0; t < total_trains; ++t) {
                if (train_active[t] && train_x[t] == c && train_y[t] == r) {
                    if (train_id[t] >= 0 && train_id[t] < 10) cout << (char)('0' + train_id[t]);
                    else cout << '*';
                    printedTrain = true;
                    break;
                }
            }
            if (printedTrain) continue;

            if (ch == '\0' || ch == ' ' || ch == '.') cout << '.';
            else cout << ch;
        }
        cout << "\n";
    }

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
             << " color=" << train_color[i] << "\n";
    }

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

            sleepMs(500);  // 500ms delay between ticks

            simulateOneTick();
            ++tickCount;

            printAsciiGrid();

            if (isSimulationComplete()) {
                cout << "Simulation complete at tick " << current_tick << "\n";
                break;
            }
        }

        writeMetrics();             
        cout << "Metrics written. Exiting.\n";
        return 0;
    }

    cout << "Starting SFML viewer...\n";
    if (!initializeApp()) {
        cerr << "Failed to initialize SFML app.\n";
        return 1;
    }

    runApp();    
    cleanupApp();

    writeMetrics();
    cout << "Viewer closed. Metrics written. Exiting.\n";
    return 0;
}
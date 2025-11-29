#include "app.h"
#include <iostream>

using namespace std;

// ============================================================================
// MAIN.CPP - Entry point of the application
// ============================================================================

int main() {
    cout << "Initializing Switchback Rails..." << endl;

    if (!initializeApp()) {
        cerr << "Failed to initialize application!" << endl;
        return 1;
    }

    cout << "Initialization complete." << endl;
    cout << "Controls:" << endl;
    cout << "  ESC       - Exit" << endl;
    cout << "  SPACE     - Pause / Resume" << endl;
    cout << "  . (Period)- Step 1 tick while paused" << endl;
    cout << "  Drag Left Mouse - Move Camera" << endl;
    cout << "  Left Click on Tile - Toggle Track Safety" << endl;

    runApp();
    cleanupApp();

    cout << "Application closed." << endl;
    return 0;
}

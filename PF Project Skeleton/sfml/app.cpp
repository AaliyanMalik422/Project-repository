#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/grid.h"
#include "../core/io.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

// ============================================================================
// APP.CPP - Visuals (PARALLEL ARRAY VERSION)
// ============================================================================

// Texture index constants (simple ints, no enums)
const int TEX_HORZ         = 0;
const int TEX_VERT         = 1;
const int TEX_DIAG_UP      = 2;
const int TEX_DIAG_DOWN    = 3;
const int TEX_INTERSECTION = 4;
const int TEX_SWITCH       = 5;
const int TEX_SOURCE       = 6;
const int TEX_DESTINATION  = 7;
const int TEX_SAFETY       = 8;
const int TEX_TRAIN        = 9;

static sf::RenderWindow* g_window = nullptr;
static sf::View g_camera;
static bool g_isPaused = true;
static bool g_isStepMode = false;
static bool g_isDragging = false;
static int g_lastMouseX = 0, g_lastMouseY = 0;
static float g_cellSize = 32.0f;
static float g_gridOffsetX = 50.0f, g_gridOffsetY = 50.0f;
static sf::Texture g_textures[10];
static bool g_texturesLoaded = false;

// ----------------------------------------------------------------------------
// initializeApp
// ----------------------------------------------------------------------------
bool initializeApp() {
    // Reset global simulation state
    initializeSimulationState();

    // Load a level (change file name if you want another level)
    if (!loadLevelFile("data/levels/easy_level.lvl")) {
        cerr << "Failed to load level!" << endl;
        return false;
    }

    // Create window & camera
    g_window = new sf::RenderWindow(sf::VideoMode(1024, 768), "Switchback Rails");
    g_window->setFramerateLimit(60);
    g_camera.setSize(1024, 768);
    g_camera.setCenter(512, 384);

    // Load textures in the order of the constants above
    const char* files[] = {
        "Sprites/track_horizontal.png",   // TEX_HORZ
        "Sprites/track_vertical.png",     // TEX_VERT
        "Sprites/track_diagonal_up.png",      // TEX_DIAG_UP
        "Sprites/track_diagonal_down.png",    // TEX_DIAG_DOWN
        "Sprites/track_intersection.png", // TEX_INTERSECTION
        "Sprites/switch.png",             // TEX_SWITCH
        "Sprites/source.png",             // TEX_SOURCE
        "Sprites/destination.png",        // TEX_DESTINATION
        "Sprites/safety.png",             // TEX_SAFETY
        "Sprites/train.png"               // TEX_TRAIN
    };

    bool ok = true;
    for (int i = 0; i < 10; ++i) {
        if (!g_textures[i].loadFromFile(files[i])) {
            cerr << "Warning: failed to load texture: " << files[i] << endl;
            ok = false;
        }
    }
    g_texturesLoaded = ok;

    // Initialize simulation internals after level load
    initializeSimulation();

    return true;
}

// ----------------------------------------------------------------------------
// runApp - main loop: input -> simulation -> render
// ----------------------------------------------------------------------------
void runApp() {
    if (!g_window) return;

    sf::Clock clock;
    float timeSinceLastTick = 0.0f;

    while (g_window->isOpen()) {
        sf::Event event;
        while (g_window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                g_window->close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) g_window->close();
                if (event.key.code == sf::Keyboard::Space) g_isPaused = !g_isPaused;
                if (event.key.code == sf::Keyboard::Period) g_isStepMode = true;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                g_isDragging = true;
                g_lastMouseX = event.mouseButton.x;
                g_lastMouseY = event.mouseButton.y;

                sf::Vector2f wPos = g_window->mapPixelToCoords(sf::Mouse::getPosition(*g_window), g_camera);
                int gx = static_cast<int>((wPos.x - g_gridOffsetX) / g_cellSize);
                int gy = static_cast<int>((wPos.y - g_gridOffsetY) / g_cellSize);
                toggleSafetyTile(gx, gy);
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                g_isDragging = false;
            }
            if (event.type == sf::Event::MouseMoved && g_isDragging) {
                // Move camera by the mouse delta
                g_camera.move(static_cast<float>(g_lastMouseX - event.mouseMove.x),
                              static_cast<float>(g_lastMouseY - event.mouseMove.y));
                g_lastMouseX = event.mouseMove.x;
                g_lastMouseY = event.mouseMove.y;
            }
        }

        float dt = clock.restart().asSeconds();
        timeSinceLastTick += dt;

        // Fixed tick every 0.5 seconds (as originally used)
        if (timeSinceLastTick >= 0.5f) {
            if (!g_isPaused || g_isStepMode) {
                simulateOneTick();
                g_isStepMode = false;
            }
            timeSinceLastTick = 0.0f;
        }

        // Render
        g_window->clear(sf::Color(40, 44, 52));
        g_window->setView(g_camera);

        if (g_texturesLoaded) {
            sf::Sprite sprite;

            // 1) Draw Grid
            for (int r = 0; r < grid_rows; ++r) {
                for (int c = 0; c < grid_cols; ++c) {
                    char tile = grid[r][c];
                    int texID = -1;

                    if (tile == '-')          texID = TEX_HORZ;
                    else if (tile == '|')     texID = TEX_VERT;
                    else if (tile == '/')     texID = TEX_DIAG_UP;
                    else if (tile == '\\')    texID = TEX_DIAG_DOWN;
                    else if (tile == '+')     texID = TEX_INTERSECTION;
                    else if (tile >= 'A' && tile <= 'Z') texID = TEX_SWITCH;
                    else if (tile == 'S')     texID = TEX_SOURCE;
                    else if (tile == 'D')     texID = TEX_DESTINATION;
                    else if (tile == '=')     texID = TEX_SAFETY; // safety tile uses '=' in your grid

                    if (texID != -1) {
                        sprite.setTexture(g_textures[texID]);
                        sprite.setPosition(g_gridOffsetX + c * g_cellSize,
                                           g_gridOffsetY + r * g_cellSize);
                        g_window->draw(sprite);
                    }
                }
            }

            // 2) Draw Trains
            sprite.setTexture(g_textures[TEX_TRAIN]);
            for (int i = 0; i < total_trains; ++i) {
                if (train_active[i]) {
                    sprite.setPosition(g_gridOffsetX + train_x[i] * g_cellSize,
                                       g_gridOffsetY + train_y[i] * g_cellSize);
                    g_window->draw(sprite);
                }
            }
        } else {
            // If textures didn't fully load, at least draw a placeholder grid using rectangles
            sf::RectangleShape box(sf::Vector2f(g_cellSize - 1.0f, g_cellSize - 1.0f));
            for (int r = 0; r < grid_rows; ++r) {
                for (int c = 0; c < grid_cols; ++c) {
                    char tile = grid[r][c];
                    if (tile == '.' ) continue;
                    box.setPosition(g_gridOffsetX + c * g_cellSize, g_gridOffsetY + r * g_cellSize);
                    box.setFillColor(sf::Color(120, 120, 120));
                    g_window->draw(box);
                }
            }
        }

        g_window->display();
    }
}

// ----------------------------------------------------------------------------
// cleanupApp
// ----------------------------------------------------------------------------
void cleanupApp() {
    if (g_window) {
        delete g_window;
        g_window = nullptr;
    }
}
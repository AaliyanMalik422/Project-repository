#include "app.h"
#include "../core/simulation_state.h"
#include "../core/simulation.h"
#include "../core/grid.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;

// ============================================================================
// APP.CPP - Visuals (PARALLEL ARRAY VERSION)
// ============================================================================

static sf::RenderWindow* g_window = nullptr;
static sf::View g_camera;
static bool g_isPaused = true; 
static bool g_isStepMode = false;
static bool g_isDragging = false;
static int g_lastMouseX = 0, g_lastMouseY = 0;
static float g_cellSize = 32.0f;
static float g_gridOffsetX = 50.0f, g_gridOffsetY = 50.0f;
static sf::Texture g_textures[7]; 
static bool g_texturesLoaded = false;

bool initializeApp() {
    g_window = new sf::RenderWindow(sf::VideoMode(1024, 768), "Switchback Rails");
    g_window->setFramerateLimit(60);
    g_camera.setSize(1024, 768);
    g_camera.setCenter(512, 384);

    const char* files[] = {
        "Sprites/5.png", "Sprites/5.png", "Sprites/3.png",
        "Sprites/4.png", "Sprites/4.png", "Sprites/4.png", "Sprites/2.png"
    };

    bool allGood = true;
    for (int i = 0; i < 7; i++) {
        if (!g_textures[i].loadFromFile(files[i])) allGood = false;
    }
    g_texturesLoaded = allGood;
    return true;
}

void runApp() {
    sf::Clock clock;
    float timeSinceLastTick = 0.0f;

    while (g_window->isOpen()) {
        sf::Event event;
        while (g_window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) g_window->close();
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
                int gx = (wPos.x - g_gridOffsetX) / g_cellSize;
                int gy = (wPos.y - g_gridOffsetY) / g_cellSize;
                toggleSafetyTile(gx, gy);
            }
            if (event.type == sf::Event::MouseButtonReleased) g_isDragging = false;
            if (event.type == sf::Event::MouseMoved && g_isDragging) {
                g_camera.move(g_lastMouseX - event.mouseMove.x, g_lastMouseY - event.mouseMove.y);
                g_lastMouseX = event.mouseMove.x;
                g_lastMouseY = event.mouseMove.y;
            }
        }

        float dt = clock.restart().asSeconds();
        timeSinceLastTick += dt;

        if (timeSinceLastTick >= 0.5f) {
            if (!g_isPaused || g_isStepMode) {
                // updateSimulation(); // UNCOMMENT WHEN MEMBER B FINISHES
                g_isStepMode = false;
                timeSinceLastTick = 0.0f;
            }
        }

        g_window->clear(sf::Color(40, 44, 52));
        g_window->setView(g_camera);

        if (g_texturesLoaded) {
            sf::Sprite sprite;
            // 1. Draw Grid
            for (int r = 0; r < grid_rows; r++) {
                for (int c = 0; c < grid_cols; c++) {
                    char tile = grid[r][c];
                    int texID = -1;
                    if (tile == '-') texID = 0;
                    else if (tile == '|') texID = 1;
                    else if (tile == '/' || tile == '\\') texID = 2;
                    else if (tile == '+') texID = 3;
                    else if (tile >= 'A' && tile <= 'Z') texID = 4;
                    else if (tile == 'S' || tile == 'D') texID = 5;

                    if (texID != -1) {
                        sprite.setTexture(g_textures[texID]);
                        sprite.setPosition(g_gridOffsetX + c * g_cellSize, g_gridOffsetY + r * g_cellSize);
                        g_window->draw(sprite);
                    }
                }
            }
            // 2. Draw Trains
            sprite.setTexture(g_textures[6]);
            for (int i = 0; i < total_trains; i++) {
                if (train_active[i]) {
                    sprite.setPosition(g_gridOffsetX + train_x[i] * g_cellSize, 
                                       g_gridOffsetY + train_y[i] * g_cellSize);
                    g_window->draw(sprite);
                }
            }
        }
        g_window->display();
    }
}

void cleanupApp() { if (g_window) { delete g_window; g_window = nullptr; } }
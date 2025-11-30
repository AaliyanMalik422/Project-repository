#include "app.h"
#include "../core/simulation_state.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

using namespace std;

// SFML objects
sf::RenderWindow window;
sf::Font font;

// Textures for different track types
sf::Texture trackStraightTexture;
sf::Texture trackCurveTexture;
sf::Texture trackCrossTexture;
sf::Texture switchATexture;
sf::Texture switchBTexture;
sf::Texture sourceTexture;
sf::Texture destTexture;

// Train textures (4 directions)
sf::Texture trainTextures[4];

// Camera/view
sf::View camera;
float cameraX = 0.f, cameraY = 0.f;
float zoomLevel = 1.0f;

// Tile rendering size
const float TILE_SIZE = 64.f;

// Pause state
bool isPaused = false;

// Helper to load a texture
static bool loadTex(sf::Texture &tex, const string &path) {
    if (!tex.loadFromFile(path)) {
        cerr << "Warning: failed to load texture: " << path << "\n";
        return false;
    }
    return true;
}

bool initializeApp() {
    // Create window
    window.create(sf::VideoMode(1280, 720), "Switchback Rails Viewer");
    window.setFramerateLimit(60);

    // Load font (optional)
    if (!font.loadFromFile("assets/fonts/Arial.ttf")) {
        cerr << "Warning: could not load font. Text may not display.\n";
    }

    // Load track textures
    loadTex(trackStraightTexture, "Sprites/track_straight.png");
    loadTex(trackCurveTexture, "Sprites/track_curve.png");
    loadTex(trackCrossTexture, "Sprites/track_cross.png");
    loadTex(switchATexture, "Sprites/switch_A.png");
    loadTex(switchBTexture, "Sprites/switch_B.png");
    loadTex(sourceTexture, "Sprites/source.png");
    loadTex(destTexture, "Sprites/destination.png");

    // Load train textures (4 directions: UP=0, DOWN=1, LEFT=2, RIGHT=3)
    loadTex(trainTextures[0], "Sprites/train_up.png");
    loadTex(trainTextures[1], "Sprites/train_down.png");
    loadTex(trainTextures[2], "Sprites/train_left.png");
    loadTex(trainTextures[3], "Sprites/train_right.png");

    // Initialize camera centered on grid
    float gridPixelWidth = grid_cols * TILE_SIZE * 0.5f;
    float gridPixelHeight = grid_rows * TILE_SIZE * 0.5f;
    camera = window.getDefaultView();
    camera.setCenter(gridPixelWidth / 2.f, gridPixelHeight / 2.f);
    window.setView(camera);

    return true;
}

void runApp() {
    sf::Clock clock;
    float timeAccumulator = 0.f;
    const float TICK_INTERVAL = 0.5f; // 0.5 seconds per tick

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                if (event.key.code == sf::Keyboard::Space) {
                    isPaused = !isPaused;
                    cout << (isPaused ? "PAUSED" : "RESUMED") << "\n";
                }
                // Manual step with '.' key
                if (event.key.code == sf::Keyboard::Period) {
                    simulateOneTick();
                    logTrainTrace();
                    logSwitchState();
                    cout << "Manual step: tick " << current_tick << "\n";
                }
            }
        }

        // Auto-tick if not paused
        if (!isPaused) {
            float dt = clock.restart().asSeconds();
            timeAccumulator += dt;
            if (timeAccumulator >= TICK_INTERVAL) {
                timeAccumulator = 0.f;
                simulateOneTick();
                logTrainTrace();
                logSwitchState();

                if (isSimulationComplete()) {
                    cout << "\n*** SIMULATION COMPLETE at tick " << current_tick << " ***\n";
                    isPaused = true;
                }
            }
        } else {
            clock.restart();
        }

        // Clear window
        window.clear(sf::Color(30, 30, 30));

        // Draw grid tiles
        sf::Sprite tileSprite;
        for (int row = 0; row < grid_rows; ++row) {
            for (int col = 0; col < grid_cols; ++col) {
                char tile = grid[row][col];
                float x = col * TILE_SIZE * 0.5f;  // Adjust for scale
                float y = row * TILE_SIZE * 0.5f;  // Adjust for scale

                // Choose texture based on tile type
                if (tile == '-' || tile == '|') {
                    tileSprite.setTexture(trackStraightTexture);
                    tileSprite.setRotation(0.f);
                    tileSprite.setPosition(x, y);
                    if (tile == '|') {
                        tileSprite.setRotation(90.f);
                    }
                } else if (tile == '+') {
                    tileSprite.setTexture(trackCrossTexture);
                    tileSprite.setRotation(0.f);
                    tileSprite.setPosition(x, y);
                } else if (tile == '/') {
                    tileSprite.setTexture(trackCurveTexture);
                    tileSprite.setRotation(0.f);
                    tileSprite.setPosition(x, y);
                } else if (tile == '\\') {
                    tileSprite.setTexture(trackCurveTexture);
                    tileSprite.setRotation(90.f);
                    tileSprite.setPosition(x, y);
                } else if (tile == 'A') {
                    tileSprite.setTexture(switchATexture);
                    tileSprite.setRotation(0.f);
                    tileSprite.setPosition(x, y);
                } else if (tile == 'B') {
                    tileSprite.setTexture(switchBTexture);
                    tileSprite.setRotation(0.f);
                    tileSprite.setPosition(x, y);
                } else if (tile == 'S') {
                    tileSprite.setTexture(sourceTexture);
                    tileSprite.setRotation(0.f);
                    tileSprite.setPosition(x, y);
                } else if (tile == 'D') {
                    tileSprite.setTexture(destTexture);
                    tileSprite.setRotation(0.f);
                    tileSprite.setPosition(x, y);
                } else {
                    continue;
                }

                tileSprite.setScale(0.5f, 0.5f);
                window.draw(tileSprite);
            }
        }

        // Draw trains
        sf::Sprite trainSprite;
        for (int i = 0; i < MAX_TRAINS; ++i) {
            if (!train_active[i]) continue;

            int tx = train_x[i];
            int ty = train_y[i];
            int dir = train_direction[i];

            float screenX = tx * TILE_SIZE * 0.5f;  // Adjust for scale
            float screenY = ty * TILE_SIZE * 0.5f;  // Adjust for scale

            trainSprite.setTexture(trainTextures[dir]);
            trainSprite.setPosition(screenX, screenY);
            trainSprite.setScale(0.4f, 0.4f);
            window.draw(trainSprite);
        }

        // Draw UI text
        sf::Text infoText;
        infoText.setFont(font);
        infoText.setCharacterSize(18);
        infoText.setFillColor(sf::Color::White);
        infoText.setString("Tick: " + to_string(current_tick) + 
                          (isPaused ? " [PAUSED]" : "") +
                          "\nPress SPACE to pause/resume\nPress . to step");
        infoText.setPosition(10, 10);
        
        // Draw text in screen coordinates (not world)
        window.setView(window.getDefaultView());
        window.draw(infoText);
        window.setView(camera);

        window.display();
    }
}

void cleanupApp() {
    // Nothing specific to clean up for now
}
#include "app.h"
#include "../core/simulation_state.h"
#include "../core/grid.h"
#include "../core/switches.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

using namespace std;

sf::RenderWindow window;
sf::Font font;

sf::Texture trackStraightTexture;
sf::Texture trackCurveDownTexture;
sf::Texture trackCurveUpTexture;
sf::Texture trackCrossTexture;
sf::Texture switchATexture;
sf::Texture switchBTexture;
sf::Texture sourceTexture;
sf::Texture destTexture;
sf::Texture safetyTexture;
sf::Texture trainTextures[4];

sf::View camera;
float zoomLevel = 1.0f;
const float TILE_SIZE = 64.f;
const float WORLD_TILE_SIZE = TILE_SIZE * 0.5f;
bool isPaused = false;
static bool fontLoaded = false;
static bool dragging = false;
static sf::Vector2i lastMousePosition;

static bool loadTex(sf::Texture &tex, const string &path) {
    if (!tex.loadFromFile(path)) {
        cerr << "Warning: failed to load texture: " << path << "\n";
        return false;
    }
    return true;
}

bool initializeApp() {
    window.create(sf::VideoMode(1280, 720), "Switchback Rails Viewer");
    if (!window.isOpen()) return false;
    window.setFramerateLimit(60);

    fontLoaded = font.loadFromFile("assets/fonts/Arial.ttf");
    if (!fontLoaded) {
        cerr << "Warning: assets/fonts/Arial.ttf not found; text overlay disabled.\n";
    }

    loadTex(trackStraightTexture, "Sprites/track_horizontal.png");
    loadTex(trackCurveDownTexture, "Sprites/track_diagonal_down.png");
    loadTex(trackCurveUpTexture, "Sprites/track_diagonal_up.png");
    loadTex(trackCrossTexture, "Sprites/track_cross.png");
    loadTex(switchATexture, "Sprites/switch_A.png");
    loadTex(switchBTexture, "Sprites/switch_B.png");
    loadTex(sourceTexture, "Sprites/tile_source_blue.png");
    loadTex(destTexture, "Sprites/tile_destination.png");
    loadTex(safetyTexture, "Sprites/tile_safety.png");

    loadTex(trainTextures[DIR_UP], "Sprites/train_up.png");
    loadTex(trainTextures[DIR_RIGHT], "Sprites/train_right.png");
    loadTex(trainTextures[DIR_DOWN], "Sprites/train_down.png");
    loadTex(trainTextures[DIR_LEFT], "Sprites/train_left.png");

    camera = window.getDefaultView();
    camera.setCenter(grid_cols * WORLD_TILE_SIZE / 2.f,
                     grid_rows * WORLD_TILE_SIZE / 2.f);
    window.setView(camera);
    return true;
}

static void handleMouseClick(const sf::Event::MouseButtonEvent &mouseEvent) {
    const sf::Vector2f world = window.mapPixelToCoords(
        sf::Vector2i(mouseEvent.x, mouseEvent.y), camera);
    const int col = static_cast<int>(floor(world.x / WORLD_TILE_SIZE));
    const int row = static_cast<int>(floor(world.y / WORLD_TILE_SIZE));

    if (!isInBounds(col, row)) return;

    if (mouseEvent.button == sf::Mouse::Left) {
        if (toggleSafetyTile(col, row)) {
            cout << "Safety tile updated at (" << col << ", " << row << ")\n";
        }
    } else if (mouseEvent.button == sf::Mouse::Right) {
        if (isSwitchTile(col, row)) {
            const int switchIndex = getSwitchIndex(col, row);
            if (switchIndex >= 0) {
                toggleSwitch(switchIndex);
                cout << "Switch " << grid[row][col] << " toggled.\n";
            }
        }
    }
}

void runApp() {
    sf::Clock clock;
    float timeAccumulator = 0.f;
    const float TICK_INTERVAL = 0.5f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                } else if (event.key.code == sf::Keyboard::Space) {
                    isPaused = !isPaused;
                } else if (event.key.code == sf::Keyboard::Period) {
                    simulateOneTick();
                    logTrainTrace();
                    logSwitchState();
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    dragging = true;
                    lastMousePosition = sf::Mouse::getPosition(window);
                } else if (event.mouseButton.button == sf::Mouse::Left ||
                           event.mouseButton.button == sf::Mouse::Right) {
                    handleMouseClick(event.mouseButton);
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Middle) {
                    dragging = false;
                }
            } else if (event.type == sf::Event::MouseMoved && dragging) {
                const sf::Vector2i current = sf::Mouse::getPosition(window);
                const sf::Vector2i delta = current - lastMousePosition;
                camera.move(-static_cast<float>(delta.x) * zoomLevel,
                            -static_cast<float>(delta.y) * zoomLevel);
                lastMousePosition = current;
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0.f) {
                    camera.zoom(0.9f);
                    zoomLevel *= 0.9f;
                } else {
                    camera.zoom(1.1f);
                    zoomLevel *= 1.1f;
                }
            }
        }

        if (!isPaused) {
            timeAccumulator += clock.restart().asSeconds();
            if (timeAccumulator >= TICK_INTERVAL) {
                timeAccumulator -= TICK_INTERVAL;
                simulateOneTick();
                logTrainTrace();
                logSwitchState();

                if (isSimulationComplete()) {
                    isPaused = true;
                    cout << "Simulation complete at tick " << current_tick << "\n";
                }
            }
        } else {
            clock.restart();
        }

        window.clear(sf::Color(30, 30, 30));
        window.setView(camera);

        sf::Sprite tileSprite;
        for (int row = 0; row < grid_rows; ++row) {
            for (int col = 0; col < grid_cols; ++col) {
                const char tile = grid[row][col];
                const float x = col * WORLD_TILE_SIZE;
                const float y = row * WORLD_TILE_SIZE;
                bool drawTile = true;

                if (tile == '-' || tile == '|' || tile == '=') {
                    tileSprite.setTexture(tile == '=' ? safetyTexture : trackStraightTexture);
                    tileSprite.setRotation(tile == '|' ? 90.f : 0.f);
                } else if (tile == '+') {
                    tileSprite.setTexture(trackCrossTexture);
                    tileSprite.setRotation(0.f);
                } else if (tile == '/') {
                    tileSprite.setTexture(trackCurveUpTexture);
                    tileSprite.setRotation(0.f);
                } else if (tile == '\\') {
                    tileSprite.setTexture(trackCurveDownTexture);
                    tileSprite.setRotation(0.f);
                } else if (isSwitchTile(col, row)) {
                    const int switchIndex = getSwitchIndex(col, row);
                    tileSprite.setTexture(
                        switchIndex >= 0 && switch_state[switchIndex] == 1
                            ? switchBTexture : switchATexture);
                    tileSprite.setRotation(0.f);
                } else if (tile == 'S') {
                    tileSprite.setTexture(sourceTexture);
                    tileSprite.setRotation(0.f);
                } else if (tile == 'D') {
                    tileSprite.setTexture(destTexture);
                    tileSprite.setRotation(0.f);
                } else {
                    drawTile = false;
                }

                if (drawTile) {
                    tileSprite.setPosition(x, y);
                    tileSprite.setScale(0.5f, 0.5f);
                    window.draw(tileSprite);
                }
            }
        }

        sf::Sprite trainSprite;
        for (int i = 0; i < total_trains; ++i) {
            if (!train_active[i]) continue;
            const int direction = train_direction[i];
            if (direction < DIR_UP || direction > DIR_LEFT) continue;

            trainSprite.setTexture(trainTextures[direction]);
            trainSprite.setPosition(train_x[i] * WORLD_TILE_SIZE,
                                    train_y[i] * WORLD_TILE_SIZE);
            trainSprite.setScale(0.4f, 0.4f);
            window.draw(trainSprite);
        }

        if (fontLoaded) {
            sf::Text infoText;
            infoText.setFont(font);
            infoText.setCharacterSize(18);
            infoText.setFillColor(sf::Color::White);
            infoText.setString("Tick: " + to_string(current_tick) +
                               (isPaused ? " [PAUSED]" : "") +
                               "\nSPACE: pause/resume  .: step\nLeft click: safety  Right click: switch\nMiddle drag: pan  Wheel: zoom");
            infoText.setPosition(10.f, 10.f);
            window.setView(window.getDefaultView());
            window.draw(infoText);
        }

        window.display();
    }
}

void cleanupApp() {
    if (window.isOpen()) window.close();
}

#include "app.h"
#include "../core/simulation_state.h"
#include "../core/grid.h"
#include "../core/switches.h"
#include "../core/simulation.h"
#include "../core/io.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <optional>
#include <string>

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
const float WORLD_TILE_SIZE = 32.f;
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
    window.create(sf::VideoMode({1280u, 720u}), "Switchback Rails Viewer");
    if (!window.isOpen()) return false;
    window.setFramerateLimit(60);

    fontLoaded = font.openFromFile("assets/fonts/Arial.ttf");
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
    camera.setCenter({grid_cols * WORLD_TILE_SIZE / 2.f,
                      grid_rows * WORLD_TILE_SIZE / 2.f});
    window.setView(camera);
    return true;
}

static void handleMouseClick(sf::Vector2i pixel, sf::Mouse::Button button) {
    const sf::Vector2f world = window.mapPixelToCoords(pixel, camera);
    const int col = static_cast<int>(floor(world.x / WORLD_TILE_SIZE));
    const int row = static_cast<int>(floor(world.y / WORLD_TILE_SIZE));

    if (!isInBounds(col, row)) return;

    if (button == sf::Mouse::Button::Left) {
        if (toggleSafetyTile(col, row)) {
            cout << "Safety tile updated at (" << col << ", " << row << ")\n";
        }
    } else if (button == sf::Mouse::Button::Right) {
        if (isSwitchTile(col, row)) {
            const int switchIndex = getSwitchIndex(col, row);
            if (switchIndex >= 0) {
                toggleSwitch(switchIndex);
                cout << "Switch " << grid[row][col] << " toggled.\n";
            }
        }
    }
}

static void drawTile(const sf::Texture &texture, float x, float y,
                     float rotation = 0.f) {
    sf::Sprite sprite(texture);
    sprite.setPosition({x, y});
    sprite.setScale({0.5f, 0.5f});
    sprite.setRotation(sf::degrees(rotation));
    window.draw(sprite);
}

void runApp() {
    sf::Clock clock;
    float timeAccumulator = 0.f;
    const float TICK_INTERVAL = 0.5f;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                } else if (key->code == sf::Keyboard::Key::Space) {
                    isPaused = !isPaused;
                } else if (key->code == sf::Keyboard::Key::Period) {
                    if (!isSimulationComplete()) {
                        simulateOneTick();
                        logTrainTrace();
                        logSwitchState();
                    }
                }
            } else if (const auto *pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (pressed->button == sf::Mouse::Button::Middle) {
                    dragging = true;
                    lastMousePosition = sf::Mouse::getPosition(window);
                } else if (pressed->button == sf::Mouse::Button::Left ||
                           pressed->button == sf::Mouse::Button::Right) {
                    handleMouseClick(pressed->position, pressed->button);
                }
            } else if (const auto *released = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (released->button == sf::Mouse::Button::Middle) dragging = false;
            } else if (const auto *moved = event->getIf<sf::Event::MouseMoved>()) {
                if (dragging) {
                    const sf::Vector2i current(moved->position);
                    const sf::Vector2i delta = current - lastMousePosition;
                    camera.move({-static_cast<float>(delta.x) * zoomLevel,
                                 -static_cast<float>(delta.y) * zoomLevel});
                    lastMousePosition = current;
                }
            } else if (const auto *wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (wheel->delta > 0.f) {
                    camera.zoom(0.9f);
                    zoomLevel *= 0.9f;
                } else {
                    camera.zoom(1.1f);
                    zoomLevel *= 1.1f;
                }
            }
        }

        const float elapsed = clock.restart().asSeconds();
        if (!isPaused && !isSimulationComplete()) {
            timeAccumulator += elapsed;
            while (timeAccumulator >= TICK_INTERVAL) {
                timeAccumulator -= TICK_INTERVAL;
                simulateOneTick();
                logTrainTrace();
                logSwitchState();
                if (isSimulationComplete()) {
                    isPaused = true;
                    cout << "Simulation complete at tick " << current_tick << "\n";
                    break;
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.setView(camera);

        for (int row = 0; row < grid_rows; ++row) {
            for (int col = 0; col < grid_cols; ++col) {
                const char tile = grid[row][col];
                const float x = col * WORLD_TILE_SIZE;
                const float y = row * WORLD_TILE_SIZE;

                if (tile == '-' || tile == '|' || tile == '=') {
                    drawTile(tile == '=' ? safetyTexture : trackStraightTexture,
                             x, y, tile == '|' ? 90.f : 0.f);
                } else if (tile == '+') {
                    drawTile(trackCrossTexture, x, y);
                } else if (tile == '/') {
                    drawTile(trackCurveUpTexture, x, y);
                } else if (tile == '\\') {
                    drawTile(trackCurveDownTexture, x, y);
                } else if (isSwitchTile(col, row)) {
                    const int index = getSwitchIndex(col, row);
                    if (index >= 0 && switch_state[index] == 1)
                        drawTile(switchBTexture, x, y);
                    else
                        drawTile(switchATexture, x, y);
                } else if (tile == 'S') {
                    drawTile(sourceTexture, x, y);
                } else if (tile == 'D') {
                    drawTile(destTexture, x, y);
                }
            }
        }

        for (int i = 0; i < total_trains; ++i) {
            if (!train_active[i]) continue;
            const int direction = train_direction[i];
            if (direction < DIR_UP || direction > DIR_LEFT) continue;
            sf::Sprite trainSprite(trainTextures[direction]);
            trainSprite.setPosition({train_x[i] * WORLD_TILE_SIZE,
                                     train_y[i] * WORLD_TILE_SIZE});
            trainSprite.setScale({0.4f, 0.4f});
            window.draw(trainSprite);
        }

        if (fontLoaded) {
            sf::Text infoText(font,
                "Tick: " + to_string(current_tick) +
                (isPaused ? " [PAUSED]" : "") +
                "\nSPACE: pause/resume  .: step\nLeft click: safety  Right click: switch\nMiddle drag: pan  Wheel: zoom",
                18);
            infoText.setFillColor(sf::Color::White);
            infoText.setPosition({10.f, 10.f});
            window.setView(window.getDefaultView());
            window.draw(infoText);
        }

        window.display();
    }
}

void cleanupApp() {
    if (window.isOpen()) window.close();
}

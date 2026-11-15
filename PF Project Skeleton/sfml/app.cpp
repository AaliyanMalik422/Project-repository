#include "app.h"
#include "../core/simulation_state.h"
#include "../core/grid.h"
#include "../core/switches.h"
#include "../core/simulation.h"
#include "../core/io.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <optional>
#include <string>

using namespace std;

sf::RenderWindow window;
sf::Font font;
sf::View camera;

const float WORLD_TILE_SIZE = 48.f;
bool isPaused = false;

static bool fontLoaded = false;
static bool dragging = false;
static sf::Vector2i lastMousePosition{0, 0};
static float zoomLevel = 1.f;

static int boardMinX = 0;
static int boardMinY = 0;
static int boardMaxX = 0;
static int boardMaxY = 0;

static const sf::Color BACKGROUND(15, 22, 31);
static const sf::Color BOARD_DARK(29, 40, 51);
static const sf::Color BOARD_LIGHT(32, 44, 56);
static const sf::Color GRID_LINE(43, 57, 69);
static const sf::Color TRACK_BALLAST(43, 49, 56);
static const sf::Color TRACK_SLEEPER(112, 75, 48);
static const sf::Color TRACK_RAIL(190, 202, 210);
static const sf::Color TRACK_RAIL_DARK(117, 133, 145);
static const sf::Color SAFETY_RAIL(87, 211, 157);
static const sf::Color SOURCE_BLUE(40, 135, 218);
static const sf::Color DEST_GREEN(40, 169, 119);
static const sf::Color SWITCH_GOLD(242, 181, 72);

static bool isVisibleTile(char tile) {
    return tile != ' ' && tile != '\t' && tile != '\0';
}

static void findBoardBounds() {
    boardMinX = grid_cols;
    boardMinY = grid_rows;
    boardMaxX = -1;
    boardMaxY = -1;

    for (int row = 0; row < grid_rows; ++row) {
        for (int col = 0; col < grid_cols; ++col) {
            if (!isVisibleTile(grid[row][col])) continue;
            boardMinX = min(boardMinX, col);
            boardMinY = min(boardMinY, row);
            boardMaxX = max(boardMaxX, col);
            boardMaxY = max(boardMaxY, row);
        }
    }

    if (boardMaxX < boardMinX || boardMaxY < boardMinY) {
        boardMinX = boardMinY = 0;
        boardMaxX = max(0, grid_cols - 1);
        boardMaxY = max(0, grid_rows - 1);
    }
}

static void frameBoard() {
    findBoardBounds();

    const sf::Vector2u windowSize = window.getSize();
    const float aspect = windowSize.y == 0
        ? 16.f / 9.f
        : static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);

    const float boardWidth = (boardMaxX - boardMinX + 1) * WORLD_TILE_SIZE;
    const float boardHeight = (boardMaxY - boardMinY + 1) * WORLD_TILE_SIZE;
    const float margin = WORLD_TILE_SIZE * 1.15f;

    // Fit the actual map, not the often much larger ROWS/COLS allocation.
    const float viewWidth = max(
        (boardWidth + margin * 2.f) / 0.84f,
        (boardHeight + margin * 2.f) * aspect / 0.84f
    );

    camera = window.getDefaultView();
    camera.setSize({viewWidth, viewWidth / aspect});
    camera.setCenter({
        (boardMinX + boardMaxX + 1) * WORLD_TILE_SIZE * 0.5f,
        (boardMinY + boardMaxY + 1) * WORLD_TILE_SIZE * 0.5f
    });
    window.setView(camera);
    zoomLevel = 1.f;
}

static void drawCenteredBar(float centerX, float centerY,
                            float length, float thickness,
                            float rotation, const sf::Color &color) {
    sf::RectangleShape bar({length, thickness});
    bar.setOrigin({length * 0.5f, thickness * 0.5f});
    bar.setPosition({centerX, centerY});
    bar.setRotation(sf::degrees(rotation));
    bar.setFillColor(color);
    window.draw(bar);
}

static sf::Vector2f pointAlongTrack(float centerX, float centerY,
                                    float angleRadians, float along,
                                    float across = 0.f) {
    const float dx = std::cos(angleRadians);
    const float dy = std::sin(angleRadians);
    return {
        centerX + dx * along - dy * across,
        centerY + dy * along + dx * across
    };
}

static void drawTrackLine(float centerX, float centerY, float angleDegrees,
                          bool safety = false) {
    const float angle = angleDegrees * 3.14159265358979323846f / 180.f;
    const float lineLength = WORLD_TILE_SIZE + 2.f;

    // A dark bed keeps adjoining pieces visually connected.
    drawCenteredBar(centerX, centerY, lineLength, 15.f,
                     angleDegrees, TRACK_BALLAST);

    // Wooden sleepers sit perpendicular to the rails.
    for (float along = -18.f; along <= 18.f; along += 12.f) {
        const sf::Vector2f p = pointAlongTrack(
            centerX, centerY, angle, along
        );
        drawCenteredBar(p.x, p.y, 25.f, 4.5f,
                        angleDegrees + 90.f, TRACK_SLEEPER);
    }

    const sf::Color railColor = safety ? SAFETY_RAIL : TRACK_RAIL;
    const float railOffset = 6.2f;
    for (float side : {-railOffset, railOffset}) {
        const sf::Vector2f p = pointAlongTrack(
            centerX, centerY, angle, 0.f, side
        );
        drawCenteredBar(p.x, p.y, lineLength, 2.8f,
                        angleDegrees, railColor);
    }

    if (safety) {
        sf::CircleShape marker(3.3f);
        marker.setOrigin({3.3f, 3.3f});
        marker.setPosition({centerX, centerY});
        marker.setFillColor(SAFETY_RAIL);
        window.draw(marker);
    }
}

static void drawCellBackground(int col, int row) {
    const float x = col * WORLD_TILE_SIZE;
    const float y = row * WORLD_TILE_SIZE;

    sf::RectangleShape cell({WORLD_TILE_SIZE - 1.f, WORLD_TILE_SIZE - 1.f});
    cell.setPosition({x + 0.5f, y + 0.5f});
    cell.setFillColor(((col + row) % 2 == 0) ? BOARD_DARK : BOARD_LIGHT);
    cell.setOutlineThickness(0.5f);
    cell.setOutlineColor(GRID_LINE);
    window.draw(cell);
}

static void drawSource(float centerX, float centerY) {
    sf::CircleShape outer(15.f);
    outer.setOrigin({15.f, 15.f});
    outer.setPosition({centerX, centerY});
    outer.setFillColor(SOURCE_BLUE);
    outer.setOutlineThickness(2.f);
    outer.setOutlineColor(sf::Color(190, 225, 250));
    window.draw(outer);

    sf::CircleShape inner(5.f);
    inner.setOrigin({5.f, 5.f});
    inner.setPosition({centerX, centerY});
    inner.setFillColor(sf::Color::White);
    window.draw(inner);
}

static void drawDestination(float centerX, float centerY) {
    sf::CircleShape outer(15.f);
    outer.setOrigin({15.f, 15.f});
    outer.setPosition({centerX, centerY});
    outer.setFillColor(DEST_GREEN);
    outer.setOutlineThickness(2.f);
    outer.setOutlineColor(sf::Color(190, 245, 220));
    window.draw(outer);

    sf::CircleShape ring(8.f);
    ring.setOrigin({8.f, 8.f});
    ring.setPosition({centerX, centerY});
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineThickness(2.f);
    ring.setOutlineColor(sf::Color::White);
    window.draw(ring);

    sf::CircleShape center(3.f);
    center.setOrigin({3.f, 3.f});
    center.setPosition({centerX, centerY});
    center.setFillColor(sf::Color::White);
    window.draw(center);
}

static bool hasTrackNeighbor(int col, int row) {
    return isInBounds(col, row) && isTrackTile(col, row);
}

static void drawSwitch(int col, int row, float centerX, float centerY) {
    const bool horizontal =
        hasTrackNeighbor(col - 1, row) || hasTrackNeighbor(col + 1, row);
    const bool vertical =
        hasTrackNeighbor(col, row - 1) || hasTrackNeighbor(col, row + 1);

    if (horizontal || !vertical)
        drawTrackLine(centerX, centerY, 0.f);
    if (vertical)
        drawTrackLine(centerX, centerY, 90.f);

    const int index = getSwitchIndex(col, row);
    const bool active = index >= 0 && switch_state[index] == 1;

    sf::CircleShape indicator(6.5f);
    indicator.setOrigin({6.5f, 6.5f});
    indicator.setPosition({centerX, centerY});
    indicator.setFillColor(active ? SWITCH_GOLD : sf::Color(83, 177, 205));
    indicator.setOutlineThickness(2.f);
    indicator.setOutlineColor(sf::Color(20, 29, 37));
    window.draw(indicator);
}

static void drawTrain(int x, int y, int direction) {
    const float centerX = (x + 0.5f) * WORLD_TILE_SIZE;
    const float centerY = (y + 0.5f) * WORLD_TILE_SIZE;
    const bool vertical = direction == DIR_UP || direction == DIR_DOWN;
    const float width = vertical ? 15.f : 25.f;
    const float height = vertical ? 25.f : 15.f;

    sf::RectangleShape body({width, height});
    body.setOrigin({width * 0.5f, height * 0.5f});
    body.setPosition({centerX, centerY});
    body.setFillColor(sf::Color(48, 105, 170));
    body.setOutlineThickness(2.f);
    body.setOutlineColor(sf::Color(220, 235, 248));
    window.draw(body);

    float noseX = 0.f;
    float noseY = 0.f;
    if (direction == DIR_UP) noseY = -9.f;
    else if (direction == DIR_RIGHT) noseX = 9.f;
    else if (direction == DIR_DOWN) noseY = 9.f;
    else if (direction == DIR_LEFT) noseX = -9.f;

    sf::CircleShape nose(3.5f);
    nose.setOrigin({3.5f, 3.5f});
    nose.setPosition({centerX + noseX, centerY + noseY});
    nose.setFillColor(sf::Color(255, 224, 135));
    window.draw(nose);
}

static void handleMouseClick(sf::Vector2i pixel, sf::Mouse::Button button) {
    const sf::Vector2f world = window.mapPixelToCoords(pixel, camera);
    const int col = static_cast<int>(std::floor(world.x / WORLD_TILE_SIZE));
    const int row = static_cast<int>(std::floor(world.y / WORLD_TILE_SIZE));

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

static void loadViewerFont() {
    const string candidates[] = {
        "assets/fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };

    for (const string &path : candidates) {
        if (font.openFromFile(path)) {
            fontLoaded = true;
            return;
        }
    }

    cerr << "Warning: no viewer font found; HUD text disabled.\n";
}

bool initializeApp() {
    window.create(sf::VideoMode({1280u, 800u}), "Switchback Rails Viewer");
    if (!window.isOpen()) return false;
    window.setFramerateLimit(60);

    loadViewerFont();
    frameBoard();
    return true;
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
                } else if (key->code == sf::Keyboard::Key::Home) {
                    frameBoard();
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
                    const sf::Vector2u size = window.getSize();
                    const float worldPerPixelX = camera.getSize().x /
                        static_cast<float>(max(1u, size.x));
                    const float worldPerPixelY = camera.getSize().y /
                        static_cast<float>(max(1u, size.y));
                    camera.move({
                        -static_cast<float>(delta.x) * worldPerPixelX,
                        -static_cast<float>(delta.y) * worldPerPixelY
                    });
                    lastMousePosition = current;
                }
            } else if (const auto *wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                const float factor = wheel->delta > 0.f ? 0.88f : 1.14f;
                if (zoomLevel * factor >= 0.35f && zoomLevel * factor <= 4.f) {
                    camera.zoom(factor);
                    zoomLevel *= factor;
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

        window.clear(BACKGROUND);
        window.setView(camera);

        // Draw a clean, continuous board over the occupied map bounds.
        for (int row = boardMinY; row <= boardMaxY; ++row) {
            for (int col = boardMinX; col <= boardMaxX; ++col) {
                drawCellBackground(col, row);
            }
        }

        for (int row = 0; row < grid_rows; ++row) {
            for (int col = 0; col < grid_cols; ++col) {
                const char tile = grid[row][col];
                if (!isVisibleTile(tile)) continue;

                const float centerX = (col + 0.5f) * WORLD_TILE_SIZE;
                const float centerY = (row + 0.5f) * WORLD_TILE_SIZE;

                if (tile == '-') {
                    drawTrackLine(centerX, centerY, 0.f);
                } else if (tile == '=') {
                    drawTrackLine(centerX, centerY, 0.f, true);
                } else if (tile == '|') {
                    drawTrackLine(centerX, centerY, 90.f);
                } else if (tile == '+') {
                    drawTrackLine(centerX, centerY, 0.f);
                    drawTrackLine(centerX, centerY, 90.f);
                } else if (tile == '/') {
                    drawTrackLine(centerX, centerY, -45.f);
                } else if (tile == '\\') {
                    drawTrackLine(centerX, centerY, 45.f);
                } else if (isSwitchTile(col, row)) {
                    drawSwitch(col, row, centerX, centerY);
                } else if (tile == 'S') {
                    drawSource(centerX, centerY);
                } else if (tile == 'D') {
                    drawDestination(centerX, centerY);
                }
            }
        }

        for (int i = 0; i < total_trains; ++i) {
            if (!train_active[i]) continue;
            const int direction = train_direction[i];
            if (direction < DIR_UP || direction > DIR_LEFT) continue;
            drawTrain(train_x[i], train_y[i], direction);
        }

        // Keep the HUD in screen space so it remains readable while zooming.
        if (fontLoaded) {
            window.setView(window.getDefaultView());

            sf::RectangleShape panel({610.f, 72.f});
            panel.setPosition({16.f, 16.f});
            panel.setFillColor(sf::Color(9, 15, 22, 218));
            panel.setOutlineThickness(1.f);
            panel.setOutlineColor(sf::Color(71, 91, 108, 220));
            window.draw(panel);

            sf::Text title(font, "SWITCHBACK RAILS", 15);
            title.setPosition({30.f, 25.f});
            title.setFillColor(sf::Color(225, 236, 244));
            window.draw(title);

            const string status =
                "Tick " + to_string(current_tick) +
                (isPaused ? "  |  PAUSED" : "  |  RUNNING") +
                "     SPACE pause  . step  HOME reset view";
            sf::Text controls(font, status, 12);
            controls.setPosition({30.f, 50.f});
            controls.setFillColor(sf::Color(165, 185, 199));
            window.draw(controls);
        }

        window.display();
    }
}

void cleanupApp() {
    if (window.isOpen()) window.close();
}

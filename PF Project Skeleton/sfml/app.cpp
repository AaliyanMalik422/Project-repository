#include "app.h"
#include "../core/simulation_state.h"
#include "../core/grid.h"
#include "../core/switches.h"
#include "../core/simulation.h"
#include "../core/io.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace std;

sf::RenderWindow window;
sf::Font font;
sf::View camera;

static constexpr float WORLD_TILE_SIZE = 48.f;
static constexpr float PI = 3.14159265358979323846f;
bool isPaused = false;

static bool fontLoaded = false;
static bool dragging = false;
static sf::Vector2i lastMousePosition{0, 0};
static float zoomLevel = 1.f;

static int boardMinX = 0;
static int boardMinY = 0;
static int boardMaxX = 0;
static int boardMaxY = 0;

static sf::Texture trackStraightTexture;
static sf::Texture trackVerticalTexture;
static sf::Texture trackCurveUpTexture;
static sf::Texture trackCurveDownTexture;
static sf::Texture trackCrossTexture;
static sf::Texture switchATexture;
static sf::Texture switchBTexture;
static sf::Texture sourceTexture;
static sf::Texture destinationTexture;
static sf::Texture safetyTexture;
static sf::Texture trainTextures[4];

static const sf::Color BACKGROUND(28, 32, 38);
static const sf::Color PAPER(250, 250, 247);
static const sf::Color PAPER_GRID(222, 226, 228);
static const sf::Color PAPER_BORDER(190, 196, 201);
static const sf::Color HUD_BACKGROUND(18, 24, 31, 225);
static const sf::Color HUD_BORDER(91, 108, 123, 210);

struct CropRect {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
};

static bool isVisibleTile(char tile) {
    return tile != ' ' && tile != '\t' && tile != '\0' && tile != '.';
}

static bool isPaperBackground(const sf::Color &pixel) {
    if (pixel.a == 0) return true;

    const unsigned int minimum = min({pixel.r, pixel.g, pixel.b});
    const unsigned int maximum = max({pixel.r, pixel.g, pixel.b});
    return minimum >= 190 && (maximum - minimum) <= 48;
}

static bool loadCroppedSprite(sf::Texture &texture,
                              const string &path,
                              CropRect crop) {
    sf::Image source;
    if (!source.loadFromFile(path)) {
        cerr << "Warning: failed to load sprite: " << path << "\n";
        return false;
    }

    const sf::Vector2u sourceSize = source.getSize();
    if (crop.x >= sourceSize.x || crop.y >= sourceSize.y) {
        cerr << "Warning: invalid crop for sprite: " << path << "\n";
        return false;
    }

    crop.width = min(crop.width, sourceSize.x - crop.x);
    crop.height = min(crop.height, sourceSize.y - crop.y);
    if (crop.width == 0 || crop.height == 0) return false;

    sf::Image image({crop.width, crop.height}, sf::Color::Transparent);
    for (unsigned int y = 0; y < crop.height; ++y) {
        for (unsigned int x = 0; x < crop.width; ++x) {
            image.setPixel(
                {x, y},
                source.getPixel({crop.x + x, crop.y + y})
            );
        }
    }

    const size_t pixelCount =
        static_cast<size_t>(crop.width) * static_cast<size_t>(crop.height);
    vector<uint8_t> visited(pixelCount, 0);
    vector<size_t> queue;
    queue.reserve(pixelCount / 2);

    auto enqueueBackground = [&](unsigned int x, unsigned int y) {
        const size_t index =
            static_cast<size_t>(y) * crop.width + static_cast<size_t>(x);
        if (visited[index]) return;
        visited[index] = 1;

        const sf::Color pixel = image.getPixel({x, y});
        if (isPaperBackground(pixel)) {
            image.setPixel({x, y}, sf::Color::Transparent);
            queue.push_back(index);
        }
    };

    for (unsigned int x = 0; x < crop.width; ++x) {
        enqueueBackground(x, 0);
        enqueueBackground(x, crop.height - 1);
    }
    for (unsigned int y = 0; y < crop.height; ++y) {
        enqueueBackground(0, y);
        enqueueBackground(crop.width - 1, y);
    }

    size_t head = 0;
    while (head < queue.size()) {
        const size_t index = queue[head++];
        const unsigned int x = static_cast<unsigned int>(index % crop.width);
        const unsigned int y = static_cast<unsigned int>(index / crop.width);

        if (x > 0) enqueueBackground(x - 1, y);
        if (x + 1 < crop.width) enqueueBackground(x + 1, y);
        if (y > 0) enqueueBackground(x, y - 1);
        if (y + 1 < crop.height) enqueueBackground(x, y + 1);
    }

    if (!texture.loadFromImage(image)) {
        cerr << "Warning: failed to create sprite texture: " << path << "\n";
        return false;
    }
    texture.setSmooth(true);
    return true;
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

static bool loadAllSprites() {
    bool ok = true;

    ok &= loadCroppedSprite(trackStraightTexture, "Sprites/track_horizontal.png",
                            {12, 18, 304, 103});
    ok &= loadCroppedSprite(trackVerticalTexture, "Sprites/track_vertical.png",
                            {9, 12, 132, 222});
    ok &= loadCroppedSprite(trackCurveUpTexture, "Sprites/track_diagonal_up.png",
                            {8, 8, 264, 265});
    ok &= loadCroppedSprite(trackCurveDownTexture, "Sprites/track_diagonal_down.png",
                            {8, 8, 266, 260});
    ok &= loadCroppedSprite(trackCrossTexture, "Sprites/track_cross.png",
                            {9, 16, 241, 278});

    ok &= loadCroppedSprite(switchATexture, "Sprites/switch_A.png",
                            {34, 10, 302, 298});
    ok &= loadCroppedSprite(switchBTexture, "Sprites/switch_B.png",
                            {30, 10, 322, 306});

    ok &= loadCroppedSprite(sourceTexture, "Sprites/tile_source_blue.png",
                            {10, 32, 165, 165});
    ok &= loadCroppedSprite(destinationTexture, "Sprites/tile_destination.png",
                            {26, 31, 164, 164});
    ok &= loadCroppedSprite(safetyTexture, "Sprites/tile_safety.png",
                            {20, 8, 160, 160});

    ok &= loadCroppedSprite(trainTextures[DIR_UP], "Sprites/train_up.png",
                            {37, 53, 339, 229});
    ok &= loadCroppedSprite(trainTextures[DIR_RIGHT], "Sprites/train_right.png",
                            {28, 51, 321, 228});
    ok &= loadCroppedSprite(trainTextures[DIR_DOWN], "Sprites/train_down.png",
                            {14, 44, 324, 229});
    ok &= loadCroppedSprite(trainTextures[DIR_LEFT], "Sprites/train_left.png",
                            {42, 44, 320, 230});

    return ok;
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
        : static_cast<float>(windowSize.x) /
          static_cast<float>(windowSize.y);

    const float margin = WORLD_TILE_SIZE * 0.8f;
    const float boardWidth =
        (boardMaxX - boardMinX + 1) * WORLD_TILE_SIZE + margin * 2.f;
    const float boardHeight =
        (boardMaxY - boardMinY + 1) * WORLD_TILE_SIZE + margin * 2.f;

    const float viewWidth = max(boardWidth, boardHeight * aspect) / 0.88f;

    camera = window.getDefaultView();
    camera.setSize({viewWidth, viewWidth / aspect});
    camera.setCenter({
        (boardMinX + boardMaxX + 1) * WORLD_TILE_SIZE * 0.5f,
        (boardMinY + boardMaxY + 1) * WORLD_TILE_SIZE * 0.5f
    });

    window.setView(camera);
    zoomLevel = 1.f;
}

static void drawPaperBoard() {
    const float left = (boardMinX - 0.5f) * WORLD_TILE_SIZE;
    const float top = (boardMinY - 0.5f) * WORLD_TILE_SIZE;
    const float width = (boardMaxX - boardMinX + 2.f) * WORLD_TILE_SIZE;
    const float height = (boardMaxY - boardMinY + 2.f) * WORLD_TILE_SIZE;

    sf::RectangleShape paper({width, height});
    paper.setPosition({left, top});
    paper.setFillColor(PAPER);
    paper.setOutlineThickness(1.5f);
    paper.setOutlineColor(PAPER_BORDER);
    window.draw(paper);

    sf::VertexArray gridLines(sf::PrimitiveType::Lines);
    for (int col = boardMinX; col <= boardMaxX + 1; ++col) {
        const float x = col * WORLD_TILE_SIZE;
        gridLines.append(sf::Vertex({x, top}, PAPER_GRID));
        gridLines.append(sf::Vertex({x, top + height}, PAPER_GRID));
    }
    for (int row = boardMinY; row <= boardMaxY + 1; ++row) {
        const float y = row * WORLD_TILE_SIZE;
        gridLines.append(sf::Vertex({left, y}, PAPER_GRID));
        gridLines.append(sf::Vertex({left + width, y}, PAPER_GRID));
    }
    window.draw(gridLines);
}

static void drawSpriteFit(const sf::Texture &texture,
                          float centerX, float centerY,
                          float maxWidth, float maxHeight) {
    const sf::Vector2u size = texture.getSize();
    if (size.x == 0 || size.y == 0) return;

    const float scale = min(maxWidth / static_cast<float>(size.x),
                            maxHeight / static_cast<float>(size.y));
    sf::Sprite sprite(texture);
    sprite.setOrigin({
        static_cast<float>(size.x) * 0.5f,
        static_cast<float>(size.y) * 0.5f
    });
    sprite.setPosition({centerX, centerY});
    sprite.setScale({scale, scale});
    window.draw(sprite);
}

static void drawSpriteFill(const sf::Texture &texture,
                           float centerX, float centerY,
                           float width, float height) {
    const sf::Vector2u size = texture.getSize();
    if (size.x == 0 || size.y == 0) return;

    sf::Sprite sprite(texture);
    sprite.setOrigin({
        static_cast<float>(size.x) * 0.5f,
        static_cast<float>(size.y) * 0.5f
    });
    sprite.setPosition({centerX, centerY});
    sprite.setScale({
        width / static_cast<float>(size.x),
        height / static_cast<float>(size.y)
    });
    window.draw(sprite);
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

bool initializeApp() {
    window.create(sf::VideoMode({1280u, 800u}), "Switchback Rails Viewer");
    if (!window.isOpen()) return false;
    window.setFramerateLimit(60);

    loadViewerFont();
    if (!loadAllSprites()) {
        cerr << "One or more viewer sprites could not be loaded.\n";
    }

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
                if (released->button == sf::Mouse::Button::Middle) {
                    dragging = false;
                }
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
            } else if (event->is<sf::Event::Resized>()) {
                const sf::Vector2u size = window.getSize();
                const float aspect = static_cast<float>(size.x) /
                    static_cast<float>(max(1u, size.y));
                const sf::Vector2f oldSize = camera.getSize();
                const float newWidth = max(oldSize.x, oldSize.y * aspect);
                camera.setSize({newWidth, newWidth / aspect});
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
        drawPaperBoard();

        for (int row = 0; row < grid_rows; ++row) {
            for (int col = 0; col < grid_cols; ++col) {
                const char tile = grid[row][col];
                if (!isVisibleTile(tile)) continue;

                const float centerX = (col + 0.5f) * WORLD_TILE_SIZE;
                const float centerY = (row + 0.5f) * WORLD_TILE_SIZE;

                if (tile == '-') {
                    drawSpriteFit(trackStraightTexture, centerX, centerY,
                                  WORLD_TILE_SIZE, WORLD_TILE_SIZE * 0.76f);
                } else if (tile == '=') {
                    drawSpriteFill(safetyTexture, centerX, centerY,
                                   WORLD_TILE_SIZE * 0.94f,
                                   WORLD_TILE_SIZE * 0.94f);
                } else if (tile == '|') {
                    drawSpriteFit(trackVerticalTexture, centerX, centerY,
                                  WORLD_TILE_SIZE * 0.76f, WORLD_TILE_SIZE);
                } else if (tile == '+') {
                    drawSpriteFill(trackCrossTexture, centerX, centerY,
                                   WORLD_TILE_SIZE * 0.98f,
                                   WORLD_TILE_SIZE * 0.98f);
                } else if (tile == '/') {
                    drawSpriteFill(trackCurveUpTexture, centerX, centerY,
                                   WORLD_TILE_SIZE * 0.98f,
                                   WORLD_TILE_SIZE * 0.98f);
                } else if (tile == '\\') {
                    drawSpriteFill(trackCurveDownTexture, centerX, centerY,
                                   WORLD_TILE_SIZE * 0.98f,
                                   WORLD_TILE_SIZE * 0.98f);
                } else if (isSwitchTile(col, row)) {
                    const int index = getSwitchIndex(col, row);
                    const sf::Texture &texture =
                        (index >= 0 && switch_state[index] == 1)
                        ? switchBTexture : switchATexture;
                    drawSpriteFill(texture, centerX, centerY,
                                   WORLD_TILE_SIZE * 0.98f,
                                   WORLD_TILE_SIZE * 0.98f);
                } else if (tile == 'S') {
                    drawSpriteFill(sourceTexture, centerX, centerY,
                                   WORLD_TILE_SIZE * 0.94f,
                                   WORLD_TILE_SIZE * 0.94f);
                } else if (tile == 'D') {
                    drawSpriteFill(destinationTexture, centerX, centerY,
                                   WORLD_TILE_SIZE * 0.94f,
                                   WORLD_TILE_SIZE * 0.94f);
                }
            }
        }

        for (int i = 0; i < total_trains; ++i) {
            if (!train_active[i]) continue;
            const int direction = train_direction[i];
            if (direction < DIR_UP || direction > DIR_LEFT) continue;

            drawSpriteFit(
                trainTextures[direction],
                (train_x[i] + 0.5f) * WORLD_TILE_SIZE,
                (train_y[i] + 0.5f) * WORLD_TILE_SIZE,
                WORLD_TILE_SIZE * 0.96f,
                WORLD_TILE_SIZE * 0.78f
            );
        }

        if (fontLoaded) {
            window.setView(window.getDefaultView());

            sf::RectangleShape panel({540.f, 68.f});
            panel.setPosition({14.f, 14.f});
            panel.setFillColor(HUD_BACKGROUND);
            panel.setOutlineThickness(1.f);
            panel.setOutlineColor(HUD_BORDER);
            window.draw(panel);

            sf::Text title(font, "Switchback Rails", 16);
            title.setPosition({28.f, 22.f});
            title.setFillColor(sf::Color(245, 247, 249));
            window.draw(title);

            const string status =
                "Tick " + to_string(current_tick) +
                (isPaused ? "  |  PAUSED" : "  |  RUNNING") +
                "     Space: pause  .: step  Home: reset view";
            sf::Text controls(font, status, 12);
            controls.setPosition({28.f, 47.f});
            controls.setFillColor(sf::Color(190, 202, 212));
            window.draw(controls);
        }

        window.display();
    }
}

void cleanupApp() {
    if (window.isOpen()) window.close();
}

#pragma once

#include <string>

/**
 * Shared interface for game lifecycle methods
 * This defines the common API that both C++ and TypeScript engines implement
 */
class IGame {
public:
    virtual ~IGame() = default;

    /**
     * Called once when the game starts
     */
    virtual void onStart() = 0;

    /**
     * Called every frame to update game logic
     * @param deltaTime Time elapsed since last frame in seconds
     */
    virtual void onUpdate(float deltaTime) = 0;

    /**
     * Called every frame to render the game
     */
    virtual void onRender() = 0;

    /**
     * Called to check if the game wants to quit
     * @returns true if the game should exit
     */
    virtual bool wantsToQuit() const { return false; }
};

/**
 * Configuration options for running a game
 */
struct GameConfig {
    int width = 800;
    int height = 450;
    std::string title = "Game";
    int targetFPS = 60;
    std::string backgroundColor = "#000000";
};

/**
 * Engine capabilities and features
 */
struct EngineCapabilities {
    bool webgl = false;  // Only relevant for web version
    bool canvas2d = false;  // Only relevant for web version
    bool audio = true;
    bool input = true;
    bool networking = false;
};

/**
 * Vector2 for 2D coordinates
 */
struct Vec2 {
    float x;
    float y;

    Vec2(float x = 0, float y = 0) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 operator*(float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }
};

/**
 * Color representation
 */
struct EngineColor {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    EngineColor(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 255)
        : r(r), g(g), b(b), a(a) {}
};

/**
 * Rectangle for collision and rendering
 */
struct Rect {
    float x;
    float y;
    float width;
    float height;

    Rect(float x = 0, float y = 0, float width = 0, float height = 0)
        : x(x), y(y), width(width), height(height) {}

    bool contains(const Vec2& point) const {
        return point.x >= x && point.x <= x + width &&
               point.y >= y && point.y <= y + height;
    }

    bool intersects(const Rect& other) const {
        return !(x + width < other.x || other.x + other.width < x ||
                 y + height < other.y || other.y + other.height < y);
    }
};


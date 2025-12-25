#include <iostream>
#include <raylib.h>

#include "../lib/engine/include/game.h"
#include "../lib/engine/include/run.h"

class MyGame : public engine::Game {
private:
    float ballX = 400.0f;
    float ballY = 225.0f;
    float ballRadius = 20.0f;
    float ballVelocityX = 200.0f; // pixels per second
    float ballVelocityY = 150.0f; // pixels per second
    int canvasWidth = 800;
    int canvasHeight = 450;

public:
    void onStart() override {
        std::cout << "C++ Game started!" << std::endl;
        std::cout << "Raylib Bouncing Ball Demo - Press ESC to quit" << std::endl;
    }

    void onUpdate(float deltaTime) override {
        // Update ball position
        ballX += ballVelocityX * deltaTime;
        ballY += ballVelocityY * deltaTime;

        // Bounce off walls
        if (ballX - ballRadius <= 0 || ballX + ballRadius >= canvasWidth) {
            ballVelocityX = -ballVelocityX;
            ballX = std::max(ballRadius, std::min((float)canvasWidth - ballRadius, ballX));
        }

        if (ballY - ballRadius <= 0 || ballY + ballRadius >= canvasHeight) {
            ballVelocityY = -ballVelocityY;
            ballY = std::max(ballRadius, std::min((float)canvasHeight - ballRadius, ballY));
        }
    }

    void onRender() override {
        // Draw ball
        DrawCircle(ballX, ballY, ballRadius, RED);

        // Draw ball border
        DrawCircleLines(ballX, ballY, ballRadius, MAROON);

        // Draw some text
        DrawText("Raylib Bouncing Ball Demo", canvasWidth / 2 - 150, 30, 20, WHITE);
        DrawText("Press ESC to quit", canvasWidth / 2 - 80, canvasHeight - 30, 16, WHITE);

        // Draw some rectangles to show Raylib rendering
        DrawRectangle(50, 50, 100, 50, Fade(BLUE, 0.5f));

        // Draw a line
        DrawLine(200, 100, 300, 150, YELLOW);
    }
};

int main() {
    MyGame game;

    // Try new config-based API first, fallback to old API
    try {
        GameConfig config;
        config.width = 800;
        config.height = 450;
        config.title = "C++ Game Engine - Bouncing Ball";
        config.targetFPS = 60;
        config.backgroundColor = "#2c3e50";

        return engine::run(game, config);
    } catch (...) {
        // Fallback to old API if shared interfaces aren't available
        return engine::run(game, 800, 450, "C++ Game Engine - Bouncing Ball");
    }
}


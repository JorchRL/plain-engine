#include "../include/run.h"
#include <raylib.h>

namespace engine {
int run(Game &game, int width, int height, const char *title) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(width, height, title);
  SetTargetFPS(60);

  game.onStart();
  while (!WindowShouldClose() && !game.wantsToQuit()) {
    float deltaTime = GetFrameTime();
    game.onUpdate(deltaTime);
    BeginDrawing();
    ClearBackground(BLACK);
    game.onRender();
    EndDrawing();
  }
  CloseWindow();
  return 0;
}

int run(Game &game, const GameConfig& config) {
  return run(game, config.width, config.height, config.title.c_str());
}

} // namespace engine
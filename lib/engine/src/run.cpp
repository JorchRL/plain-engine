#include "run.h"
#include "raylib.h"

namespace engine {
int run(Game &game, int width, int height, const char *title) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(width, height, title);
  SetTargetFPS(60);

  game.onStart();
  while (!WindowShouldClose() && !game.wantsToQuit()) {
    game.onUpdate();
    BeginDrawing();
    ClearBackground(BLACK);
    game.onRender();
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
} // namespace engine

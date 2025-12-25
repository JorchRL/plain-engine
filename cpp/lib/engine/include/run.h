#pragma once
#include "game.h"


namespace engine {
int run(Game &game, const GameConfig& config = {});
int run(Game &game, int width = 800, int height = 450,
        const char *title = "Game");
}

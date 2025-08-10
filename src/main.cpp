#include <iostream>

#include "game.h"
#include "run.h"

class MyGame : public engine::Game {
public:
  void onStart() override { std::cout << "Start\n"; }
  void onUpdate() override {}
  void onRender() override {}
};

int main() {
  MyGame game;
  engine::run(game, 800, 450, "Pacman");
}

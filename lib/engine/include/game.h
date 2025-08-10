//
// Created by Jorge Romero Lagunas on 7/27/25.
//

#ifndef GAME_H
#define GAME_H
#pragma once

namespace engine {
class Renderer;

class Game {
public:
  virtual ~Game() = default;
  virtual void onStart() = 0;
  virtual void onUpdate() = 0;
  virtual void onRender() = 0;
  virtual bool wantsToQuit() const { return false; }
};
} // namespace engine

#endif // GAME_H

//
// Created by Jorge Romero Lagunas on 7/27/25.
//

#ifndef GAME_H
#define GAME_H
#pragma once

#include "IGame.h"

namespace engine {
class Renderer;

class Game : public IGame {
public:
  virtual ~Game() = default;

  // IGame interface implementation
  virtual void onStart() override = 0;
  virtual void onUpdate(float deltaTime) override = 0;
  virtual void onRender() override = 0;
  virtual bool wantsToQuit() const override { return false; }

  // Engine-specific methods
  virtual void setRenderer(Renderer* renderer) { this->renderer = renderer; }
  virtual Renderer* getRenderer() { return renderer; }

protected:
  Renderer* renderer = nullptr;
};
} // namespace engine

#endif // GAME_H

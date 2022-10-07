#pragma once

#include "vengine.h"
#include "../RoomHandler.h"

class GameScene: public Scene {
  private:
    int camEntity;
    int entity;

    RoomHandler roomHandler;

  public:
    GameScene();
    virtual ~GameScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void update() override;
};

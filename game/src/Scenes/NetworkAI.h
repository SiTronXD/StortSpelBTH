#pragma once

#include "vengine.h"

class NetworkAI: public Scene {
  private:
    int camEntity;
    int entity;

  public:
    NetworkAI();
    virtual ~NetworkAI();

    // Inherited via Scene
    virtual void init() override;
    virtual void update() override;
};

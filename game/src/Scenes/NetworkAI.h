#pragma once

#include "vengine.h"

class NetworkAI: public Scene {
  private:
    int camEntity;
    int player;

  public:
    NetworkAI();
    virtual ~NetworkAI();

    // Inherited via Scene
    virtual void init() override;
    virtual void update() override;
};

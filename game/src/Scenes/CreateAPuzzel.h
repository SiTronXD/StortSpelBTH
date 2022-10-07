#pragma once

#include "vengine.h"

class PuzzelCreator: public Scene {
  private:
    int camEntity;
    int ground;
    int mover;
    std::vector<int> puzzelObjects;
  public:
    PuzzelCreator();
    virtual ~PuzzelCreator();

    // Inherited via Scene
    virtual void init() override;
    virtual void update() override;
};

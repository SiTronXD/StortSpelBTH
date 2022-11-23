#pragma once

#include "vengine.h"

class GameOverScene : public Scene
{
private:
    Entity cam;

    Entity dirLight;
    Entity graveStone;
    Entity lightPillars[4];
    Entity crystals[4];
    Entity groundPillar;
    Entity ground;
    Entity trees[40];
    //Entity groundPillars[6];

    float camRotSpeed = 20.f;
    float camDist = 8.f; //15.f;
public:
    GameOverScene();
    virtual ~GameOverScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};


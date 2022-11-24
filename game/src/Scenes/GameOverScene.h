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
    Entity trees[148];
    Entity stones[60];

    float camRotSpeed = 20.f;
    float camDist = 35.f;

    float shakeScalar = 0.3f;
    float shakeTimer = 0.f;
    float shakeDuration = 0.1f;
    bool isShaking = false;

public:
    GameOverScene();
    virtual ~GameOverScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};


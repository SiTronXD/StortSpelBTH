#pragma once

#include "vengine.h"

class TutorialScene : public Scene
{
private:
    bool shouldLoadNextScene;

public:
    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};
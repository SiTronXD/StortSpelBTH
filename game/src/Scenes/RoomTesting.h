#pragma once

#include <cmath>

#include "vengine.h"
#include "../World Handling/Room Handler.h"

class RoomTesting : public Scene
{
private:

    RoomHandler roomHandler;
    Entity player1;
    Entity player2;

public:
    RoomTesting();
    virtual ~RoomTesting();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;

};

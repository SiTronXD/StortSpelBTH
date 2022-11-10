#pragma once

#include <cmath>

#include "vengine.h"
#include "../World Handling/Room Handler.h"

class RoomTesting : public Scene
{
private:

    RoomHandler roomHandler;

public:
    RoomTesting();
    virtual ~RoomTesting();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;

};

#pragma once

#include "vengine.h"
#include "vengine/shared_memory/Comlib.h"
#include "vengine/shared_memory/mayaStructures.h"
#include "../LevelEditor/ObjectHandler.h"
#include "../World Handling/Room Handler.h"

class LevelEditor: public Scene
{
private:
    Comlib* comlib;
    char* data;
    MessageHeader* header;

    ObjectHandler objectHandler;
    RoomHandler roomHandler;

    Entity playerID;
    Entity playerCamera;
    Entity mayaCamera;

    void readBuffer();

public:
    LevelEditor();
    ~LevelEditor();

    //  Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};

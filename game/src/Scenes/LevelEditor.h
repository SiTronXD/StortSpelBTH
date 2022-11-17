#pragma once

#include "vengine.h"
#include "vengine/shared_memory/Comlib.h"
#include "vengine/shared_memory/mayaStructures.h"
#include "../LevelEditor/ObjectHandler.h"

class LevelEditor: public Scene
{
private:
    Comlib* comlib;
    char* data;
    MessageHeader* header;
    
    Entity playerID;
    ObjectHandler objectHandler;
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
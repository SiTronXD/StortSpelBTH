#pragma once

#include "vengine.h"
#include "vengine/shared_memory/Comlib.h"
#include "vengine/shared_memory/mayaStructures.h"

class LevelEditor: public Scene
{
private:
  Comlib* comlib;
  char* msg;
  MessageHeader* header;

  Entity playerID;
  std::unordered_map<std::string, Entity> mayaObjects;

public:
    LevelEditor();
    ~LevelEditor();

    //  Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
};
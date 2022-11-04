#pragma once
#include "vengine/network/ServerEngine/NetworkScene.h"
#include "../vengine/vengine/ai/PathFinding.h"

class TheServerGame : public NetworkScene
{
 private:

 public:
  TheServerGame();
  void init();
  void update(float dt);

};
#pragma once
#include "../vengine/vengine/network/ServerGame.h"
#include "../AI/PathFinding.h"

class TheServerGame : public ServerGame
{
 private:
  PathFindingManager pfm;

 public:
  TheServerGame();
  void update(float dt);

};
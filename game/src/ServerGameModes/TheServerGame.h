#pragma once
#include "../vengine/vengine/network/ServerGame.h"
#include "../AI/PathFinding.h"
#include "../World Handling/RoomHandler.h"

class TheServerGame : public ServerGame
{
 private:
  PathFindingManager pfm;
  RoomHandler roomHandler;

 public:
  TheServerGame();
  void update(float dt);

};
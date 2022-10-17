#pragma once
#include "../vengine/vengine/network/ServerGame.h"
#include "../vengine/vengine/ai/PathFinding.h"
#include "../World Handling/RoomHandler.h"

class TheServerGame : public ServerGame
{
 private:
  RoomHandler roomHandler;

 public:
  TheServerGame();
  void update(float dt);

};
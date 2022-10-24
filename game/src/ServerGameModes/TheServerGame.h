#pragma once
#include "../vengine/vengine/network/ServerEngine/ServerGame.h"
#include "../vengine/vengine/ai/PathFinding.h"
#include "../World Handling/RoomHandler.h"

class TheServerGame : public ServerGameMode
{
 private:
  RoomHandler roomHandler;

 public:
  TheServerGame();
  void init();
  void update(float dt);

};
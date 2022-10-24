#pragma once
#include "../vengine/vengine/network/ServerEngine/ServerGame.h"
#include "../vengine/vengine/ai/PathFinding.h"

class TheServerGame : public ServerGameMode
{
 private:

 public:
  TheServerGame();
  void init();
  void update(float dt);

};
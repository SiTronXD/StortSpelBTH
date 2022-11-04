#pragma once
#include "vengine/network/ServerEngine/NetworkScene.h"
#include "../vengine/vengine/ai/PathFinding.h"
#include "ai/AIHandler.hpp"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"

class TheServerGame : public NetworkScene
{
 private:

  uint32_t fontTextureIndex; 
  AIHandler* aiHandler = nullptr;
  std::vector<int> enemyIDs;
  std::vector<SwarmGroup*> swarmGroups;
  SwarmFSM swarmFSM;
 public:
  TheServerGame();
  void start() override;
  void init() override;
  void update(float dt) override;
  void aiExample();
};
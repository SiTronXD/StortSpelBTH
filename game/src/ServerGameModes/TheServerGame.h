#pragma once
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "vengine/ai/AIHandler.hpp"
#include "vengine/network/ServerEngine/NetworkScene.h"

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
  virtual ~TheServerGame();
  void start() override;
  void init() override;
  void update(float dt) override;
  void aiExample();
};
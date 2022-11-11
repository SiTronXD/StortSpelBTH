#pragma once
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../vengine/vengine/ai/PathFinding.h"
#include "vengine/ai/AIHandler.hpp"
#include "vengine/network/ServerEngine/NetworkScene.h"
#include "../World Handling/Room Handler.h"

//SERVER SIDE!!!
class NetworkGameScene : public NetworkScene
{
private:
  uint32_t fontTextureIndex;
  AIHandler* aiHandler = nullptr;
  std::vector<int> enemyIDs;
  std::vector<SwarmGroup*> swarmGroups;
  SwarmFSM swarmFSM;

  //room handler
  int roomSeed;
  RoomHandler roomHandler;

public:
  NetworkGameScene();
  virtual ~NetworkGameScene();
  void start() override;
  void init() override;
  void update(float dt) override;
  void aiExample();
};
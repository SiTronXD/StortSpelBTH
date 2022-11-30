#pragma once
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../World Handling/Room Handler.h"
#include "../vengine/vengine/ai/PathFinding.h"
#include "vengine/ai/AIHandler.hpp"
#include "../World Handling/SpawnHandler.hpp"
#include "vengine/network/ServerEngine/NetworkScene.h"

//SERVER SIDE!!!
class NetworkGameScene : public NetworkScene
{
private:
  uint32_t					fontTextureIndex;
  AIHandler*				aiHandler = nullptr;
  SpawnHandler spawnHandler;

  //room handler
  int roomSeed;
  RoomHandler roomHandler;
  uint8_t numRoomsCleared;
  bool newRoomFrame;


public:
  NetworkGameScene();
  virtual ~NetworkGameScene();
  void start() override;
  void init() override;
  void update(float dt) override;
  virtual void onTriggerStay(Entity e1, Entity e2) override;
  virtual void onTriggerEnter(Entity e1, Entity e2) override;
  virtual void onCollisionEnter(Entity e1, Entity e2) override;
  virtual void onCollisionStay(Entity e1, Entity e2) override;
  virtual void onCollisionExit(Entity e1, Entity e2) override;

private:
  void aiExample();
  bool allDead();

  void createPortal();
};
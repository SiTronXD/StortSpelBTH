#pragma once

#include <cmath>

#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../World Handling/Room Handler.h"
#include "vengine.h"

class GameScene : public Scene
{
private:
  RoomHandler roomHandler;
  AIHandler* aiHandler = nullptr;

  Entity playerID;
  Entity swordID;
  Entity portal;
  Entity perk;
  Entity perk1;
  Entity perk2;
  Entity ability;

  int numRoomsCleared;
  bool newRoomFrame;

  std::vector<int> enemyIDs;
  std::vector<SwarmGroup*> swarmGroups;

  uint32_t abilityTextures[3];
  uint32_t perkTextures[4];
  uint32_t fontTextureIndex;

  uint32_t hpBarBackgroundTextureID;
  uint32_t hpBarTextureID;
  uint32_t portalOffMesh;
  uint32_t portalOnMesh;

public:
  GameScene();
  virtual ~GameScene();

  // Inherited via Scene
  virtual void init() override;
  virtual void start() override;
  virtual void update() override;
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
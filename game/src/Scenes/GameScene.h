#pragma once

#include <cmath>
#include <string>

#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../World Handling/Room Handler.h"
#include "../World Handling/SpawnHandler.hpp"
#include "vengine.h"
#include "../World Handling/Room Handler.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../Ai/Behaviors/Tank/TankFSM.hpp"
#include "../Ai/Behaviors/Lich/LichFSM.hpp"
#include "../Components/AICombatTank.hpp"
#include "../World Handling/ParticleSystemGenerator.hpp"

class NetworkHandlerGame;

class GameScene : public Scene
{
private:
  RoomHandler roomHandler;
  SpawnHandler spawnHandler;
  AIHandler* aiHandler = nullptr;
  NetworkHandlerGame* networkHandler;

  bool paused = false;
  UIArea resumeButton;
  UIArea exitButton;

  Entity playerID;
  Entity portal;
  Entity perk;
  Entity perk1;
  Entity perk2;
  Entity perk3;
  Entity perk4;
  Entity ability;
  Entity ability1;

  Entity dirLightEntity;

  int numRoomsCleared;
  bool newRoomFrame;

  int abilityMeshes[2];
  int perkMeshes[5];

  uint32_t abilityTextures[3];
  uint32_t perkTextures[6];
  uint32_t fontTextureIndex;
  uint32_t blackTextureIndex;

  uint32_t hpBarBackgroundTextureID;
  uint32_t hpBarTextureID;
  uint32_t portalOffMesh;
  uint32_t portalOnMesh;

  ParticleSystemInstance healParticleSystem;
  ParticleSystemInstance bloodParticleSystems;
  ParticleSystemInstance swarmParticleSystems;

  bool deletedParticleSystems;

  void initParticleSystems();
  void deleteInitialParticleSystems();

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

  inline const ParticleSystem& getHealParticleSystem() { return this->healParticleSystem.getParticleSystem(); }
  inline const ParticleSystem& getBloodParticleSystem() { return this->bloodParticleSystems.getParticleSystem(); }
  inline const ParticleSystem& getSwarmParticleSystem() { return this->swarmParticleSystems.getParticleSystem(); }

private:

  void createPortal();

  int colliderTest;
};
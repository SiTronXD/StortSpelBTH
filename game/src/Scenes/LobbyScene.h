#pragma once

#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../World Handling/Room Handler.h"
#include "vengine.h"
#include "vengine/network/NetworkEnumAndDefines.h"

class LobbyScene : public Scene
{
private:
  uint32_t fontTextureId;
  uint32_t backgroundId;

  Entity players[MAXNUMBEROFPLAYERS - 1];
  std::vector<Entity>activePlayers;

  std::vector<std::string> playersNames;
  std::vector<glm::vec3> playerPositions;

  int startButton;
  int disconnectButton;
  int light;
  int playerModel;

public:
  LobbyScene();
  virtual ~LobbyScene();

  // Inherited via Scene
  virtual void init() override;
  virtual void start() override;
  virtual void update() override;
  //virtual void onTriggerStay(Entity e1, Entity e2) override;
  //virtual void onCollisionStay(Entity e1, Entity e2) override;

private:
};

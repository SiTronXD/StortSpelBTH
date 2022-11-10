#pragma once

#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../World Handling/Room Handler.h"
#include "vengine.h"

class LobbyScene : public Scene
{
private:
  uint32_t fontTextureId;
  uint32_t backgroundId;

  std::vector<Entity> players;
  std::vector<std::string> playersNames;
  std::vector<glm::vec3> playerPositions;

  int startButton;
  int disconnectButton;
  int light;

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

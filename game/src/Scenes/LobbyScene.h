#pragma once
#include "vengine.h"

class LobbyScene : public Scene
{
private:
  static const int MaxNumberOfPlayers = 4;
  Entity Players[MaxNumberOfPlayers];
  uint32_t fontTextureId;

  std::vector<glm::vec3> playerPositions;

public:
  LobbyScene();
  virtual ~LobbyScene();

  // Inherited via Scene
  virtual void init() override;
  virtual void start() override;
  virtual void update() override;

private:
};

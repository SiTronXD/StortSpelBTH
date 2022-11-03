#pragma once
#include "vengine.h"

class LobbyScene : public Scene
{
private:
	
	Entity Players[4];//Max 4 players
  uint32_t fontTextureId;

public:
    LobbyScene();
  virtual ~LobbyScene();

  // Inherited via Scene
  virtual void init() override;
  virtual void start() override;
  virtual void update() override;

private:
};

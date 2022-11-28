#pragma once
#include "vengine/network/ServerEngine/NetworkSceneHandler.h"

class NetworkLobbyScene : public NetworkScene
{
  public:
	NetworkLobbyScene();
    virtual ~NetworkLobbyScene();
	void init();
	void update(float dt);
};
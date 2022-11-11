#include "vengine/pch/pch.h"
#include "NetworkLobbyScene.h"
#include "NetworkGameScene.h"

NetworkLobbyScene::NetworkLobbyScene() {}

void NetworkLobbyScene::init() {
	((NetworkSceneHandler*)this->getSceneHandler())->clientGetFunc();
}

void NetworkLobbyScene::update(float dt)
{
	if (((NetworkSceneHandler*)this->getSceneHandler())->getCallFromClient() == GameEvents::START)
	{
		std::cout << "got start in network lobby" << std::endl;
      ((NetworkSceneHandler*)this->getSceneHandler())->setScene(new NetworkGameScene());
		this->addEvent({(int)GameEvents::START});
	}
}
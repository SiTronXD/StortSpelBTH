#include "NetworkLobbyScene.h"
#include "NetworkGameScene.h"
#include "../Network/ServerGameMode.h"

NetworkLobbyScene::NetworkLobbyScene() {}

NetworkLobbyScene::~NetworkLobbyScene() {}

void NetworkLobbyScene::init() {
}


void NetworkLobbyScene::update(float dt)
{
	std::cout << "got start in network lobby" << std::endl;
	((NetworkSceneHandler*)this->getSceneHandler())->setScene(new ServerGameMode(3)); // REMOVE THE 3 LATER
	this->addEvent({(int)NetworkEvent::START});
}
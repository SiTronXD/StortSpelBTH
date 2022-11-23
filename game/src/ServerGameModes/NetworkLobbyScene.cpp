#include "NetworkLobbyScene.h"
#include "NetworkGameScene.h"

NetworkLobbyScene::NetworkLobbyScene() {}

NetworkLobbyScene::~NetworkLobbyScene() {}

void NetworkLobbyScene::init() {
	((NetworkSceneHandler*)this->getSceneHandler())->clientGetFunc();
}


void NetworkLobbyScene::update(float dt)
{
	sf::Packet packet = ((NetworkSceneHandler*)this->getSceneHandler())->getCallFromClient();
  while (!packet.endOfPacket())
  {
      int gameEvent;
      packet >> gameEvent;
      if (gameEvent == (int)NetworkEvent::START)
        {
          std::cout << "got start in network lobby" << std::endl;
          ((NetworkSceneHandler*)this->getSceneHandler())->setScene(new NetworkGameScene());
          this->addEvent({(int)NetworkEvent::START});
        }
  }
	
}
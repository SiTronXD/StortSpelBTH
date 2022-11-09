#include "LobbyScene.h"
#include "vengine/network/ServerGameModes/NetworkLobbyScene.h"
#include "GameScene.h"

LobbyScene::LobbyScene() {}
LobbyScene::~LobbyScene() {}

void LobbyScene::init()
{
  TextureSamplerSettings samplerSettings{};
  samplerSettings.filterMode = vk::Filter::eNearest;
  samplerSettings.unnormalizedCoordinates = VK_TRUE;

  this->backgroundId =
      this->getResourceManager()->addTexture("assets/textures/UI/background.png"
      );

  this->fontTextureId = Scene::getResourceManager()->addTexture(
      "assets/textures/UI/testBitmapFont.png", {samplerSettings, true}
  );
  Scene::getUIRenderer()->setBitmapFont(
      {"abcdefghij",
       "klmnopqrst",
       "uvwxyz+-.'",
       "0123456789",
       "!?,<>:()#^",
       "@         "},
      this->fontTextureId,
      16,
      16
  );

  int camEntity = this->createEntity();
  this->setComponent<Camera>(camEntity);
  this->setMainCamera(camEntity);
  this->getComponent<Transform>(camEntity).position = glm::vec3(0, 0, 0);
  this->getComponent<Transform>(camEntity).rotation = glm::vec3(0, 0, 0);

  playerPositions.push_back(glm::vec3(0, -10, 15));
  playerPositions.push_back(glm::vec3(10, -10, 18));
  playerPositions.push_back(glm::vec3(-10, -10, 18));
  playerPositions.push_back(glm::vec3(-5, -10, 21));

  this->startButton = this->createEntity();
  this->joinButton = this->createEntity();
  this->createServerButton = this->createEntity();
  this->disconnectButton = this->createEntity();

  UIArea area{};
  area.position = glm::vec2(800.f, 0.f);
  area.dimension = glm::vec2(20 * 10, 20);
  this->setComponent<UIArea>(this->startButton, area);

  area.position = glm::vec2(-800.f, -60.f);
  area.dimension = glm::vec2(20 * 4, 20);
  this->setComponent<UIArea>(this->joinButton, area);

  area.position = glm::vec2(-800.f, -120.f);
  area.dimension = glm::vec2(13 * 20, 20);
  this->setComponent<UIArea>(this->createServerButton, area);

  area.position = glm::vec2(-800.f, -60.f);
  area.dimension = glm::vec2(20 * 10, 20);
  this->setComponent<UIArea>(this->disconnectButton, area);
}

void LobbyScene::update()
{
  //set model position and player names
  if (this->getNetworkHandler()->getPlayers().size() > players.size())
    {
      for (int i = players.size();
           i < this->getNetworkHandler()->getPlayers().size();
           i++)
        {
          int e = this->getNetworkHandler()->getPlayers()[i].first;
          this->players.push_back(e);
          this->getComponent<Transform>(e).position = playerPositions[i];
          this->playersNames.push_back(
              this->getNetworkHandler()->getPlayers()[i].second
          );
        }
    }

  //write player names in lobby
  Scene::getUIRenderer()->setTexture(this->fontTextureId);
  for (int i = 0; i < this->playersNames.size(); i++)
    {
      this->getUIRenderer()->renderString(
          playersNames[i],
          1920 / 2 - 500,
          1080.f / 2 - 140 - (70 * i),
          40.f,
          40.f,
          0,
          StringAlignment::LEFT
      );
    }
  //start game
  if (getNetworkHandler()->hasServer())
    {
      this->getUIRenderer()->setTexture(this->fontTextureId);
      this->getUIRenderer()->renderString(
          "start match", 800.f, 0.f, 20.f, 20.f
      );
      if (this->getComponent<UIArea>(this->startButton).isClicking())
        {
          //send two
          std::cout << "pressed start" << std::endl;
          this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{GameEvents::START});
          this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{GameEvents::START});
          this->getSceneHandler()->setScene(new GameScene(), "scripts/gamescene.lua");
        }
      
    }
  else
    {
      if (this->getNetworkHandler()->getClient() != nullptr &&
          this->getNetworkHandler()->getClient()->hasStarted())
        {
          this->getSceneHandler()->setScene(
              new GameScene(), "scripts/gamescene.lua"
          );
      }
  }
  if (this->getNetworkHandler()->getClient() != nullptr && this->getNetworkHandler()->getClient()->isConnected())
  {
      this->getUIRenderer()->setTexture(this->fontTextureId);
      this->getUIRenderer()->renderString("disconnect", -800.f, -60.f, 20.f, 20.f);
      if (this->getComponent<UIArea>(disconnectButton).isClicking()) {
          this->getNetworkHandler()->disconnectClient();
      }
  }
  else
  {
      this->getUIRenderer()->setTexture(this->fontTextureId);
      this->getUIRenderer()->renderString("join", -800.f, -60.f, 20.f, 20.f);
      if (this->getComponent<UIArea>(this->joinButton).isClicking())
        {
          //TODO : change this to make it more graphical
          std::string ipaddres, name;
          std::cout << "name please" << std::endl;
          std::cin >> name;
          std::cout << "ip address please" << std::endl;
          std::cin >> ipaddres;

          std::cout << "joining server" << std::endl;
          this->getNetworkHandler()->createClient(name);
          if (this->getNetworkHandler()->connectClient(ipaddres))
            {
              std::cout << "pls server get name" << std::endl;
              this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent({GameEvents::GetPlayerNames}));
          }
          else
            {
              std::cout << "connect client return false" << std::endl;  
          }
          
        }

      this->getUIRenderer()->setTexture(this->fontTextureId);
      this->getUIRenderer()->renderString("create server", -800.f, -120.f, 20.f, 20.f);
      if (this->getComponent<UIArea>(this->createServerButton).isClicking())
      {
          std::cout << "create server" << std::endl;
          this->getNetworkHandler()->createServer(new NetworkLobbyScene());
          this->getNetworkHandler()->createClient("hosty");
          this->getNetworkHandler()->connectClientToThis();
      }
  }
  
}

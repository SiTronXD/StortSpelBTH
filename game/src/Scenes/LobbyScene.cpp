#include "LobbyScene.h"
//#include "vengine/network/ServerGameModes/NetworkLobbyScene.h"
#include "GameSceneNetwork.h"
#include "MainMenu.h"

LobbyScene::LobbyScene() {}
LobbyScene::~LobbyScene() {}

void LobbyScene::init()
{
    //TODO : FIX THIS!
  
  int playerModel = this->getResourceManager()->addAnimations(
      std::vector<std::string>({"assets/models/Character/CharRun.fbx"}), 
      "assets/textures/playerMesh"
  );
  this->getNetworkHandler()->setMeshes("PlayerMesh", playerModel);

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
      glm::vec2(16,16)
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
  this->disconnectButton = this->createEntity();

  UIArea area{};
  area.position = glm::vec2(800.f, 0.f);
  area.dimension = glm::vec2(20 * 10, 20);
  this->setComponent<UIArea>(this->startButton, area);

  area.position = glm::vec2(-800.f, -60.f);
  area.dimension = glm::vec2(20 * 10, 20);
  this->setComponent<UIArea>(this->disconnectButton, area);

  int mainPlayer = this->createEntity();
  this->setComponent<MeshComponent>(mainPlayer, playerModel);  //TODO : change to player model
  this->getComponent<Transform>(mainPlayer).position = playerPositions[0];
}

void LobbyScene::start() {
  if (!this->getNetworkHandler()->hasServer())
  {
      this->getNetworkHandler()->sendTCPDataToClient(
          TCPPacketEvent({GameEvents::GetPlayerNames})
      );  
  }  

  int background = this->createEntity();
  this->setComponent<MeshComponent>(background, 0);
  this->getComponent<Transform>(background).position.z = 100;
  this->getComponent<Transform>(background).scale.x = 200;
  this->getComponent<Transform>(background).scale.y = 100;

  int light = this->createEntity();
  this->setComponent<PointLight>(light);
  this->getComponent<PointLight>(light).color = glm::vec3(255, 200, 200);
  this->getComponent<Transform>(light).position = glm::vec3(0, 0, 0);
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
          this->getComponent<Transform>(e).position = playerPositions[i + 1];
          this->playersNames.push_back(
              this->getNetworkHandler()->getPlayers()[i].second
          );
        }
    }

  //write player names in lobby
  Scene::getUIRenderer()->setTexture(this->fontTextureId);
  this->getUIRenderer()->renderString(
      this->getNetworkHandler()->getClientName(),
      glm::vec2(1920 / 2 - 500, 1080.f / 2 - 140 - (70 * 0)),
      glm::vec2(40.f, 40.f),
      0,
      StringAlignment::LEFT
  );
  for (int i = 0; i < this->playersNames.size(); i++)
    {
      this->getUIRenderer()->renderString(
          playersNames[i],
          glm::vec2(1920 / 2 - 500,
          1080.f / 2 - 140 - (70 * (i + 1))),
          glm::vec2(40.f,
          40.f),
          0,
          StringAlignment::LEFT
      );
    }

  //start game
  if (getNetworkHandler()->hasServer())
    {
      this->getUIRenderer()->setTexture(this->fontTextureId);
      this->getUIRenderer()->renderString(
          "start match", glm::vec2(800.f, 0.f), glm::vec2(20.f, 20.f)
      );
      if (this->getComponent<UIArea>(this->startButton).isClicking())
        {
          //send two
          std::cout << "pressed start" << std::endl;
          this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{GameEvents::START});
          this->getSceneHandler()->setScene(
              new GameSceneNetwork(), "scripts/gamescene.lua"
          );
        }
      
    }
  else
    {
      if (this->getNetworkHandler()->getClient() != nullptr &&
          this->getNetworkHandler()->getClient()->hasStarted())
        {
          this->getSceneHandler()->setScene(
              new GameSceneNetwork(), "scripts/gamescene.lua"
          );
      }
  }

  if (this->getNetworkHandler()->getClient() != nullptr && this->getNetworkHandler()->getClient()->isConnected())
  {
      this->getUIRenderer()->setTexture(this->fontTextureId);
      this->getUIRenderer()->renderString("disconnect", glm::vec2(-800.f, -60.f), glm::vec2(20.f, 20.f));
      if (this->getComponent<UIArea>(disconnectButton).isClicking()) {
          this->getNetworkHandler()->disconnectClient();
          this->getNetworkHandler()->deleteServer();
          this->getSceneHandler()->setScene(new MainMenu, "scripts/MainMenu.lua");
      }
  }
  else
  {
      this->getNetworkHandler()->deleteServer();
      this->getSceneHandler()->setScene(new MainMenu, "scripts/MainMenu.lua");
  }

  
}

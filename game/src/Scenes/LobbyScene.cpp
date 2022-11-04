#include "LobbyScene.h"

LobbyScene::LobbyScene() {}

LobbyScene::~LobbyScene() {}

void LobbyScene::init()
{
  TextureSamplerSettings samplerSettings{};
  samplerSettings.filterMode = vk::Filter::eNearest;
  samplerSettings.unnormalizedCoordinates = VK_TRUE;
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
      fontTextureId,
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
}

void LobbyScene::start()
{
  this->getPhysicsEngine()->renderDebugShapes(true);
  int playerModel = this->getResourceManager()->addMesh(
      "assets/models/character_ready.fbx", "assets/textures/playerMesh"
  );
  for (int i = 0; i < MaxNumberOfPlayers; i++)
    {
      Players[i] = this->createEntity();
      this->setComponent<MeshComponent>(Players[i], playerModel);
      this->getComponent<Transform>(Players[i]).position = playerPositions[i];
      this->getComponent<Transform>(Players[i]).rotation = glm::vec3(0, 180, 0);
    }
}

void LobbyScene::update()
{
  this->getPhysicsEngine()->renderDebugShapes(true);
  //this is handle in networkHandler
  std::string playerNames[this->MaxNumberOfPlayers] = {
      "wwwwwwwwwwww", "lowe", "beccie", "oliwer"};

  Scene::getUIRenderer()->setTexture(this->fontTextureId);
  for (int i = 0; i < this->MaxNumberOfPlayers; i++)
    {
      this->getUIRenderer()->renderString(
          playerNames[i],
          1920 / 2 - 500,
          1080.f / 2 - 140 - (70 * i),
          40.f,
          40.f,
          0,
          StringAlignment::LEFT
      );
    }
  ////////////////
  //if we are the host create a start button
  //if (this->getNetworkHandler()->hasServer())
  //{
  //TODO : Make this a button
  this->getUIRenderer()->renderString(
      "start",
      -1920 / 2 + 250,
      -1080.f / 2 + 250,
      40.f,
      40.f,
      0,
      StringAlignment::CENTER
  );
  //}

  //TODO : make this a button
  this->getUIRenderer()->renderString(
      "disconnect",
      -1920 / 2 + 250,
      -1080.f / 2 + 180.f,
      40.f,
      40.f,
      0,
      StringAlignment::CENTER
  );
}

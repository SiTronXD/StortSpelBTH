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
}

void LobbyScene::start() {
  this->getPhysicsEngine()->renderDebugShapes(true);
}

void LobbyScene::update()
{
  this->getPhysicsEngine()->renderDebugShapes(true);
  //this is handle in networkHandler
  std::string players[4] = {"simon", "lowe", "beccie", "oliwer"};
  Scene::getUIRenderer()->setTexture(this->fontTextureId);
  for (int i = 0; i < 4; i++)
  {
      this->getUIRenderer()->renderString(
          players[i], 1920.f/2, 100.f - 60 * i, 40.f, 40.f, 0, StringAlignment::RIGHT
      );
  }
  ////////////////

  //if we are the host create a start button
  //if (this->getNetworkHandler()->hasServer())
  //{
  //TODO : Make this a button
      this->getUIRenderer()->renderString(
        "start", -400.f, 0.f, 40.f, 40.f, 0
      );
  //}
    
    //TODO : make this a button
      this->getUIRenderer()->renderString(
          "disconnect", -400.f, -50.f, 40.f, 40.f, 0
      );
    
}

#include "logInScene.h"

logInScene::logInScene() {

}
logInScene::~logInScene() {}

void logInScene::start()
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
}

void logInScene::update() 
{
    Input::k
}
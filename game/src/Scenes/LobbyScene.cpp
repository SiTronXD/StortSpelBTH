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

}

void LobbyScene::start() {}

void LobbyScene::update()
{
  ///this is handle in networkHandler
  Scene::getUIRenderer()->setTexture(this->fontTextureId);
  this->getUIRenderer()->renderString(
      "Play 1\nSettings 2\nQuit 3", 0.f, 0.f, 50.f, 50.f
  );

  ////////////////
}

#include "LobbyScene.h"


LobbyScene::LobbyScene()
{
  this->fontTextureIndex = Scene::getResourceManager()->addTexture(
      "assets/textures/UI/testBitmapFont.png"
  );
  Scene::getUIRenderer()->setBitmapFont(
      {"abcdefghij",
       "klmnopqrst",
       "uvwxyz+-.'",
       "0123456789",
       "!?,<>:()#^",
       "@         "},
      fontTextureIndex,
      16,
      16
  );
}

LobbyScene::~LobbyScene() {

}

void LobbyScene::init() {

}

void LobbyScene::start() {

}

void LobbyScene::update() {
  ///this is handle in networkHandler


  ////////////////
}

#include "LoadingScene.h"
#include "LobbyScene.h"

LoadingScene::LoadingScene()
	: loadingTextureIndex(~0u),
	blackTextureIndex(~0u),
	oneFrameHasPassed(false)
{

}

LoadingScene::~LoadingScene()
{

}

void LoadingScene::init() 
{
	// Camera
	int camEntity = this->createEntity();
	this->setComponent<Camera>(camEntity);
	this->setMainCamera(camEntity);

	// Loading texture
	this->loadingTextureIndex = 
		this->getResourceManager()->addTexture("assets/textures/logo.png", {});
	this->blackTextureIndex = 
		this->getResourceManager()->addTexture("vengine_assets/textures/Black.jpg", {});
}

void LoadingScene::start() 
{

}

void LoadingScene::update() 
{
	// Black background
	this->getUIRenderer()->setTexture(this->blackTextureIndex);
	this->getUIRenderer()->renderTexture(glm::vec2(0.0f), ResTranslator::getInternalDimensions());

	// Logo
	this->getUIRenderer()->setTexture(this->loadingTextureIndex);
	this->getUIRenderer()->renderTexture(glm::vec2(0.0f), glm::vec2(1921, 1079));

	// Switch scene after 1 frame
	if (this->oneFrameHasPassed)
	{
		this->switchScene(new LobbyScene(), "");
	}
	this->oneFrameHasPassed = true;
}

void LoadingScene::onTriggerStay(Entity e1, Entity e2) 
{

}

void LoadingScene::onTriggerEnter(Entity e1, Entity e2) 
{

}

void LoadingScene::onCollisionEnter(Entity e1, Entity e2) 
{

}

void LoadingScene::onCollisionStay(Entity e1, Entity e2) 
{

}

void LoadingScene::onCollisionExit(Entity e1, Entity e2) 
{

}

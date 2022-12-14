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
		this->getResourceManager()->addTexture("assets/textures/logo.png");
	this->blackTextureIndex = 
		this->getResourceManager()->addTexture("vengine_assets/textures/Black.jpg");

	// Font
	TextureSamplerSettings samplerSettings{};
	samplerSettings.filterMode = vk::Filter::eNearest;
	samplerSettings.unnormalizedCoordinates = VK_TRUE;

	int fontTextureId = Scene::getResourceManager()->addTexture(
		"assets/textures/UI/font.png", { samplerSettings, true });
	Scene::getUIRenderer()->setBitmapFont(
		{ "abcdefghij",
		 "klmnopqrst",
		 "uvwxyz+-.'",
		 "0123456789",
		 "!?,<>:()#^",
		 "@%/       " },
		fontTextureId,
		glm::uvec2(50, 50)
	);
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

	// Text
	this->getUIRenderer()->renderString(
		"loading...", 
		glm::vec2(0.0f, -350.0f), 
		glm::vec2(70.0f), 
		0.0f
	);

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

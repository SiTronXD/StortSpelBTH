#include "MainMenu.h"
#include "GameScene.h"

void MainMenu::init() 
{
	this->state = State::Menu;
	this->startGame = false;

	TextureSamplerSettings samplerSettings{};
	samplerSettings.filterMode = vk::Filter::eNearest;
	samplerSettings.unnormalizedCoordinates = VK_TRUE;

	this->backgroundId = this->getResourceManager()->addTexture("assets/textures/UI/background.png");

	this->fontTextureId = Scene::getResourceManager()->addTexture("assets/textures/UI/testBitmapFont.png", { samplerSettings, true });
	Scene::getUIRenderer()->setBitmapFont(
		{
			"abcdefghij",
			"klmnopqrst",
			"uvwxyz+-.'",
			"0123456789",
			"!?,<>:()#^",
			"@         "
		},
		this->fontTextureId, 16, 16);
}

void MainMenu::start() 
{
	
}

void MainMenu::update() 
{
	// Switches next frame to render loading texture
	if (this->startGame) 
	{
		this->switchScene(new GameScene(), "scripts/gamescene.lua");
	}

	this->getUIRenderer()->setTexture(this->backgroundId);
	this->getUIRenderer()->renderTexture(0.f, 0.f, 1920.f, 1080.f);

	switch (this->state)
	{
	default:
		break;
	case Menu:
		
		this->getUIRenderer()->setTexture(this->fontTextureId);
		this->getUIRenderer()->renderString("play: 1", 0.f, 200.f, 50.f, 50.f);
		this->getUIRenderer()->renderString("settings: 2", 0.f, 100.f, 50.f, 50.f);
		this->getUIRenderer()->renderString("how to play: 3", 0.f, 0.f, 50.f, 50.f);
		this->getUIRenderer()->renderString("quit: 4", 0.f, -100.f, 50.f, 50.f);

		if		(Input::isKeyReleased(Keys::ONE))	{ this->state = State::Play; }
		else if (Input::isKeyReleased(Keys::TWO))	{ this->state = State::Settings; }
		else if (Input::isKeyReleased(Keys::THREE)) { this->state = State::HowToPlay; }
		else if (Input::isKeyReleased(Keys::FOUR))	{ this->state = State::Quit; }

		break;
		
	case Play:
		this->getUIRenderer()->setTexture(this->fontTextureId);
		this->getUIRenderer()->renderString("loading...", 0.f, 0.f, 100.f, 100.f);
		this->startGame = true;
		break;

	case Settings:
		this->settings();
		break;
	case HowToPlay:
		this->howToPlay();		
		break;
	
	case Quit:
		this->getSceneHandler()->getWindow()->close();
		break;
	}
}

void MainMenu::settings()
{
	static bool fullscreen = false;
	if (Input::isKeyReleased(Keys::ONE))
	{ 
		this->state = State::Menu;
	}
	else if (Input::isKeyReleased(Keys::TWO)) 
	{ 
		fullscreen = fullscreen ? false : true;
		this->getSceneHandler()->getWindow()->setFullscreen(fullscreen);
	}

	UIRenderer* uiRenderer = this->getUIRenderer();

	uiRenderer->setTexture(this->fontTextureId);
	uiRenderer->renderString("back: 1", -(1920/2) + 200, (1080/2) - 100, 50.f, 50.f);
	
	uiRenderer->renderString("-- settings --", 0.f, 300.f, 50.f, 50.f);

	uiRenderer->renderString("temp switch key: 2", 0.f, 240.f, 15.f, 15.f);
	uiRenderer->renderString(fullscreen ? "fullscreen: on" : "fullscreen: off", 0.f, 200.f, 50.f, 50.f);

	uiRenderer->renderString("volume change disable until sound is happy happy", 0.f, 140.f, 15.f, 15.f);
	uiRenderer->renderString("volume: " + std::to_string((int)this->getAudioHandler()->getMasterVolume()), 0.f, 100.f, 50.f, 50.f);
}

void MainMenu::howToPlay()
{
	if (Input::isKeyReleased(Keys::ONE)) { this->state = State::Menu; }

	UIRenderer* uiRenderer = this->getUIRenderer();

	uiRenderer->setTexture(this->fontTextureId);
	uiRenderer->renderString("back: 1", -(1920/2) + 200, (1080/2) - 100, 50.f, 50.f);
	
	static const float XPos1 = -500.f;
	uiRenderer->renderString("-- controls --", XPos1, 100.f, 50.f, 50.f);
	uiRenderer->renderString("move: wasd", XPos1, 0.f, 50.f, 50.f);
	uiRenderer->renderString("jump: space", XPos1, -100.f, 50.f, 50.f);
	uiRenderer->renderString("attack: mouse 1", XPos1, -200.f, 50.f, 50.f);
	
	static const float XPos2 = 500.f;
	uiRenderer->renderString("-- objective --", XPos2, 100.f, 50.f, 50.f);
	uiRenderer->renderString("kill everything", XPos2, 0.f, 50.f, 50.f);
}
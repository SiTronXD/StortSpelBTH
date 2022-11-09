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
  this->playButton = this->createEntity();
  this->joinGameButton = this->createEntity();
  this->settingsButton = this->createEntity();
  this->quitButton = this->createEntity();
  this->backButton = this->createEntity();
  this->fullscreenButton = this->createEntity();

  UIArea area{};
  area.position = glm::vec2(0.f, 200.f);
  area.dimension = glm::vec2(50 * 5, 50);
  this->setComponent<UIArea>(this->playButton, area);

  area.position = glm::vec2(0.f, 100.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->joinGameButton, area);

  area.position = glm::vec2(0.f, 0.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->settingsButton, area);

    area.position = glm::vec2(0.f, -100.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->howToPlayButton, area);

  area.position = glm::vec2(0.f, -200.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->quitButton, area);

  area.position = glm::vec2(-(1920 / 2) + 200, (1080 / 2) - 100);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->backButton, area);

  area.position = glm::vec2(0.f, 200.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->fullscreenButton, area);


  Input::setHideCursor(false);
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
		this->getUIRenderer()->renderString("play", 0.f, 200.f, 50.f, 50.f);
		this->getUIRenderer()->renderString("join game", 0.f, 100.f, 50.f, 50.f);
		this->getUIRenderer()->renderString("settings", 0.f, 0.f, 50.f, 50.f);
        this->getUIRenderer()->renderString("how to play", 0.f, -100.f, 50.f, 50.f);
		this->getUIRenderer()->renderString("quit", 0.f, -200.f, 50.f, 50.f);
		
		if (this->getComponent<UIArea>(playButton).isClicking())
        {
            //go to write name
		}
        if (this->getComponent<UIArea>(joinGameButton).isClicking())
        {
            //go to write name and ip
        }
        if (this->getComponent<UIArea>(settingsButton).isClicking())
        {
            this->state = State::Settings;
        }
        if (this->getComponent<UIArea>(howToPlayButton).isClicking())
        {
            this->state = State::HowToPlay;
        }
        if (this->getComponent<UIArea>(quitButton).isClicking())
        {
            this->state = State::Quit;
        }

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
    if (this->getComponent<UIArea>(backButton).isClicking())
    {
        this->state = State::Menu;  
	}
    if (this->getComponent<UIArea>(fullscreenButton).isClicking())
    {
        fullscreen = fullscreen ? false : true;
        this->getSceneHandler()->getWindow()->setFullscreen(fullscreen);
    }

	UIRenderer* uiRenderer = this->getUIRenderer();

	uiRenderer->setTexture(this->fontTextureId);
	uiRenderer->renderString("back", -(1920 / 2) + 200, (1080 / 2) - 100, 50.f, 50.f);

	uiRenderer->renderString("-- settings --", 0.f, 300.f, 50.f, 50.f);

	//uiRenderer->renderString("temp switch key: 2", 0.f, 240.f, 15.f, 15.f);
	uiRenderer->renderString(fullscreen ? "fullscreen: on" : "fullscreen: off", 0.f, 200.f, 50.f, 50.f);

	uiRenderer->renderString("volume change disable until sound is happy happy", 0.f, 140.f, 15.f, 15.f);
	uiRenderer->renderString("volume: " + std::to_string((int)this->getAudioHandler()->getMasterVolume()), 0.f, 100.f, 50.f, 50.f);
}

void MainMenu::howToPlay()
{
	if (this->getComponent<UIArea>(backButton).isClicking())
    {
      this->state = State::Menu;
    }

	UIRenderer* uiRenderer = this->getUIRenderer();

	uiRenderer->setTexture(this->fontTextureId);
	uiRenderer->renderString("back: 1", -(1920 / 2) + 200, (1080 / 2) - 100, 50.f, 50.f);

	static const float XPos1 = -500.f;
	uiRenderer->renderString("-- controls --", XPos1, 100.f, 50.f, 50.f);
	uiRenderer->renderString("move: wasd", XPos1, 0.f, 50.f, 50.f);
	uiRenderer->renderString("jump: space", XPos1, -100.f, 50.f, 50.f);
	uiRenderer->renderString("attack: mouse 1", XPos1, -200.f, 50.f, 50.f);

	static const float XPos2 = 500.f;
	uiRenderer->renderString("-- objective --", XPos2, 100.f, 50.f, 50.f);
	uiRenderer->renderString("kill everything", XPos2, 0.f, 50.f, 50.f);
}
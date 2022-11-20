#include "MainMenu.h"
#include "GameScene.h"
#include "logInScene.h"
#include "../Network/ServerGameMode.h"
//#include "vengine/network/ServerGameModes/NetworkLobbyScene.h"

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
			"@%        "
		},
		fontTextureId,
		glm::uvec2(16, 16)
	);
}

void MainMenu::start()
{
	this->getAudioHandler()->setMusic("assets/Sounds/BackgroundMusic.ogg");
	this->getAudioHandler()->setMasterVolume(0.5f);
	this->getAudioHandler()->setMusicVolume(1.f);
	this->getAudioHandler()->playMusic();
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

	this->getUIRenderer()->setTexture(this->backgroundId);
	this->getUIRenderer()->renderTexture(glm::vec2(0.0f), glm::vec2(1920.0f, 1080.0f));

	switch (this->state)
	{
	default:
		break;
	case Menu:

		this->getUIRenderer()->setTexture(this->fontTextureId);
		this->getUIRenderer()->renderString("play",         glm::vec2(0.f, 200.f),  glm::vec2(50.f, 50.f));
		this->getUIRenderer()->renderString("join game",    glm::vec2(0.f, 100.f),  glm::vec2(50.f, 50.f));
		this->getUIRenderer()->renderString("settings",     glm::vec2(0.f, 0.f),    glm::vec2(50.f, 50.f));
        this->getUIRenderer()->renderString("how to play",  glm::vec2(0.f, -100.f), glm::vec2(50.f, 50.f));
		this->getUIRenderer()->renderString("quit",         glm::vec2(0.f, -200.f), glm::vec2(50.f, 50.f));
		
		if (this->getComponent<UIArea>(playButton).isClicking())
        {
            this->getUIRenderer()->setTexture(this->fontTextureId);
            this->getUIRenderer()->renderString(
                "loading...", glm::vec2(0.f, 0.f), glm::vec2(100.f, 100.f)
            );
            this->getNetworkHandler()->createServer(new ServerGameMode());
			this->getSceneHandler()->setScene(new logInScene());
		}
        if (this->getComponent<UIArea>(joinGameButton).isClicking())
        {
            this->getUIRenderer()->setTexture(this->fontTextureId);
            this->getUIRenderer()->renderString(
                "loading...", glm::vec2(0.f, 0.f), glm::vec2(100.f, 100.f)
            );
			this->getSceneHandler()->setScene(new logInScene());
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

    uiRenderer->renderString(
        "back: 1",
        glm::vec2(-(1920 / 2) + 200, (1080 / 2) - 100),
        glm::vec2(50.0f)
    );

    uiRenderer->renderString(
        "-- settings --", glm::vec2(0.f, 300.f), glm::vec2(50.0f)
    );

    uiRenderer->renderString(
        "temp switch key: 2", glm::vec2(0.f, 240.f), glm::vec2(15.0f)
    );
    uiRenderer->renderString(
        fullscreen ? "fullscreen: on" : "fullscreen: off",
        glm::vec2(0.f, 200.f),
        glm::vec2(50.0f)
    );

    uiRenderer->renderString(
        "volume change disable until sound is happy happy",
        glm::vec2(0.f, 140.f),
        glm::vec2(15.0f)
    );
    uiRenderer->renderString(
        "volume: " +
            std::to_string((int)this->getAudioHandler()->getMasterVolume()),
        glm::vec2(0.f, 100.f),
        glm::vec2(50.0f)
    );
}

void MainMenu::howToPlay()
{
	if (this->getComponent<UIArea>(backButton).isClicking())
    {
      this->state = State::Menu;
    }

	UIRenderer* uiRenderer = this->getUIRenderer();

	uiRenderer->setTexture(this->fontTextureId);
    uiRenderer->renderString("back: 1", glm::vec2(-(1920 / 2) + 200, (1080 / 2) - 100), glm::vec2(50.f, 50.f));

	static const float XPos1 = -500.f;
    uiRenderer->renderString("-- controls --", glm::vec2(XPos1, 100.f), glm::vec2(50.f, 50.f));
    uiRenderer->renderString("move: wasd", glm::vec2(XPos1, 0.f), glm::vec2(50.f, 50.f));
    uiRenderer->renderString("jump: space", glm::vec2(XPos1, -100.f), glm::vec2(50.f, 50.f));
    uiRenderer->renderString("attack: mouse 1", glm::vec2(XPos1, -200.f), glm::vec2(50.f, 50.f));

	static const float XPos2 = 500.f;
    uiRenderer->renderString("-- objective --", glm::vec2(XPos2, 100.f), glm::vec2(50.f, 50.f));
    uiRenderer->renderString("kill everything", glm::vec2(XPos2, 0.f), glm::vec2(50.f, 50.f));
}
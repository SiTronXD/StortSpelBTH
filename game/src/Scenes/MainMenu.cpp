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
			"@%        "
		},
		fontTextureId,
		glm::uvec2(16, 16)
	);
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
	this->getUIRenderer()->renderTexture(glm::vec2(0.0f), glm::vec2(1920.0f, 1080.0f));

	switch (this->state)
	{
	default:
		break;
	case Menu:

		//this->getUIRenderer()->setTexture(this->fontTextureId);
		this->getUIRenderer()->renderString("play: 1", glm::vec2(0.0f, 200.0f), glm::vec2(50.0f));
		this->getUIRenderer()->renderString("settings: 2", glm::vec2(0.0f, 100.f), glm::vec2(50.0f));
		this->getUIRenderer()->renderString("how to play: 3", glm::vec2(0.0f, 0.f), glm::vec2(50.0f));
		this->getUIRenderer()->renderString("quit: 4", glm::vec2(0.0f, -100.f), glm::vec2(50.0f));

		if (Input::isKeyReleased(Keys::ONE)) { this->state = State::Play; }
		else if (Input::isKeyReleased(Keys::TWO)) { this->state = State::Settings; }
		else if (Input::isKeyReleased(Keys::THREE)) { this->state = State::HowToPlay; }
		else if (Input::isKeyReleased(Keys::FOUR)) { this->state = State::Quit; }

		break;

	case Play:
		//this->getUIRenderer()->setTexture(this->fontTextureId);
		this->getUIRenderer()->renderString("loading...", glm::vec2(0.0f), glm::vec2(100.0f));
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

	//uiRenderer->setTexture(this->fontTextureId);
	uiRenderer->renderString("back: 1", glm::vec2(-(1920 / 2) + 200, (1080 / 2) - 100), glm::vec2(50.0f));

	uiRenderer->renderString("-- settings --", glm::vec2(0.f, 300.f), glm::vec2(50.0f));

	uiRenderer->renderString("temp switch key: 2", glm::vec2(0.f, 240.f), glm::vec2(15.0f));
	uiRenderer->renderString(fullscreen ? "fullscreen: on" : "fullscreen: off", glm::vec2(0.f, 200.f), glm::vec2(50.0f));

	uiRenderer->renderString("volume change disable until sound is happy happy", glm::vec2(0.f, 140.f), glm::vec2(15.0f));
	uiRenderer->renderString("volume: " + std::to_string((int)this->getAudioHandler()->getMasterVolume()), glm::vec2(0.f, 100.f), glm::vec2(50.0f));
}

void MainMenu::howToPlay()
{
	if (Input::isKeyReleased(Keys::ONE)) { this->state = State::Menu; }

	UIRenderer* uiRenderer = this->getUIRenderer();

	uiRenderer->setTexture(this->fontTextureId);
	uiRenderer->renderString("back: 1", glm::vec2(-(1920 / 2) + 200, (1080 / 2) - 100), glm::vec2(50.0f));

	static const float XPos1 = -500.f;
	uiRenderer->renderString("-- controls --", glm::vec2(XPos1, 100.f), glm::vec2(50.0f));
	uiRenderer->renderString("move: wasd", glm::vec2(XPos1, 0.f), glm::vec2(50.0f));
	uiRenderer->renderString("jump: space", glm::vec2(XPos1, -100.f), glm::vec2(50.0f));
	uiRenderer->renderString("attack: mouse 1", glm::vec2(XPos1, -200.f), glm::vec2(50.0f));

	static const float XPos2 = 500.f;
	uiRenderer->renderString("-- objective --", glm::vec2(XPos2, 100.f), glm::vec2(50.0f));
	uiRenderer->renderString("kill everything", glm::vec2(XPos2, 0.f), glm::vec2(50.0f));
}
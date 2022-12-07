#include "MainMenu.h"
#include "GameScene.h"
#ifdef WIN32
#include "LevelEditor.h"
#endif 
#include "logInScene.h"
#include "../ServerGameModes/NetworkLobbyScene.h"
#include "../Network/ServerGameMode.h"

void MainMenu::init()
{
	this->state = State::Menu;
	this->startGame = false;

	TextureSamplerSettings samplerSettings{};
	samplerSettings.filterMode = vk::Filter::eNearest;
	samplerSettings.unnormalizedCoordinates = VK_TRUE;

	Entity scene = this->createEntity();
	this->setComponent<MeshComponent>(
		scene,
		(int)this->getResourceManager()->addMesh("assets/models/Menu/scene.obj")
		);

	signpost = this->createEntity();
	this->setComponent<MeshComponent>(
		signpost,
		(int)this->getResourceManager()->addMesh("assets/models/Menu/signpost.obj")
		);

	// Fireflies particle system
	Entity fireflies = this->createEntity();
	Transform& firefliesTransform = this->getComponent<Transform>(fireflies);
	firefliesTransform.position = glm::vec3(7.438f, 16.571f, 0.813f);

	this->setComponent<ParticleSystem>(fireflies);
	ParticleSystem& firefliesPS = this->getComponent<ParticleSystem>(fireflies);
	firefliesPS.maxlifeTime = 2.0f;
	firefliesPS.numParticles = 8;
	firefliesPS.textureIndex = this->getResourceManager()->addTexture("assets/textures/firefliesParticle.png");
	firefliesPS.startSize = glm::vec2(0.7f);
	firefliesPS.endSize = glm::vec2(0.0f);
	firefliesPS.startColor = glm::vec4(0.0f);
	firefliesPS.endColor = glm::vec4(1.0f);
	firefliesPS.velocityStrength = 1.0f;
	firefliesPS.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	firefliesPS.spawnRate = 0.01f;
	firefliesPS.coneSpawnVolume.diskRadius = 1.5f;
	firefliesPS.coneSpawnVolume.coneAngle = 170.0f;
	firefliesPS.coneSpawnVolume.localDirection = glm::vec3(0.0f, -1.0f, 0.0f);
	firefliesPS.coneSpawnVolume.localPosition = glm::vec3(0.0f, 0.0f, 0.0f);


	this->settingsBackgroundId =
		this->getResourceManager()->addTexture(
			"assets/textures/UI/settings.png"
		);
	this->howToPlayBackgroundId =
		this->getResourceManager()->addTexture("assets/textures/UI/howToPlay.png"
		);

	Transform& tS = this->getComponent<Transform>(signpost);
	tS.position = glm::vec3(18.3f, -6.2f, -12.4f);
	tS.rotation = glm::vec3(0, 196.5f, 0);

	//Light
	light = this->createEntity();
	this->setComponent<DirectionalLight>(
		light, glm::vec3(-0.5f, -1.0f, 1.0f), glm::vec3(0.6f)
		);
	this->setComponent<PointLight>(
		light, { glm::vec3(8, 16,0), glm::vec3(50.f, 25.f, 50.f) }
	);
	DirectionalLight& dirLight =
		this->getComponent<DirectionalLight>(light);

	this->fontTextureId = Scene::getResourceManager()->addTexture("assets/textures/UI/font.png", { samplerSettings, true });
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
		glm::uvec2(50, 50)
	);
}

void MainMenu::start()
{
	camera = this->createEntity();
	this->setComponent<Camera>(camera);
	this->setMainCamera(camera);
	this->getComponent<Transform>(camera).position = glm::vec3(10, 10, -20);
	this->getComponent<Transform>(camera).rotation = glm::vec3(0, 11, -0);

	this->getSceneHandler()->getScriptHandler()->getGlobal(character, "character");
	this->setAnimation(character, "idle");
	Transform& t = this->getComponent<Transform>(character);
	t.rotation = glm::vec3(0, 108, 0);

	this->getAudioHandler()->setMusic("assets/Sounds/BackgroundMusic.ogg");
	this->getAudioHandler()->setMasterVolume(0.5f);
	this->getAudioHandler()->setMusicVolume(1.f);
	this->getAudioHandler()->playMusic();
	this->howToPlayButton = this->createEntity();
	this->playButton = this->createEntity();
	this->joinGameButton = this->createEntity();
	this->settingsButton = this->createEntity();
	this->quitButton = this->createEntity();
	this->backButton = this->createEntity();
	this->fullscreenButton = this->createEntity();
	this->howToPlayButton = this->createEntity();
	this->levelEditButton = this->createEntity();

	UIArea area{};
	area.position = glm::vec2(-430.f, 415.f);
	area.dimension = glm::vec2(60 * 10, 100.f);
	this->setComponent<UIArea>(this->playButton, area);


	area.position = glm::vec2(-450.f, 230.f);
	area.dimension = glm::vec2(60 * 10, 100.f);
	this->setComponent<UIArea>(this->joinGameButton, area);

	area.position = glm::vec2(-440.f, 50.f);
	area.dimension = glm::vec2(60 * 10, 100.f);
	this->setComponent<UIArea>(this->levelEditButton, area);

	area.position = glm::vec2(-450.f, -80.f);
	area.dimension = glm::vec2(60 * 10, 100.f);
	this->setComponent<UIArea>(this->settingsButton, area);

	area.position = glm::vec2(-450.f, -220.f);
	area.dimension = glm::vec2(60 * 10, 100.f);
	this->setComponent<UIArea>(this->howToPlayButton, area);

	area.position = glm::vec2(-490.f, -390.f);
	area.dimension = glm::vec2(60 * 10, 100.f);
	this->setComponent<UIArea>(this->quitButton, area);

	area.position = glm::vec2(-745.f, -360.f);
	area.dimension = glm::vec2(38 * 5, 65);
	this->setComponent<UIArea>(this->backButton, area);

	area.position = glm::vec2(0.f, 195.f);
	area.dimension = glm::vec2(50 * 10, 55);
	this->setComponent<UIArea>(this->fullscreenButton, area);

	if (this->getNetworkHandler()->hasServer())
    {
		this->getNetworkHandler()->deleteServer();    
	}
    if (this->getNetworkHandler()->isConnected())
    {
		this->getNetworkHandler()->disconnectClient();    
	}

	Input::setHideCursor(false);
}

void MainMenu::update()
{
	switch (this->state)
	{
	default:
		break;
	case Menu:
		if (this->getComponent<UIArea>(playButton).isClicking())
		{
			this->getUIRenderer()->setTexture(this->fontTextureId);
			this->getUIRenderer()->renderString(
				"loading...", glm::vec2(0.f, 0.f), glm::vec2(100.f, 100.f)
			);
			this->getNetworkHandler()->createServer(new NetworkLobbyScene());
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
		if (this->getComponent<UIArea>(levelEditButton).isClicking())
		{
			this->state = State::LevelEdit;
		}

		break;

	case Settings:
		this->settings();
		break;
	case HowToPlay:
		this->howToPlay();
		break;

#ifdef WIN32
	case LevelEdit:
		this->switchScene(new LevelEditor(), "scripts/levelEditor.lua");

		break;
#endif 

	case Quit:
		this->getSceneHandler()->getWindow()->close();
		break;
	}
}

void MainMenu::settings()
{
	UIRenderer* uiRenderer = this->getUIRenderer();

	uiRenderer->setTexture(this->settingsBackgroundId);
	uiRenderer->renderTexture(glm::vec2(0.0f), glm::vec2(1920.0f, 1080.0f));

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


	uiRenderer->renderString(
		fullscreen ? "fullscreen: on" : "fullscreen: off",
		glm::vec2(0.f, 200.f),
		glm::vec2(50.0f)
	);

	uiRenderer->renderString(
		"volume change disable until sound is happy happy",
		glm::vec2(0.f, 140.f),
		glm::vec2(25.0f)
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
	this->getUIRenderer()->setTexture(this->howToPlayBackgroundId);
	this->getUIRenderer()->renderTexture(
		glm::vec2(0.0f), glm::vec2(1920.0f, 1080.0f)
	);

	if (this->getComponent<UIArea>(backButton).isClicking())
	{
		this->state = State::Menu;
	}
}
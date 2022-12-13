#include "MainMenu.h"
#include "GameScene.h"
#ifdef WIN32
#include "LevelEditor.h"
#endif 
#include "logInScene.h"
#include "../ServerGameModes/NetworkLobbyScene.h"
#include "../Network/ServerGameMode.h"
#include "../Settings/Settings.h"

void MainMenu::init()
{
	this->state = State::Menu;
	Settings::sceneHandler = this->getSceneHandler();

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
	firefliesPS.numParticles = 6;
	firefliesPS.textureIndex = this->getResourceManager()->addTexture("assets/textures/firefliesParticle.png");
	firefliesPS.startSize = glm::vec2(1.5f);
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

	// Leaves particle system
	this->leaves = this->createEntity();
	Transform& leavesTransform = this->getComponent<Transform>(leaves);
	leavesTransform.position = glm::vec3(-20.0f, 20.0f, 7.0f);

	this->setComponent<ParticleSystem>(leaves);
	ParticleSystem& leavesPS = this->getComponent<ParticleSystem>(leaves);
	leavesPS.maxlifeTime = 12.0f;
	leavesPS.numParticles = 128;
	leavesPS.textureIndex = this->getResourceManager()->addTexture("assets/textures/leafParticle.png");
	leavesPS.startSize = glm::vec2(0.0f);
	leavesPS.endSize = glm::vec2(0.7f);
	leavesPS.startColor = glm::vec4(1.0f);
	leavesPS.endColor = glm::vec4(0.3f);
	leavesPS.velocityStrength = 5.0f;
	leavesPS.acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	leavesPS.spawnRate = 1.0f;
	leavesPS.coneSpawnVolume.diskRadius = 10.5f;
	leavesPS.coneSpawnVolume.coneAngle = 140.0f;
	leavesPS.coneSpawnVolume.localDirection = glm::vec3(1.0f, -1.0f, 0.2f);
	leavesPS.coneSpawnVolume.localPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	leavesPS.initialSimulation = true;

	this->settingsBackgroundId = this->getResourceManager()->addTexture("assets/textures/UI/settings.png");
	this->howToPlayBackgroundId = this->getResourceManager()->addTexture("assets/textures/UI/howToPlay.png");
	this->buttonTexture = this->getResourceManager()->addTexture("assets/textures/UI/button.jpg");
	this->buttonSound = this->getResourceManager()->addSound("assets/Sounds/buttonClick.ogg");
	this->qrCodeID = this->getResourceManager()->addTexture("assets/textures/UI/Presumed Dead QR.png");

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
	dirLight.shadowMapAngleBias = 0.006f;

	this->fontTextureId = Scene::getResourceManager()->addTexture("assets/textures/UI/font.png", { samplerSettings, true });
	Scene::getUIRenderer()->setBitmapFont(
		{
			"abcdefghij",
			"klmnopqrst",
			"uvwxyz+-.'",
			"0123456789",
			"!?,<>:()#^",
			"@%/       "
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

	this->getAudioHandler()->setMusic("assets/Sounds/GameMusic/MenuMusic.ogg");
	this->getAudioHandler()->setMasterVolume(0.5f);
	this->getAudioHandler()->setMusicVolume(1.f);
	this->getAudioHandler()->playMusic();
	this->howToPlayButton = this->createEntity();
	this->hostButton = this->createEntity();
	this->joinGameButton = this->createEntity();
	this->settingsButton = this->createEntity();
	this->quitButton = this->createEntity();
	this->backButton = this->createEntity();
	this->howToPlayButton = this->createEntity();
	this->levelEditButton = this->createEntity();
    this->singlePlayerButton = this->createEntity();

	UIArea area{};
	area.position = glm::vec2(-430.f, 415.f);
	area.dimension = glm::vec2(600.0f, 100.f);
	this->setComponent<UIArea>(this->hostButton, area);

	area.position = glm::vec2(-450.f, 230.f);
	area.dimension = glm::vec2(600.0f, 100.f);
	this->setComponent<UIArea>(this->joinGameButton, area);

	area.position = glm::vec2(650.f, -400.f);
	area.dimension = glm::vec2(300.0f, 100.f);
	this->setComponent<UIArea>(this->levelEditButton, area);

	area.position = glm::vec2(-440.f, 50.f);
    area.dimension = glm::vec2(60 * 10, 100.f);
    this->setComponent<UIArea>(this->singlePlayerButton, area);

	area.position = glm::vec2(-450.f, -80.f);
	area.dimension = glm::vec2(600.0f, 100.f);
	this->setComponent<UIArea>(this->settingsButton, area);

	area.position = glm::vec2(-450.f, -220.f);
	area.dimension = glm::vec2(600.0f, 100.f);
	this->setComponent<UIArea>(this->howToPlayButton, area);

	area.position = glm::vec2(-490.f, -390.f);
	area.dimension = glm::vec2(600.0f, 100.f);
	this->setComponent<UIArea>(this->quitButton, area);

	area.position = glm::vec2(-745.f, -360.f);
	area.dimension = glm::vec2(190.0f, 65.0f);
	this->setComponent<UIArea>(this->backButton, area);

	if (this->getNetworkHandler()->hasServer())
    {
		this->getNetworkHandler()->deleteServer();    
	}
    if (this->getNetworkHandler()->isConnected())
    {
		this->getNetworkHandler()->disconnectClient();    
	}
    this->getNetworkHandler()->setStatus(ServerStatus::WAITING);

	Input::setHideCursor(false);
	this->settingsEntity = this->createEntity();
	this->setScriptComponent(this->settingsEntity, "scripts/settings.lua");
	Settings::setEntity(this->settingsEntity);
	Settings::updateValues();
}

void MainMenu::update()
{
	this->setInactive(this->settingsEntity);

	switch (this->state)
	{
	default:
		break;
	case Menu:
		if (this->getComponent<UIArea>(hostButton).isClicking())
		{
			this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
			this->getUIRenderer()->setTexture(this->fontTextureId);
			this->getUIRenderer()->renderString(
				"loading...", glm::vec2(0.f, 0.f), glm::vec2(100.f, 100.f)
			);
			this->getNetworkHandler()->createServer(new NetworkLobbyScene());
			this->getSceneHandler()->setScene(new logInScene());
		}
		if (this->getComponent<UIArea>(joinGameButton).isClicking())
		{
			this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
			this->getUIRenderer()->setTexture(this->fontTextureId);
			this->getUIRenderer()->renderString(
				"loading...", glm::vec2(0.f, 0.f), glm::vec2(100.f, 100.f)
			);
			this->getSceneHandler()->setScene(new logInScene());
		}
		if (this->getComponent<UIArea>(settingsButton).isClicking())
		{
			this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
			this->state = State::Settings;
		}
		if (this->getComponent<UIArea>(howToPlayButton).isClicking())
		{
			this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
			this->state = State::HowToPlay;
		}
		if (this->getComponent<UIArea>(quitButton).isClicking())
		{
			this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
			this->state = State::Quit;
		}
        if (this->getComponent<UIArea>(singlePlayerButton).isClicking())
        {
            this->getUIRenderer()->setTexture(this->fontTextureId);
			this->getUIRenderer()->renderString(
				"loading...", glm::vec2(0.f, 0.f), glm::vec2(100.f, 100.f)
			);
            this->getSceneHandler()->setScene(new GameScene(), "scripts/gamescene.lua");
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
	this->setActive(this->settingsEntity);
	UIRenderer* uiRenderer = this->getUIRenderer();

	uiRenderer->setTexture(this->settingsBackgroundId);
	uiRenderer->renderTexture(glm::vec2(0.0f), glm::vec2(1920.0f, 1080.0f));

	if (this->getComponent<UIArea>(backButton).isClicking())
	{
		this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
		this->state = State::Menu;
	}
	Settings::updateValues();

	/*UIRenderer* uiRenderer = this->getUIRenderer();

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
	);*/
}

void MainMenu::howToPlay()
{
	this->getUIRenderer()->setTexture(this->howToPlayBackgroundId);
	this->getUIRenderer()->renderTexture(
		glm::vec2(0.0f), glm::vec2(1920.0f, 1080.0f)
	);

	this->getUIRenderer()->setTexture(this->buttonTexture);
	this->getUIRenderer()->renderTexture(glm::vec2(-745.0f, -360.0f), glm::vec2(190.0f, 65.0f), glm::uvec4(0, 0, 1, 1),
		glm::vec4(1.0f, 1.0f, 1.0f, 0.85f + this->getComponent<UIArea>(backButton).isHovering() * 0.15f));
	this->getUIRenderer()->renderString("back", glm::vec2(-745.0f, -355.0f), glm::vec2(40.0f));

	if (this->getComponent<UIArea>(backButton).isClicking())
	{
		this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
		this->state = State::Menu;
	}
}
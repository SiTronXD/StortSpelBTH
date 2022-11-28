#include "MainMenu.h"
#include "GameScene.h"
#include "LevelEditor.h"
#include "logInScene.h"

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
    

    //Light
    light = this->createEntity();
    this->setComponent<DirectionalLight>(
        light, glm::vec3(-0.5f, -1.0f, 1.0f), glm::vec3(0.6f)
    );
    this->setComponent<PointLight>(
        light, {glm::vec3(8, 16,0), glm::vec3(50.f, 25.f, 50.f)}
    );
    DirectionalLight& dirLight =
        this->getComponent<DirectionalLight>(light);
    dirLight.cascadeSizes[0] = 0.044f;
    dirLight.cascadeSizes[1] = 0.149f;
    dirLight.cascadeSizes[2] = 1.0f;
    dirLight.cascadeDepthScale = 36.952f;
    dirLight.shadowMapMinBias = 0.00001f;
    dirLight.shadowMapAngleBias = 0.0004f;

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
    camera = this->createEntity();
    this->setComponent<Camera>(camera);
    this->setMainCamera(camera);
    this->getComponent<Transform>(camera).position = glm::vec3(10, 10, -20);
    this->getComponent<Transform>(camera).rotation = glm::vec3(0, 11, -0);

    this->getSceneHandler()->getScriptHandler()->getGlobal(character, "character");
    this->getComponent<AnimationComponent>(character).animationIndex = 0;
    this->setAnimation(character, "idle");
    Transform& t = this->getComponent<Transform>(character);
    t.rotation = glm::vec3(0,108,0);

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
  this->levelEditButton = this->createEntity();

  UIArea area{};
  area.position = glm::vec2(-500.f, 200.f);
  area.dimension = glm::vec2(50 * 5, 50);
  this->setComponent<UIArea>(this->playButton, area);

  area.position = glm::vec2(-500.f, 100.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->joinGameButton, area);

  area.position = glm::vec2(-500.f, 0.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->settingsButton, area);

    area.position = glm::vec2(-500.f, -100.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->howToPlayButton, area);

  area.position = glm::vec2(-500.f, -200.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->quitButton, area);

  area.position = glm::vec2(-(1920 / 2) + 200, (1080 / 2) - 100);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->backButton, area);

  area.position = glm::vec2(0.f, 200.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->fullscreenButton, area);

  area.position = glm::vec2(0.f, -300.f);
  area.dimension = glm::vec2(50 * 10, 50);
  this->setComponent<UIArea>(this->levelEditButton, area);

  Input::setHideCursor(false);
}

void MainMenu::update()
{
	// Switches next frame to render loading texture

	//this->getUIRenderer()->setTexture(this->backgroundId);
	//this->getUIRenderer()->renderTexture(glm::vec2(0.0f), glm::vec2(1920.0f, 1080.0f));

    ImGui::Begin("Character transform");
    Transform& t = this->getComponent<Transform>(character);
    ImGui::SliderFloat("position X", &t.position.x, -20.f, 20.f);
    ImGui::SliderFloat("position Y", &t.position.y, -20.f, 20.f);
    ImGui::SliderFloat("position Z", &t.position.z, -20.f, 20.f);

    ImGui::SliderFloat("rotation X", &t.rotation.x, 0.f, 360.f);
    ImGui::SliderFloat("rotation Y", &t.rotation.y, 0.f, 360.f);
    ImGui::SliderFloat("rotation Z", &t.rotation.z, 0.f, 360.f);
    ImGui::End();

    ImGui::Begin("Signpost transform");
    Transform& tS = this->getComponent<Transform>(signpost);
    ImGui::SliderFloat("position X", &tS.position.x, -20.f, 20.f);
    ImGui::SliderFloat("position Y", &tS.position.y, -20.f, 20.f);
    ImGui::SliderFloat("position Z", &tS.position.z, -20.f, 20.f);
                                               
    ImGui::SliderFloat("rotation X", &tS.rotation.x, 0.f, 360.f);
    ImGui::SliderFloat("rotation Y", &tS.rotation.y, 0.f, 360.f);
    ImGui::SliderFloat("rotation Z", &tS.rotation.z, 0.f, 360.f);
    ImGui::End();

    
    ImGui::Begin("Point light");
    PointLight& pL = this->getComponent<PointLight>(light);
    ImGui::SliderFloat("position X", &pL.color.x, 0.f, 50.f);
    ImGui::SliderFloat("position Y", &pL.color.y, 0.f, 50.f);
    ImGui::SliderFloat("position Z", &pL.color.z, 0.f, 50.f);

    ImGui::End();

	switch (this->state)
	{
	default:
		break;
	case Menu:

		this->getUIRenderer()->setTexture(this->fontTextureId);
		this->getUIRenderer()->renderString("play",         glm::vec2(-430.f, 410.f),  glm::vec2(50.f, 50.f));
		this->getUIRenderer()->renderString("join game",    glm::vec2(-450.f, 230.f),  glm::vec2(50.f, 50.f));
		this->getUIRenderer()->renderString("level editor",     glm::vec2(-440.f, 50.f),    glm::vec2(50.f, 50.f));
		this->getUIRenderer()->renderString("settings",     glm::vec2(-450.f, -80.f),    glm::vec2(50.f, 50.f));
        this->getUIRenderer()->renderString("how to play",  glm::vec2(-450.f, -205.f), glm::vec2(50.f, 50.f));
		this->getUIRenderer()->renderString("quit",         glm::vec2(-470.f, -365.f), glm::vec2(50.f, 50.f));
		
		if (this->getComponent<UIArea>(playButton).isClicking())
        {
            this->getUIRenderer()->setTexture(this->fontTextureId);
            this->getUIRenderer()->renderString(
                "loading...", glm::vec2(0.f, 0.f), glm::vec2(100.f, 100.f)
            );
            //TODO : Change to networkLobbyScene
            this->getNetworkHandler()->createServer();
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

	case LevelEdit:
		this->switchScene(new LevelEditor(), "scripts/levelEditor.lua");
		
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
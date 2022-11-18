#include "LobbyScene.h"
#include "GameScene.h"
#include "MainMenu.h"

LobbyScene::LobbyScene() {}
LobbyScene::~LobbyScene() {}

void LobbyScene::init()
{
    this->networkHandler = dynamic_cast<NetworkHandlerGame*>(this->getNetworkHandler());
  
    //TODO : FIX THIS!
    int playerModel = this->getResourceManager()->addAnimations({ "assets/models/Character/Emotes/CharDance.fbx" }, "assets/textures/playerMesh");
    this->getNetworkHandler()->setMeshes("PlayerMesh", playerModel);

    TextureSamplerSettings samplerSettings{};
    samplerSettings.filterMode = vk::Filter::eNearest;
    samplerSettings.unnormalizedCoordinates = VK_TRUE;

    this->backgroundId =
        this->getResourceManager()->addTexture("assets/textures/UI/background.png"
        );

    this->fontTextureId = Scene::getResourceManager()->addTexture(
        "assets/textures/UI/testBitmapFont.png", {samplerSettings, true}
    );
    Scene::getUIRenderer()->setBitmapFont(
        {"abcdefghij",
        "klmnopqrst",
        "uvwxyz+-.'",
        "0123456789",
        "!?,<>:()#^",
        "@%        "},
        this->fontTextureId,
        glm::vec2(16,16)
    );

    int camEntity = this->createEntity();
    this->setComponent<Camera>(camEntity);
    this->setMainCamera(camEntity);
    this->getComponent<Transform>(camEntity).position = glm::vec3(0, 0, 0);
    this->getComponent<Transform>(camEntity).rotation = glm::vec3(0, 0, 0);

    this->players.resize(4);
    this->playersNames.resize(4);
    for (int i = 0; i < 4; i++)
    {
        this->players[i] = this->createEntity();
        this->setComponent<MeshComponent>(this->players[i], playerModel);
        this->setComponent<AnimationComponent>(this->players[i]);

        Transform& t = this->getComponent<Transform>(this->players[i]);
        t.position = this->POSITIONS[i];
        t.rotation = glm::vec3(0, 180, 0);
        t.scale = glm::vec3(1);
    }

    this->startButton = this->createEntity();
    this->disconnectButton = this->createEntity();

    UIArea area{};
    area.position = glm::vec2(800.f, 0.f);
    area.dimension = glm::vec2(20 * 10, 20);
    this->setComponent<UIArea>(this->startButton, area);

    area.position = glm::vec2(-800.f, -60.f);
    area.dimension = glm::vec2(20 * 10, 20);
    this->setComponent<UIArea>(this->disconnectButton, area);
}

void LobbyScene::start() 
{
    int background = this->createEntity();
    this->setComponent<MeshComponent>(background, 0);
    this->getComponent<Transform>(background).position.z = 100;
    this->getComponent<Transform>(background).scale.x = 200;
    this->getComponent<Transform>(background).scale.y = 100;

    int light = this->createEntity();
    this->setComponent<PointLight>(light);
    this->getComponent<PointLight>(light).color = glm::vec3(10, 10, 10);
    this->getComponent<Transform>(light).position = glm::vec3(0, 0, 0);
}

void LobbyScene::update()
{
    // Set model position and player names
    auto netPlayers = this->networkHandler->getPlayers();
    if (netPlayers.size() != this->activePlayers - 1)
    {
        this->playersNames.clear();
        this->activePlayers = netPlayers.size() + 1;
        Log::write("Active players: " + std::to_string(this->activePlayers));
        for (int i = 0; i < netPlayers.size(); i++)
        {
            this->setActive(this->players[i + 1]);
            this->playersNames.push_back(
                this->getNetworkHandler()->getPlayers()[i].second
            );
        }
        for (int i = this->activePlayers; i < MAX_PLAYER_COUNT; i++)
        {
            this->setInactive(this->players[i]);
        }
    }

    // Write player names in lobby
    Scene::getUIRenderer()->setTexture(this->fontTextureId);
    this->getUIRenderer()->renderString(
        this->getNetworkHandler()->getClientName(),
        glm::vec2(1920 / 2 - 500, 1080.f / 2 - 140 - (70 * 0)),
        glm::vec2(40.f, 40.f),
        0,
        StringAlignment::LEFT
    );
    for (int i = 0; i < this->playersNames.size(); i++)
    {
        this->getUIRenderer()->renderString(
            playersNames[i],
            glm::vec2(1920 / 2 - 500,
            1080.f / 2 - 140 - (70 * (i + 1))),
            glm::vec2(40.f,
            40.f),
            0,
            StringAlignment::LEFT
        );
    }

    // Start game
    if (getNetworkHandler()->hasServer())
    {
        this->getUIRenderer()->setTexture(this->fontTextureId);
        this->getUIRenderer()->renderString(
            "start match", glm::vec2(800.f, 0.f), glm::vec2(20.f, 20.f)
        );
        if (this->getComponent<UIArea>(this->startButton).isClicking())
        {
            this->helpPacket << (int)NetworkEvent::START;
            this->getNetworkHandler()->sendDataToServerTCP(helpPacket);
            this->getSceneHandler()->setScene(
                new GameScene(), "scripts/gamescene.lua"
            );
        }
    }
    else if (this->getNetworkHandler()->getStatus() == ServerStatus::RUNNING)
    {
        this->getSceneHandler()->setScene(
            new GameScene(), "scripts/gamescene.lua"
        );
    }

    if (!this->getNetworkHandler()->isConnected())
    {
        this->switchScene(new MainMenu, "scripts/MainMenu.lua");
    }

    if (this->getNetworkHandler()->getClient() != nullptr)
    {
        this->getUIRenderer()->setTexture(this->fontTextureId);
        this->getUIRenderer()->renderString("disconnect", glm::vec2(-800.f, -60.f), glm::vec2(20.f, 20.f));
        if (this->getComponent<UIArea>(disconnectButton).isClicking()) {
            this->getNetworkHandler()->disconnectClient();
            this->getNetworkHandler()->deleteServer();
            this->getSceneHandler()->setScene(new MainMenu, "scripts/MainMenu.lua");
        }
    }

    this->helpPacket.clear();
}

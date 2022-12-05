#include "LobbyScene.h"
#include "GameScene.h"
#include "MainMenu.h"

LobbyScene::LobbyScene(const std::string& serverIP) : serverIP(serverIP) {}
LobbyScene::~LobbyScene() {}

void LobbyScene::init()
{
  this->playerModel = this->getResourceManager()->addAnimations(
        std::vector<std::string>(
            {"assets/models/Character/CharIdle.fbx",
             "assets/models/Character/CharRun.fbx",
             "assets/models/Character/CharDodge.fbx",
             "assets/models/Character/CharOutwardAttack.fbx",
             "assets/models/Character/CharHeavyAttack.fbx",
             "assets/models/Character/CharSpinAttack.fbx",
             "assets/models/Character/CharKnockbackAttack.fbx",
             "assets/models/Character/CharInwardAttack.fbx",
             "assets/models/Character/CharSlashAttack.fbx",
             "assets/models/Character/DeathAnim.fbx"}
        ), 
      "assets/textures/playerMesh"
  );
    this->getResourceManager()->mapAnimations(
        playerModel,
        std::vector<std::string>(
            {"idle",
             "run",
             "dodge",
             "lightAttack",
             "heavyAttack",
             "spinAttack",
             "knockback",
             "mixAttack",
             "slashAttack",
             "dead"}
        )
    );

    int tank = this->getResourceManager()->addAnimations({
                        "assets/models/Tank/TankWalk.fbx",
                        "assets/models/Tank/TankCharge.fbx",
                        "assets/models/Tank/TankGroundHump.fbx",
                        "assets/models/Tank/TankRaiseShield.fbx"
        },
        "assets/textures/"
    );
    this->getResourceManager()->mapAnimations(tank, {
        "Walk",
        "Charge",
        "GroundHump",
        "RaiseShield",
        });
    this->getResourceManager()->createAnimationSlot(tank, "LowerBody", "Character1_Hips");
    this->getResourceManager()->createAnimationSlot(tank, "UpperBody", "Character1_Spine");

    int lich = this->getResourceManager()->addAnimations({
                "assets/models/Lich/Lich_Walk.fbx",
                "assets/models/Lich/Lich_Attack.fbx",
        },
        "assets/textures/Lich/"
        );
    this->getResourceManager()->mapAnimations(lich,
        {
            "Walk",
            "Attack"
        });
   
  TextureSamplerSettings samplerSettings{};
  samplerSettings.filterMode = vk::Filter::eNearest;
  samplerSettings.unnormalizedCoordinates = VK_TRUE;
    this->networkHandler = dynamic_cast<NetworkHandlerGame*>(this->getNetworkHandler());

    this->backgroundId =
        this->getResourceManager()->addTexture("assets/textures/blackTex.png"
        );
    this->buttonId =
        this->getResourceManager()->addTexture("assets/textures/UI/button.png");

    this->fontTextureId = Scene::getResourceManager()->addTexture(
        "assets/textures/UI/font.png", {samplerSettings, true}
    );
    Scene::getUIRenderer()->setBitmapFont(
        {"abcdefghij",
        "klmnopqrst",
        "uvwxyz+-.'",
        "0123456789",
        "!?,<>:()#^",
        "@%        "},
        this->fontTextureId,
        glm::vec2(50,50)
    );

    int camEntity = this->createEntity();
    this->setComponent<Camera>(camEntity);
    this->setMainCamera(camEntity);
    this->getComponent<Transform>(camEntity).position = glm::vec3(0, 0, 0);
    this->getComponent<Transform>(camEntity).rotation = glm::vec3(0, 0, 0);

    scene = this->createEntity();
    this->setComponent<MeshComponent>(scene, (int)this->getResourceManager()->addMesh("assets/models/Menu/lobby.obj"));
    Transform& t = this->getComponent<Transform>(scene);
    t.position = glm::vec3(0.f, 0.5f, 40.f);
    t.rotation = glm::vec3(0.f, 180.f, 0.f);

    this->players.resize(MAX_PLAYER_COUNT);
    this->playersNames.resize(MAX_PLAYER_COUNT);
    for (int i = 0; i < 4; i++)
    {
        this->players[i] = this->createEntity();
        this->setComponent<MeshComponent>(this->players[i], playerModel);
        this->setComponent<AnimationComponent>(this->players[i]);
        this->setInactive(this->players[i]);
        Transform& t = this->getComponent<Transform>(this->players[i]);
        t.position = this->POSITIONS[i];
        t.rotation = glm::vec3(0, 180, 0);

        if (i != 0)
        {
            MeshComponent& mesh = this->getComponent<MeshComponent>(this->players[i]);
            this->getResourceManager()->makeUniqueMaterials(mesh);
            mesh.overrideMaterials[0].tintColor = this->networkHandler->playerColors[i];
        }
    }
    this->setActive(players[0]);

    startButton.position = glm::vec2(0.0f, -400.f);
    startButton.dimension = glm::vec2(275.0f, 100.0f);

    disconnectButton.position = glm::vec2(-700.f, -400.f);
    disconnectButton.dimension = glm::vec2(225.0f, 75.0f);
}

void LobbyScene::start() 
{
    int background = this->createEntity();
    this->setComponent<MeshComponent>(background, 0);
    this->getComponent<Transform>(background).position.z = 100;
    this->getComponent<Transform>(background).scale.x = 200;
    this->getComponent<Transform>(background).scale.y = 100;

    int light = this->createEntity();
    this->setComponent<DirectionalLight>(
        light, glm::vec3(-0.5f, -1.0f, 1.0f), glm::vec3(0.6f)
    );
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
        this->getComponent<AnimationComponent>(this->players[0]).aniSlots[0].timer = 0.0f;
        for (int i = 0; i < netPlayers.size(); i++)
        {
            this->setActive(this->players[i + 1]);
            this->playersNames.push_back(
                this->getNetworkHandler()->getPlayers()[i].second
            );
            this->getComponent<AnimationComponent>(this->players[i + 1]).aniSlots[0].timer = 0.0f;
        }
        for (int i = this->activePlayers; i < MAX_PLAYER_COUNT; i++)
        {
            this->setInactive(this->players[i]);
        }
    }

    // Button backtgrounds
    this->getUIRenderer()->setTexture(this->buttonId);
    this->getUIRenderer()->renderTexture(
        this->disconnectButton.position, this->disconnectButton.dimension,
        glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, 0.85f + this->disconnectButton.isHovering() * 0.15f));
    if (this->getNetworkHandler()->hasServer())
    {
        this->getUIRenderer()->renderTexture(
            this->startButton.position, this->startButton.dimension,
            glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, 0.85f + this->startButton.isHovering() * 0.15f));
    }

    // Write player names in lobby
    this->getUIRenderer()->setTexture(this->backgroundId);
    this->getUIRenderer()->renderString(
        this->getNetworkHandler()->getClientName(),
        this->POSITIONS[0] + glm::vec3(0.0f, 20.0f, 0.0f),
        glm::vec2(100.0f)
    );
    for (int i = 0; i < this->playersNames.size(); i++)
    {
        this->getUIRenderer()->renderString(
            playersNames[i],
            this->POSITIONS[i + 1] + glm::vec3(0.0f, 20.0f, 0.0f),
            glm::vec2(100.0f)
        );
    }
    this->getUIRenderer()->renderString(
        "address code: " + this->serverIP,
        glm::vec2(0.0f, 450.0f),
        glm::vec2(40.0f)
    );

    // Start game
    if (this->getNetworkHandler()->getStatus() == ServerStatus::RUNNING)
    {
        this->switchScene(
            new GameScene(), "scripts/gamescene.lua"
        );
    }
    else if (this->getNetworkHandler()->hasServer())
    {
        this->getUIRenderer()->renderString(
            "start", this->startButton.position, glm::vec2(50.0f), 0.0f, StringAlignment::CENTER
        );
        if (this->startButton.isClicking())
        {
            // Start singleplayer
            if (this->activePlayers == 1)
            {
               this->getNetworkHandler()->disconnectClient();
               this->getNetworkHandler()->deleteServer();
               this->switchScene(
                   new GameScene(), "scripts/gamescene.lua"
               );
            }
            else
            {
                this->helpPacket << (int)NetworkEvent::START;
                this->getNetworkHandler()->sendDataToServerTCP(helpPacket);
            }
        }
    }

    if (!this->getNetworkHandler()->isConnected())
    {
        this->getNetworkHandler()->disconnectClient();
        this->getNetworkHandler()->deleteServer();
        this->switchScene(new MainMenu, "scripts/MainMenu.lua");
    }

    this->getUIRenderer()->renderString(
        "disconnect", this->disconnectButton.position, glm::vec2(25.0f), 0.0f, StringAlignment::CENTER
    );
    if (this->disconnectButton.isClicking()) {
        this->getNetworkHandler()->disconnectClient();
        this->getNetworkHandler()->deleteServer();
        this->getSceneHandler()->setScene(new MainMenu, "scripts/MainMenu.lua");
    }

    this->helpPacket.clear();
}

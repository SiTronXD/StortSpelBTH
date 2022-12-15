#include "LobbyScene.h"
#include "GameScene.h"
#include "MainMenu.h"

LobbyScene::LobbyScene(const std::string& serverIP) : serverIP(serverIP) {}
LobbyScene::~LobbyScene() {}

void LobbyScene::preloadAssets()
{
    // ----------------------- Main menu -----------------------
    this->getResourceManager()->addMesh("assets/models/Menu/scene.obj");
    this->getResourceManager()->addMesh("assets/models/Menu/signpost.obj");

    this->getResourceManager()->addTexture("assets/textures/firefliesParticle.png");
    this->getResourceManager()->addTexture("assets/textures/leafParticle.png");
    this->getResourceManager()->addTexture("assets/textures/UI/settings.png");
    this->getResourceManager()->addTexture("assets/textures/UI/howToPlay.png");
    this->getResourceManager()->addTexture("assets/textures/UI/button.jpg");
    this->getResourceManager()->addTexture("assets/textures/UI/Presumed Dead QR.png");

    TextureSettings fogSettings{};
    fogSettings.samplerSettings.addressMode = vk::SamplerAddressMode::eClampToEdge;
    this->getResourceManager()->addTexture("assets/textures/UI/fogGradient.png", fogSettings);
    this->getResourceManager()->addTexture("assets/textures/UI/fog.jpg", fogSettings);

    this->getResourceManager()->addSound("assets/Sounds/buttonClick.ogg");

    TextureSamplerSettings samplerSettings{};
    samplerSettings.filterMode = vk::Filter::eNearest;
    samplerSettings.unnormalizedCoordinates = VK_TRUE;
    uint32_t fontTextureId = Scene::getResourceManager()->addTexture("assets/textures/UI/font.png", { samplerSettings, true });
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


    // ----------------------- Game scene -----------------------
    int playerMesh = this->getResourceManager()->addAnimations(
        {
            "assets/models/Character/CharIdle.fbx",
            "assets/models/Character/CharRun2.fbx",
            "assets/models/Character/CharDodge.fbx",
            "assets/models/Character/CharOutwardAttack.fbx",
            "assets/models/Character/CharHeavyAttack.fbx",
            "assets/models/Character/CharSpinAttack.fbx",
            "assets/models/Character/CharKnockbackAttack.fbx",
            "assets/models/Character/CharInwardAttack.fbx",
            "assets/models/Character/CharSlashAttack.fbx",
            "assets/models/Character/DeathAnim.fbx"
        },
        "assets/textures/playerMesh"
    );
    this->getResourceManager()->mapAnimations(
        playerMesh,
        {
            "idle",
            "run",
            "dodge",
            "lightAttack",
            "heavyAttack",
            "spinAttack",
            "knockback",
            "mixAttack",
            "slashAttack",
            "dead"
        }
    );
    this->getResourceManager()->createAnimationSlot(
        playerMesh, "LowerBody", "mixamorig:Hips"
    );
    this->getResourceManager()->createAnimationSlot(
        playerMesh, "UpperBody", "mixamorig:Spine1"
    );

    int swarm =
        this->getResourceManager()->addMesh("assets/models/Swarm_RotTest.obj");
    int tank = this->getResourceManager()->addAnimations(
        {
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
        }
    );
    this->getResourceManager()->createAnimationSlot(tank, "LowerBody", "Character1_Hips");
    this->getResourceManager()->createAnimationSlot(tank, "UpperBody", "Character1_Spine");
    int lich = this->getResourceManager()->addAnimations(
        {
            "assets/models/Lich/Lich_Walk.fbx",
            "assets/models/Lich/Lich_Attack.fbx",
        },
        "assets/textures/Lich/"
    );
    this->getResourceManager()->mapAnimations(lich, 
        {
        "Walk",
        "Attack"
        }
    );
    this->getResourceManager()->addMesh("assets/models/KnockbackAbility.obj");
    this->getResourceManager()->addMesh("assets/models/Ability_Healing.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_Hp.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_Dmg.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_AtkSpeed.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_Movement.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_Stamina.obj");
    this->getResourceManager()->addTexture("assets/textures/UI/knockbackAbility.png");
    this->getResourceManager()->addTexture("assets/textures/UI/HealingAbility.png");
    this->getResourceManager()->addTexture("assets/textures/UI/empty.png");
    this->getResourceManager()->addTexture("assets/textures/UI/hpUp.png");
    this->getResourceManager()->addTexture("assets/textures/UI/dmgUp.png");
    this->getResourceManager()->addTexture("assets/textures/UI/atkSpeedUp.png");
    this->getResourceManager()->addTexture("assets/textures/UI/moveUp.png");
    this->getResourceManager()->addTexture("assets/textures/UI/staminaUp.png");
    this->getResourceManager()->addTexture("assets/textures/UI/empty.png");
    this->getResourceManager()->addTexture("assets/textures/UI/hpBarBackground.jpg");
    this->getResourceManager()->addTexture("assets/textures/UI/hpBar.jpg");
    this->getResourceManager()->addTexture("vengine_assets/textures/Black.jpg");
    this->getResourceManager()->addTexture("assets/textures/UI/GhostUI.png");
    this->getResourceManager()->addSound("assets/Sounds/buttonClick.ogg");

    this->getResourceManager()->addTexture("assets/textures/playerMesh/CharacterTextureGhost.jpg");
    this->getResourceManager()->addTexture("assets/textures/playerMesh/CharacterTextureGhostGlow.jpg");

    this->getResourceManager()->addMesh("assets/models/PortalOff.obj");
    this->getResourceManager()->addMesh("assets/models/PortalOn.obj");
    this->getResourceManager()->addCollisionShapeFromMesh("assets/models/portal.fbx");
    this->getResourceManager()->addMesh("assets/models/fire_orb.obj");
    this->getResourceManager()->addMesh("assets/models/light_orb.obj");
    this->getResourceManager()->addMesh("assets/models/ice_orb.obj");

    this->getResourceManager()->addTexture("assets/textures/UI/HealingAbilityParticle.png");
    this->getResourceManager()->addTexture("assets/textures/bloodParticle.png");
    this->getResourceManager()->addTexture("assets/textures/slimeParticle.png");        
    this->getResourceManager()->addTexture("assets/textures/grassDustParticle.png");
    this->getResourceManager()->addTexture("assets/textures/portalParticle.png");
    this->getResourceManager()->addTexture("assets/textures/orbParticle.png");

    // ----------------------- Lua game scene -----------------------
    TextureSettings pixelArtSettings{};
    pixelArtSettings.samplerSettings.filterMode = vk::Filter::eNearest;

    this->getResourceManager()->addTexture("assets/textures/UI/hpBarBackground.jpg");
    this->getResourceManager()->addTexture("assets/textures/UI/staminaBar.jpg");
    this->getResourceManager()->addTexture("assets/textures/UI/perkSlot.png", pixelArtSettings);
    this->getResourceManager()->addTexture("assets/textures/UI/UIBarMid.png");
    this->getResourceManager()->addTexture("assets/textures/UI/UIBarSide.png");
    this->getResourceManager()->addTexture("assets/textures/UI/UIBarMid.png");
    this->getResourceManager()->addTexture("assets/textures/UI/UIBarSide.png");
    this->getResourceManager()->addTexture("assets/textures/UI/UIBorder.png");
    this->getResourceManager()->addTexture("assets/textures/UI/frame.png");
    this->getResourceManager()->addTexture("assets/textures/UI/button.jpg");

    // ----------------------- Room handler -----------------------
    for (uint32_t i = 0; i < RoomHandler::NUM_BORDER; i++)
    {
        this->getResourceManager()->addMesh("assets/models/Tiles/Border/" + std::to_string(i + 1u) + ".obj");
    }
    this->getResourceManager()->addMesh("assets/models/Tiles/Border/innerBorder.obj");

    for (uint32_t i = 0; i < RoomHandler::NUM_ONE_X_ONE; i++)
    {
        this->getResourceManager()->addMesh("assets/models/Tiles/OneXOne/" + std::to_string(i + 1u) + ".obj");
    }

    for (uint32_t i = 0; i < RoomHandler::NUM_ONE_X_TWO; i++)
    {
        this->getResourceManager()->addMesh("assets/models/Tiles/OneXTwo/" + std::to_string(i + 1u) + ".obj");
        const int collId = (int) this->getResourceManager()->addCollisionShapeFromMesh("assets/models/Tiles/OneXTwo/" + std::to_string(i + 1u) + ".obj");
        this->getResourceManager()->getCollisionShapeFromMesh(collId).size() ? collId : ~0u;
    }

    for (uint32_t i = 0; i < RoomHandler::NUM_TWO_X_TWO; i++)
    {
        this->getResourceManager()->addMesh("assets/models/Tiles/TwoXTwo/" + std::to_string(i + 1u) + ".obj");
        const int collId = this->getResourceManager()->addCollisionShapeFromMesh("assets/models/Tiles/TwoXTwo/" + std::to_string(i + 1u) + ".obj");
        this->getResourceManager()->getCollisionShapeFromMesh(collId).size() ? collId : ~0u;

    }

    this->getResourceManager()->addMesh("assets/models/door.obj");
    this->getResourceManager()->addMesh("assets/models/tempRock.obj");
    this->getResourceManager()->addMesh("assets/models/rockFence.obj");
    this->getResourceManager()->addMesh("assets/models/Tiles/Floor.obj");
    this->getResourceManager()->addMesh("assets/models/Tiles/OneXTwo/lamp.obj");

    this->getResourceManager()->addTexture("assets/textures/lampTex.jpg");
    this->getResourceManager()->addTexture("assets/textures/Perk_HpTex.jpg");

    // ----------------------- Log in scene -----------------------
    this->getResourceManager()->addTexture("assets/textures/blackTex.jpg");
    this->getResourceManager()->addTexture("assets/textures/UI/button.jpg");

    this->buttonSound = this->getResourceManager()->addSound("assets/Sounds/buttonClick.ogg");

    // ----------------------- Game over scene -----------------------
    this->getResourceManager()->addMesh("assets/models/GameOverScene/GraveStone.obj");
    this->getResourceManager()->addMesh("assets/models/GameOverScene/LightPillar.obj");
    this->getResourceManager()->addMesh("assets/models/Tiles/Floor.obj");
    this->getResourceManager()->addMesh("assets/models/Tiles/Border/1.obj");
    this->getResourceManager()->addMesh("assets/models/Tiles/OneXOne/1.obj");
    this->getResourceManager()->addMesh("assets/models/GameOverScene/GroundTop.obj");
    this->getResourceManager()->addMesh("assets/models/GameOverScene/Crystal.obj");

    this->getResourceManager()->addTexture("vengine_assets/textures/DefaultEmission.jpg");

    // ----------------------- Network handler game -----------------------
    this->getResourceManager()->addSound("assets/Sounds/PlayerSounds/RunningSound.ogg");
    this->getResourceManager()->addMesh("assets/models/Perk_Hp.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_Dmg.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_AtkSpeed.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_Movement.obj");
    this->getResourceManager()->addMesh("assets/models/Perk_Stamina.obj");
    this->getResourceManager()->addMesh("assets/models/KnockbackAbility.obj");
    this->getResourceManager()->addMesh("assets/models/Ability_Healing.obj");
    this->getResourceManager()->addMesh("assets/models/HealingAbility.obj");
    this->getResourceManager()->addMesh("assets/models/MainSword.fbx", "assets/textures");

    this->getResourceManager()->addMesh("assets/models/grave.obj");
    this->getResourceManager()->addMesh("assets/models/alter.obj");
    this->getResourceManager()->addMesh("assets/models/hump.obj");

    this->getResourceManager()->addSound("assets/Sounds/Enemysounds/Golem/GolemTakeDmg.ogg");
    this->getResourceManager()->addSound("assets/Sounds/Enemysounds/Golem/Shockwave.ogg");
    this->getResourceManager()->addSound("assets/Sounds/Enemysounds/Golem/GolemCharge.ogg");
    this->getResourceManager()->addSound("assets/Sounds/EnemySounds/Lich/LichTakeDmg.ogg");
    this->getResourceManager()->addSound("assets/Sounds/EnemySounds/Lich/ChargeLightning.ogg");
    this->getResourceManager()->addSound("assets/Sounds/EnemySounds/Lich/ChargeFire.ogg");
    this->getResourceManager()->addSound("assets/Sounds/EnemySounds/Lich/ChargeIce.ogg");
    this->getResourceManager()->addSound("assets/Sounds/EnemySounds/Swarm/SwarmTakeDmg.ogg");
    this->getResourceManager()->addSound("assets/Sounds/EnemySounds/Swarm/SwarmAttack.ogg");
}

void LobbyScene::init()
{
    this->preloadAssets();

    this->playerModel = this->getResourceManager()->addAnimations(
        std::vector<std::string>(
            {"assets/models/Character/CharIdle.fbx",
                "assets/models/Character/CharRun2.fbx",
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
   
  TextureSamplerSettings samplerSettings{};
  samplerSettings.filterMode = vk::Filter::eNearest;
  samplerSettings.unnormalizedCoordinates = VK_TRUE;
    this->networkHandler = dynamic_cast<NetworkHandlerGame*>(this->getNetworkHandler());

    this->backgroundId =
        this->getResourceManager()->addTexture("assets/textures/blackTex.jpg"
        );
    this->buttonId =
        this->getResourceManager()->addTexture("assets/textures/UI/button.jpg");

    this->buttonSound = this->getResourceManager()->addSound("assets/Sounds/buttonClick.ogg");

    this->fontTextureId = Scene::getResourceManager()->addTexture(
        "assets/textures/UI/font.png", {samplerSettings, true}
    );
    Scene::getUIRenderer()->setBitmapFont(
        {"abcdefghij",
        "klmnopqrst",
        "uvwxyz+-.'",
        "0123456789",
        "!?,<>:()#^",
        "@%/       "},
        this->fontTextureId,
        glm::vec2(50,50)
    );

    int camEntity = this->createEntity();
    this->setComponent<Camera>(camEntity, 55.f);
    this->setMainCamera(camEntity);
    this->getComponent<Transform>(camEntity).position = glm::vec3(0, 0, -20);
    this->getComponent<Transform>(camEntity).rotation = glm::vec3(0, 0, 0);

    scene = this->createEntity();
    this->setComponent<MeshComponent>(scene, (int)this->getResourceManager()->addMesh("assets/models/Menu/lobby.obj"));
    Transform& t = this->getComponent<Transform>(scene);
    t.position = glm::vec3(0.f, 0.5f, 40.f);
    t.rotation = glm::vec3(0.f, 180.f, 0.f);

    addCandle(glm::vec3(8, 0, -24));
    addCandle(glm::vec3(-8, 0, -24));
    addCandle(glm::vec3(22, 0, -14.5));
    addCandle(glm::vec3(-22, 0, -14.5));
    addCandle(glm::vec3(26.5, 0, 0));
    addCandle(glm::vec3(-26, 0, 0));
    addCandle(glm::vec3(22, 0, 14.5));
    addCandle(glm::vec3(-22, 0, 14.5));
    addCandle(glm::vec3(8, 0, 26));
    addCandle(glm::vec3(-8, 0, 26));

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
    this->setActive(this->players[0]);

    startButton.position = glm::vec2(0.0f, -450.f);
    startButton.dimension = glm::vec2(275.0f, 100.0f);

    disconnectButton.position = glm::vec2(-800.f, -450.f);
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
    this->getComponent<DirectionalLight>(light).shadowMapMinBias = 0.002f;
    this->getComponent<DirectionalLight>(light).shadowMapAngleBias = 0.007f;
}

void LobbyScene::addCandle(glm::vec3 position) 
{
    Entity candle = this->createEntity();
    this->setComponent<MeshComponent>(candle, (int)this->getResourceManager()->addMesh("assets/models/Menu/candle.obj"));
    this->setComponent<PointLight>(candle, glm::vec3(0.5, -7, 26), glm::vec3(40, 10, 5));
    this->getComponent<Transform>(candle).position = position;

    // Fireflies particle system
    this->setComponent<ParticleSystem>(candle);
    ParticleSystem& firefliesPS = this->getComponent<ParticleSystem>(candle);
    firefliesPS.maxlifeTime = 2.0f;
    firefliesPS.numParticles = 4;
    firefliesPS.textureIndex = this->getResourceManager()->addTexture("assets/textures/firefliesParticle2.png");
    firefliesPS.startSize = glm::vec2(0.5f);
    firefliesPS.endSize = glm::vec2(0.0f);
    firefliesPS.startColor = glm::vec4(0.0f);
    firefliesPS.endColor = glm::vec4(1.0f);
    firefliesPS.velocityStrength = 0.7f;
    firefliesPS.acceleration = glm::vec3(0.0f);
    firefliesPS.spawnRate = 0.01f;
    firefliesPS.coneSpawnVolume.diskRadius = 1.5f;
    firefliesPS.coneSpawnVolume.coneAngle = 120.0f;
    firefliesPS.coneSpawnVolume.localDirection =
        glm::vec3(0.0f, 1.0f, 0.0f);
    firefliesPS.coneSpawnVolume.localPosition = glm::vec3(0.5f, -8.2f, 26.5f);
}

void LobbyScene::update()
{
    // Set model position and player names
    auto netPlayers = this->networkHandler->getPlayers();
    if (netPlayers.size() != this->activePlayers - 1)
    {
        this->playersNames.clear();
        this->activePlayers = (int)netPlayers.size() + 1;
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

    // Button backgrounds
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
        glm::vec2(200.0f)
    );

    for (int i = 0; i < this->playersNames.size(); i++)
    {
        this->getUIRenderer()->renderString(
            playersNames[i],
            this->POSITIONS[i + 1] + glm::vec3(0.0f, 20.0f, 0.0f),
            glm::vec2(200.0f)
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
            this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
            // Start singleplayer
            if (this->activePlayers == 1 && !Input::isKeyDown(Keys::M))
            {
               this->getNetworkHandler()->disconnectClient();
               this->getNetworkHandler()->deleteServer();
               this->getNetworkHandler()->setStatus(ServerStatus::WAITING);
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
    else if (!this->getNetworkHandler()->isConnected())
    {
        this->getNetworkHandler()->disconnectClient();
        this->getNetworkHandler()->deleteServer();
        this->switchScene(new MainMenu, "scripts/MainMenu.lua");
    }

    this->getUIRenderer()->renderString(
        "disconnect", this->disconnectButton.position, glm::vec2(25.0f), 0.0f, StringAlignment::CENTER
    );
    if (this->disconnectButton.isClicking()) {
        this->getAudioHandler()->playSound(this->getMainCameraID(), this->buttonSound);
        this->getNetworkHandler()->disconnectClient();
        this->getNetworkHandler()->deleteServer();
        this->getSceneHandler()->setScene(new MainMenu, "scripts/MainMenu.lua");
    }

    this->helpPacket.clear();
}

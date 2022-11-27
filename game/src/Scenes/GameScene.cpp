#include "GameScene.h"

#include "../Systems/AiCombatSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/HealthBarSystem.hpp"
#include "../Systems/HealSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "../Network/NetworkHandlerGame.h"
#include "GameOverScene.h"
#include "MainMenu.h"

#ifdef _CONSOLE
// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);
#endif

GameScene::GameScene() :
    playerID(-1), portal(-1), numRoomsCleared(0), newRoomFrame(false), perk(-1),
    perk1(-1), perk2(-1), perk3(-1), perk4(-1), ability(-1)
{
    Input::setHideCursor(true);
}

GameScene::~GameScene()
{
}

void GameScene::init()
{
    TextureSamplerSettings samplerSettings{};
    samplerSettings.filterMode = vk::Filter::eNearest;
    samplerSettings.unnormalizedCoordinates = VK_TRUE;

    int fontTextureId = Scene::getResourceManager()->addTexture(
        "assets/textures/UI/testBitmapFont.png", { samplerSettings, true });
    Scene::getUIRenderer()->setBitmapFont(
        { "abcdefghij",
         "klmnopqrst",
         "uvwxyz+-.'",
         "0123456789",
         "!?,<>:()#^",
         "@%        " },
        fontTextureId,
        glm::uvec2(16, 16));

    int swarm =
        this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");

    roomHandler.init(
        this,
        this->getResourceManager(), true);
    
    ResourceManager* resourceMng = this->getResourceManager();
    this->perkMeshes[0] = resourceMng->addMesh("assets/models/Perk_Hp.obj");
    this->perkMeshes[1] = resourceMng->addMesh("assets/models/Perk_Dmg.obj");
    this->perkMeshes[2] = resourceMng->addMesh("assets/models/Perk_AtkSpeed.obj");
    this->perkMeshes[3] = resourceMng->addMesh("assets/models/Perk_Movement.obj");
    this->perkMeshes[4] = resourceMng->addMesh("assets/models/Perk_Stamina.obj");

    this->abilityTextures[0] = resourceMng->addTexture("assets/textures/UI/knockbackAbility.png");
    this->abilityTextures[1] = resourceMng->addTexture("assets/textures/UI/knockbackAbility.png");
    this->abilityTextures[2] = resourceMng->addTexture("assets/textures/UI/empty.png");
    this->perkTextures[0] = resourceMng->addTexture("assets/textures/UI/hpUp.png");
    this->perkTextures[1] = resourceMng->addTexture("assets/textures/UI/dmgUp.png");
    this->perkTextures[2] = resourceMng->addTexture("assets/textures/UI/atkSpeedUp.png");
    this->perkTextures[3] = resourceMng->addTexture("assets/textures/UI/moveUp.png");
    this->perkTextures[4] = resourceMng->addTexture("assets/textures/UI/staminaUp.png");
    this->perkTextures[5] = resourceMng->addTexture("assets/textures/UI/empty.png");
    this->hpBarBackgroundTextureID = resourceMng->addTexture("assets/textures/UI/hpBarBackground.png");
    this->hpBarTextureID = resourceMng->addTexture("assets/textures/UI/hpBar.png");
    this->blackTextureIndex = resourceMng->addTexture("assets/textures/blackTex.png");

    // Temporary light
    this->dirLightEntity = this->createEntity();
    this->setComponent<DirectionalLight>(
        this->dirLightEntity,
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(0.6f)
        );
    DirectionalLight& dirLight = this->getComponent<DirectionalLight>(this->dirLightEntity);
    dirLight.cascadeSizes[0] = 0.044f;
    dirLight.cascadeSizes[1] = 0.149f;
    dirLight.cascadeSizes[2] = 1.0f;
    dirLight.cascadeDepthScale = 36.952f;
    dirLight.shadowMapMinBias = 0.00001f;
    dirLight.shadowMapAngleBias = 0.0004f;

    this->createSystem<HealthBarSystem>(
        this->hpBarBackgroundTextureID,
        this->hpBarTextureID,
        this,
        this->getUIRenderer()
        );
}

void GameScene::start()
{
    this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, "playerID");

    this->getAudioHandler()->setMusic("assets/Sounds/GameMusic.ogg");
    this->getAudioHandler()->setMasterVolume(0.5f);
    this->getAudioHandler()->setMusicVolume(0.2f);
    this->getAudioHandler()->playMusic();

    this->networkHandler = dynamic_cast<NetworkHandlerGame*>(this->getNetworkHandler());
    this->networkHandler->init();
    this->networkHandler->setPlayerEntity(playerID);
    this->networkHandler->createOtherPlayers(this->getComponent<MeshComponent>(playerID).meshID);

        //TODO : was it okay to make this in to start?
    if (networkHandler->isConnected())
    {
        //GET SEED FROM SERVER
        srand(69);
        roomHandler.generate(rand());
    }
    else
    {
        roomHandler.generate(rand());
    }
    
    createPortal();

    this->setComponent<Combat>(playerID);
    this->createSystem<CombatSystem>(
        this,
        this->getResourceManager(),
        this->playerID,
        &this->paused,
        this->getPhysicsEngine(),
        this->getUIRenderer(),
        this->getScriptHandler(),
        this->networkHandler
    );
    this->createSystem<HealSystem>(
        this->playerID,
        this
    );

    if (this->networkHandler->hasServer())
    {
        this->networkHandler->spawnItemRequest(healAbility, glm::vec3(50.0f, 10.0f, 0.0f));
        this->networkHandler->spawnItemRequest(hpUpPerk, 0.5f, glm::vec3(30.0f, 7.0f, 20.0f));
        this->networkHandler->spawnItemRequest(dmgUpPerk, 0.5f, glm::vec3(30.0f, 7.0f, -20.0f));
        this->networkHandler->spawnItemRequest(attackSpeedUpPerk, 0.5f, glm::vec3(30.0f, 7.0f, 0.0f));
        this->networkHandler->spawnItemRequest(movementUpPerk, 1.0f, glm::vec3(30.0f, 5.0f, -40.0f));
        this->networkHandler->spawnItemRequest(staminaUpPerk, 0.5f, glm::vec3(30.0f, 5.0f, -60.0f));
    }

    // Pause menu
    this->resumeButton.position = glm::vec2(0.0f, 100.0f);
    this->exitButton.position = glm::vec2(0.0f, -100.0f);
    this->resumeButton.dimension = glm::vec2(500.0f, 150.0f);
    this->exitButton.dimension = glm::vec2(500.0f, 150.0f);

    //if we are not multiplayer we do this by ourself
    if (!networkHandler->isConnected())
    {
        // Ai management
        this->aiHandler = this->getAIHandler();
        this->aiHandler->init(this->getSceneHandler());
    
        spawnHandler.init(&this->roomHandler, this, 
        this->getSceneHandler(),this->aiHandler,
        this->getResourceManager(),this->getUIRenderer());
    }
}

void GameScene::update()
{
    if (!networkHandler->isConnected())
     {   
        this->aiHandler->update(Time::getDT());

        // TODO: Move to SpawnHandler ---- 
        if (this->roomHandler.playerNewRoom(this->playerID, this->getPhysicsEngine()))
        {
            this->newRoomFrame = true;

            this->spawnHandler.spawnEnemiesIntoRoom();
        }
        // ---- TODO: Move to SpawnHandler ^^^^

        if (this->spawnHandler.allDead() && this->newRoomFrame)
        {
            this->newRoomFrame = false;

            // Call when a room is cleared
            this->roomHandler.roomCompleted();
            this->numRoomsCleared++;

            if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
            {
                this->getComponent<MeshComponent>(this->portal).meshID = this->portalOnMesh;
            }
        }

        // Switch scene if the player is dead
        if (this->hasComponents<Combat>(this->playerID))
        {
            if (this->getComponent<Combat>(this->playerID).health <= 0.0f)
            {
                this->switchScene(new GameOverScene(), "scripts/GameOverScene.lua");
            }
        }
    }
    else
    {
        if (this->roomHandler.playerNewRoom(this->playerID, this->getPhysicsEngine()))
        {
            this->newRoomFrame = true;
        }

         // Server is diconnected
        if (this->networkHandler->getStatus() == ServerStatus::DISCONNECTED)
        {
            this->networkHandler->disconnectClient();
            this->switchScene(new MainMenu(), "scripts/MainMenu.lua");
        }

        // If player is dead make the player not able to move
        // and server shall say if we shall switch scene
        if (this->hasComponents<Combat>(this->playerID))
        {
            if (this->getComponent<Combat>(this->playerID).health <= 0.0f)
            {
                this->networkHandler->disconnectClient(); // TEMP: probably will be in game over scene later
                this->switchScene(new GameOverScene(), "scripts/GameOverScene.lua");
            }
        }

        // Network
        this->networkHandler->updatePlayer();
        this->networkHandler->interpolatePositions();
    }

    Combat& playerCombat = this->getComponent<Combat>(this->playerID);
    this->getUIRenderer()->setTexture(
        abilityTextures[playerCombat.ability.abilityType]
    );
    this->getUIRenderer()->renderTexture(
        glm::vec2(890.f, -390.f), glm::vec2(100.0f)
    );

    float perkXPos = -720.f;
    float perkYPos = -500.f;
    for (size_t i = 0; i < 4; i++)
    {
        this->getUIRenderer()->setTexture(
            this->perkTextures[playerCombat.perks[i].perkType]
        );
        this->getUIRenderer()->renderTexture(
            glm::vec2(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f),
            glm::vec2(70.0f)
        );
    }

    // Render HP bar UI
    float hpPercent = 1.0f;
    float maxHpPercent = 1.0f;
    if (this->hasComponents<Combat>(this->playerID))
    {
        hpPercent = playerCombat.health * 0.01f;
        maxHpPercent = playerCombat.maxHealth * 0.01f;
    }
    float xPos = -720.f;
    float yPos = -500.f;
    float xSize = 1024.f * 0.35f;
    float ySize = 64.f * 0.35f;

    this->getUIRenderer()->setTexture(this->hpBarBackgroundTextureID);
    this->getUIRenderer()->renderTexture(
        glm::vec2(xPos - (1.0f - maxHpPercent) * xSize * 0.5f, yPos + 20.f),
        glm::vec2((xSize * maxHpPercent) + 10, ySize + 10)
    );
    this->getUIRenderer()->setTexture(this->hpBarTextureID);
    this->getUIRenderer()->renderTexture(
        glm::vec2(xPos - (1.0f - hpPercent) * xSize * 0.5f, yPos + 20.f),
        glm::vec2(xSize * hpPercent, ySize)
    );

    // Paused
    if (Input::isKeyPressed(Keys::ESC))
    {
        this->paused = !this->paused;
        this->getScriptHandler()->setGlobal(this->paused, "paused");
        Input::setHideCursor(!this->paused);
        this->getComponent<Rigidbody>(this->playerID).velocity = glm::vec3(0.0f);
    }
    if (this->paused)
    {
        this->getUIRenderer()->setTexture(this->blackTextureIndex);
        this->getUIRenderer()->renderTexture(glm::vec2(0.0f), glm::vec2(1920.0f, 1080.0f), glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
        this->getUIRenderer()->renderTexture(this->resumeButton.position, this->resumeButton.dimension);
        this->getUIRenderer()->renderTexture(this->exitButton.position, this->exitButton.dimension);
        this->getUIRenderer()->renderString("resume", this->resumeButton.position, glm::vec2(50.0f));
        this->getUIRenderer()->renderString("exit", this->exitButton.position, glm::vec2(50.0f));

        if (this->resumeButton.isClicking())
        {
            this->paused = false;
            this->getScriptHandler()->setGlobal(this->paused, "paused");
            Input::setHideCursor(!this->paused);
        }
        else if (this->exitButton.isClicking())
        {
            this->networkHandler->disconnectClient();
            this->networkHandler->deleteServer();
            this->switchScene(new MainMenu(), "scripts/MainMenu.lua");
        }
    }

#ifdef _CONSOLE

    // Cascades
   DirectionalLight& dirLight = this->getComponent<DirectionalLight>(this->dirLightEntity);
   ImGui::Begin("Shadows");
   ImGui::SliderFloat("Cascade size 0", &dirLight.cascadeSizes[0], 0.0f, 1.0f);
   ImGui::SliderFloat("Cascade size 1", &dirLight.cascadeSizes[1], 0.0f, 1.0f);
   ImGui::SliderFloat("Cascade size 2", &dirLight.cascadeSizes[2], 0.0f, 1.0f);
   ImGui::SliderFloat("Cascade depth", &dirLight.cascadeDepthScale, 1.0f, 50.0f);
   ImGui::Checkbox("Visualize cascades", &dirLight.cascadeVisualization);
   ImGui::SliderFloat("Shadow map angle bias", &dirLight.shadowMapAngleBias, 0.0f, 0.005f);
   ImGui::End();

    static bool renderDebug = false;
    if (ImGui::Begin("Debug"))
    {
        if (ImGui::Checkbox("Render debug shapes", &renderDebug))
        {
            this->getPhysicsEngine()->renderDebugShapes(renderDebug);
        }
        const glm::vec3& playerPos =
            this->getComponent<Transform>(playerID).position;
        ImGui::Text(
            "Player pos: (%d, %d, %d)",
            (int)playerPos.x,
            (int)playerPos.y,
            (int)playerPos.z
        );
        ImGui::Separator();
    }
    ImGui::End();

    this->roomHandler.imgui(this->getDebugRenderer());

    decreaseFps();
#endif

}


void GameScene::onTriggerStay(Entity e1, Entity e2)
{
	Entity player = e1 == this->playerID ? e1 : e2 == this->playerID ? e2 : -1;
	
	if (player == this->playerID) // player triggered a trigger :]
	{
		Entity other = e1 == player ? e2 : e1;
    

		if (other == this->portal && this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1) // -1 not counting start room
		{
			this->switchScene(new GameScene(), "scripts/gamescene.lua");
		}
	}
}

void GameScene::onTriggerEnter(Entity e1, Entity e2)
{
  Entity ground = e1 == this->roomHandler.getFloor()   ? e1
                  : e2 == this->roomHandler.getFloor() ? e2
                                                       : -1;
  Entity perk = this->hasComponents<Perks>(e1)   ? e1
                : this->hasComponents<Perks>(e2) ? e2
                                                 : -1;
  Entity ability = this->hasComponents<Abilities>(e1)   ? e1
                   : this->hasComponents<Abilities>(e2) ? e2
                                                        : -1;
   
	if(this->hasComponents<SwarmComponent>(e1) && this->hasComponents<SwarmComponent>(e2))
	{
		SwarmComponent& s1 = this->getComponent<SwarmComponent>(e1);
		SwarmComponent& s2 = this->getComponent<SwarmComponent>(e2);
		s1.touchedFriend = true;
		s1.friendTouched = this->getComponent<Transform>(e2).position;
		s2.touchedFriend = true;
		s2.friendTouched = this->getComponent<Transform>(e1).position;
	}
	else if(this->hasComponents<SwarmComponent>(e1) && this->hasComponents<TankComponent>(e2))
	{
		SwarmComponent& s1 = this->getComponent<SwarmComponent>(e1);
		TankComponent& s2 = this->getComponent<TankComponent>(e2);
		s1.touchedFriend = true;
		s1.friendTouched = this->getComponent<Transform>(e2).position;
	}
	else if(this->hasComponents<SwarmComponent>(e1) && this->hasComponents<LichComponent>(e2))
	{
		SwarmComponent& s1 = this->getComponent<SwarmComponent>(e1);
		TankComponent& s2 = this->getComponent<TankComponent>(e2);
		s1.touchedFriend = true;
		s1.friendTouched = this->getComponent<Transform>(e2).position;
	}
	else if(this->hasComponents<SwarmComponent>(e2) && this->hasComponents<TankComponent>(e1))
	{
		SwarmComponent& s2 = this->getComponent<SwarmComponent>(e2);
		TankComponent& s1 = this->getComponent<TankComponent>(e1);
		s2.touchedFriend = true;
		s2.friendTouched = this->getComponent<Transform>(e1).position;
	}
	else if(this->hasComponents<SwarmComponent>(e2) && this->hasComponents<LichComponent>(e1))
	{
		SwarmComponent& s2 = this->getComponent<SwarmComponent>(e2);
		TankComponent& s1 = this->getComponent<TankComponent>(e1);
		s2.touchedFriend = true;
		s2.friendTouched = this->getComponent<Transform>(e1).position;
	}

    if (this->entityValid(ground))
    {
        if (this->entityValid(perk))
        {
            this->removeComponent<Rigidbody>(perk);
            Transform& perkTrans = this->getComponent<Transform>(perk);
            perkTrans.position.y = 6.f;
            this->setScriptComponent(perk, "scripts/spin.lua");
        }
        else if (this->entityValid(ability))
        {
            this->removeComponent<Rigidbody>(ability);
            Transform& abilityTrans = this->getComponent<Transform>(ability);
            abilityTrans.position.y = 4.f;
        }
    }
}

void GameScene::onCollisionEnter(Entity e1, Entity e2)
{
  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
  {
    this->getComponent<SwarmComponent>(e1).touchedFriend = true;
    this->getComponent<SwarmComponent>(e2).touchedFriend = true;
  }

}

void GameScene::onCollisionStay(Entity e1, Entity e2)
{
    Entity player = e1 == playerID ? e1 : e2 == playerID ? e2 : -1;

  if (player == playerID)  // player triggered a trigger :]
  {
    Entity other = e1 == player ? e2 : e1;
    if (this->hasComponents<SwarmComponent>(other))
    {
      auto& swarmComp = this->getComponent<SwarmComponent>(other);
      if (swarmComp.inAttack)
        {
          auto& aiCombat = this->getComponent<AiCombatSwarm>(other);
          swarmComp.inAttack = false;
          swarmComp.touchedPlayer = true;
          //aiCombat.timer = aiCombat.lightAttackTime;
          this->getComponent<Combat>(player).health -=
              (int)aiCombat.lightHit;
            
          Log::write("WAS HIT", BT_FILTER);
        }
    }
    else if (this->hasComponents<TankComponent>(other))
    {
      auto& tankComp = this->getComponent<TankComponent>(other);
      if (tankComp.canAttack)
      {
        auto& aiCombat = this->getComponent<AiCombatTank>(other);
        tankComp.canAttack = false;
        this->getComponent<Combat>(player).health -=
            (int)aiCombat.directHit;
            
        Log::write("WAS HIT", BT_FILTER);
      }
    }
  }

  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
  {
    this->getComponent<SwarmComponent>(e1).touchedFriend = true;
    this->getComponent<SwarmComponent>(e2).touchedFriend = true;
  }
}

void GameScene::onCollisionExit(Entity e1, Entity e2)
{

  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
  {
    this->getComponent<SwarmComponent>(e1).touchedFriend = false;
    this->getComponent<SwarmComponent>(e2).touchedFriend = false;
  }

}

void GameScene::createPortal()
{
    glm::vec3 portalTriggerDims(6.f, 18.f, 1.f);
    glm::vec3 portalBlockDims(3.f, 18.f, 3.f);

    portalOffMesh =
        this->getResourceManager()->addMesh("assets/models/PortalOff.obj");
    portalOnMesh =
        this->getResourceManager()->addMesh("assets/models/PortalOn.obj");

    portal = this->createEntity();
    this->getComponent<Transform>(portal).position =
        this->roomHandler.getExitRoom().position;
    this->setComponent<Collider>(
        portal, Collider::createBox(portalTriggerDims, glm::vec3(0, 0, 0), true)
        );

    this->setComponent<MeshComponent>(portal);
    this->getComponent<MeshComponent>(portal).meshID = portalOffMesh;

    Entity collider1 = this->createEntity();
    this->getComponent<Transform>(collider1).position =
        this->getComponent<Transform>(portal).position;
    this->getComponent<Transform>(collider1).position.x += 9.f;
    this->getComponent<Transform>(collider1).position.y += 9.f;
    this->setComponent<Collider>(collider1, Collider::createBox(portalBlockDims));

    Entity collider2 = this->createEntity();
    this->getComponent<Transform>(collider2).position =
        this->getComponent<Transform>(portal).position;
    this->getComponent<Transform>(collider2).position.x -= 9.f;
    this->getComponent<Transform>(collider2).position.y += 9.f;
    this->setComponent<Collider>(collider2, Collider::createBox(portalBlockDims));
}

#ifdef _CONSOLE
void decreaseFps()
{
    static double result = 1234567890.0;

    static int num = 0;
    if (ImGui::Begin("Debug"))
    {
        ImGui::Text("Fps %f", 1.f / Time::getDT());
        ImGui::InputInt("Loops", &num);
    }
    ImGui::End();

    for (int i = 0; i < num; i++)
    {
        result /= std::sqrt(heavyFunction(result));
        result /= std::sqrt(heavyFunction(result));
        result /= std::sqrt(heavyFunction(result));
        result /= std::sqrt(heavyFunction(result));
        result /= std::sqrt(heavyFunction(result));
    }
}

double heavyFunction(double value)
{
    double result = 1234567890.0;
    for (size_t i = 0; i < 3000; i++)
    {
        result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 0.892375892))));
        result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 1.476352734))));
        result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 2.248923885))));
        result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 3.691284908))));
        result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 3.376598278))));
    }

    return result;
}
#endif
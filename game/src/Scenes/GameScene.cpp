#include "GameScene.h"

#include "../Systems/AiCombatSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/HealthBarSystem.hpp"
#include "../Systems/HealSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "../Systems/ParticleRemoveEntity.hpp"
#include "../Systems/ParticleRemoveComponent.hpp"
#include "../Network/NetworkHandlerGame.h"
#include "vengine/application/Time.hpp"
#include "GameOverScene.h"
#include "MainMenu.h"

#ifdef _CONSOLE
// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);
#endif

void GameScene::testParticleSystem(const Entity& particleSystemEntity)
{
    // Used for testing particle systems
    #ifdef _CONSOLE
        if (this->entityValid(particleSystemEntity))
        {
            ParticleSystem& particleSystem = this->getComponent<ParticleSystem>(particleSystemEntity);

            ImGui::Begin("Particle System");
            ImGui::SliderFloat3("Cone pos: ", &particleSystem.coneSpawnVolume.localPosition[0], -5.0f, 5.0f);
            ImGui::SliderFloat3("Cone dir: ", &particleSystem.coneSpawnVolume.localDirection[0], -1.0f, 1.0f);
            ImGui::SliderFloat("Disk radius: ", &particleSystem.coneSpawnVolume.diskRadius, 0.0f, 10.0f);
            ImGui::SliderFloat("Cone angle: ", &particleSystem.coneSpawnVolume.coneAngle, 0.0f, 180.0f);
            ImGui::SliderFloat("Velocity strength: ", &particleSystem.velocityStrength, 0.0f, 50.0f);
            ImGui::SliderFloat("Spawn rate: ", &particleSystem.spawnRate, 0.0f, 1.0f);
            ImGui::Checkbox("Spawn: ", &particleSystem.spawn);
            ImGui::End();

            this->getDebugRenderer()->renderParticleSystemCone(particleSystemEntity);
        }
    #endif
}

void GameScene::setCurrentLevel(const GameSceneLevel& lvl) {
    this->getComponent<HealthComp>(playerID).health = lvl.hp;
    this->getComponent<Combat>(playerID).ability = lvl.ability;
    ((NetworkHandlerGame*)this->getNetworkHandler())->setPerks(lvl.perks);
}

GameSceneLevel GameScene::setNewLevel() {
    GameSceneLevel theReturn;

    theReturn.level = currentLevel.level + 1;
    theReturn.hp = this->getComponent<HealthComp>(playerID).health;
    Combat &c = this->getComponent<Combat>(playerID);
    for (int i = 0; i < 4; i++)
    {
        theReturn.perks[i] = this->getComponent<Combat>(playerID).perks[i];

    }
    theReturn.ability = this->getComponent<Combat>(playerID).ability;
    
    return theReturn;
}

GameScene::GameScene(GameSceneLevel gameSceneLevel) :
    playerID(-1), portal(-1), numRoomsCleared(0), newRoomFrame(false), perk(-1),
    perk1(-1), perk2(-1), perk3(-1), perk4(-1), ability(-1), ability1(-1), 
    deathTimer(2.f), isDead(false)
{
    Input::setHideCursor(true);
    currentLevel = gameSceneLevel;
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
        "assets/textures/UI/font.png", { samplerSettings, true });
    Scene::getUIRenderer()->setBitmapFont(
        { "abcdefghij",
         "klmnopqrst",
         "uvwxyz+-.'",
         "0123456789",
         "!?,<>:()#^",
         "@%        " },
        fontTextureId,
        glm::uvec2(50, 50));

    int swarm =
        this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");

    roomHandler.init(
        this,
        this->getResourceManager(), true);
    
    ResourceManager* resourceMng = this->getResourceManager();
    this->abilityMeshes[0] = resourceMng->addMesh("assets/models/KnockbackAbility.obj");
    this->abilityMeshes[1] = resourceMng->addMesh("assets/models/Ability_Healing.obj");
    this->perkMeshes[0] = resourceMng->addMesh("assets/models/Perk_Hp.obj");
    this->perkMeshes[1] = resourceMng->addMesh("assets/models/Perk_Dmg.obj");
    this->perkMeshes[2] = resourceMng->addMesh("assets/models/Perk_AtkSpeed.obj");
    this->perkMeshes[3] = resourceMng->addMesh("assets/models/Perk_Movement.obj");
    this->perkMeshes[4] = resourceMng->addMesh("assets/models/Perk_Stamina.obj");

    this->abilityTextures[0] = resourceMng->addTexture("assets/textures/UI/knockbackAbility.png");
    this->abilityTextures[1] = resourceMng->addTexture("assets/textures/UI/HealingAbility.png");
    this->abilityTextures[2] = resourceMng->addTexture("assets/textures/UI/empty.png");
    this->perkTextures[0] = resourceMng->addTexture("assets/textures/UI/hpUp.png");
    this->perkTextures[1] = resourceMng->addTexture("assets/textures/UI/dmgUp.png");
    this->perkTextures[2] = resourceMng->addTexture("assets/textures/UI/atkSpeedUp.png");
    this->perkTextures[3] = resourceMng->addTexture("assets/textures/UI/moveUp.png");
    this->perkTextures[4] = resourceMng->addTexture("assets/textures/UI/staminaUp.png");
    this->perkTextures[5] = resourceMng->addTexture("assets/textures/UI/empty.png");
    this->hpBarBackgroundTextureID = resourceMng->addTexture("assets/textures/UI/hpBarBackground.png");
    this->hpBarTextureID = resourceMng->addTexture("assets/textures/UI/hpBar.png");

    // Temporary light
    this->dirLightEntity = this->createEntity();
    this->setComponent<DirectionalLight>(
        this->dirLightEntity,
        glm::vec3(0.35f, -1.0f, 0.35f),
        glm::vec3(0.3f)
        );
    DirectionalLight& dirLight = this->getComponent<DirectionalLight>(this->dirLightEntity);
    dirLight.cascadeSizes[0] = 45.0f;
    dirLight.cascadeSizes[1] = 120.0f;
    dirLight.cascadeSizes[2] = 356.0f;
    dirLight.cascadeDepthScale = 36.952f;
    dirLight.shadowMapMinBias = 0.00001f;
    dirLight.shadowMapAngleBias = 0.0004f;
}

void GameScene::start()
{
    this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, "playerID");

    this->networkHandler = dynamic_cast<NetworkHandlerGame*>(this->getNetworkHandler());
    this->networkHandler->init();
    this->networkHandler->setPlayerEntity(playerID);
    this->networkHandler->createOtherPlayers(this->getComponent<MeshComponent>(playerID).meshID);

    if (networkHandler->isConnected())
    {
        int seed = this->networkHandler->getSeed();
        Log::write("Seed from server: " + std::to_string(seed));
        roomHandler.generate(seed);
        networkHandler->setRoomHandler(roomHandler, this->numRoomsCleared);
    }
    else
    {
        roomHandler.generate(rand());
    }
    
    createPortal();

    this->setComponent<HealthComp>(playerID);
    this->setComponent<Combat>(playerID);
    this->createSystem<CombatSystem>(
        this->getSceneHandler(),
        this->playerID,
        &this->paused,
        this->networkHandler
    );
    this->createSystem<HealSystem>(
        this->playerID,
        this
    );
    this->createSystem<HealthBarSystem>(
        this->hpBarBackgroundTextureID,
        this->hpBarTextureID,
        this,
        this->getUIRenderer()
        );
    this->createSystem<ParticleRemoveEntity>(this);
    this->createSystem<ParticleRemoveComponent>(this);

    if (this->networkHandler->hasServer() || !this->networkHandler->isConnected())
    {
        this->networkHandler->spawnItemRequest(knockbackAbility, glm::vec3(50.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.25f, 0.0f));
        this->networkHandler->spawnItemRequest(hpUpPerk, 0.5f, glm::vec3(30.0f, 7.0f, 20.0f), glm::vec3(0.0f, 0.25f, 0.0f));
        this->networkHandler->spawnItemRequest(dmgUpPerk, 0.5f, glm::vec3(30.0f, 7.0f, -20.0f), glm::vec3(0.0f, 0.25f, 0.0f));
        this->networkHandler->spawnItemRequest(attackSpeedUpPerk, 0.5f, glm::vec3(30.0f, 7.0f, 0.0f), glm::vec3(0.0f, 0.25f, 0.0f));
        this->networkHandler->spawnItemRequest(movementUpPerk, 1.0f, glm::vec3(30.0f, 5.0f, -40.0f), glm::vec3(0.0f, 0.25f, 0.0f));
        this->networkHandler->spawnItemRequest(staminaUpPerk, 0.5f, glm::vec3(30.0f, 5.0f, -60.0f), glm::vec3(0.0f, 0.25f, 0.0f));
    }

    this->levelString = "level " + std::to_string(currentLevel.level);

    // Pause menu
    this->resumeButton.position = glm::vec2(0.0f, 100.0f);
    this->exitButton.position = glm::vec2(0.0f, -100.0f);
    this->resumeButton.dimension = glm::vec2(500.0f, 100.0f);
    this->exitButton.dimension = glm::vec2(500.0f, 100.0f);

    this->getAudioHandler()->setMusic("assets/Sounds/GameMusic.ogg");
    this->getAudioHandler()->setMasterVolume(0.5f);
    this->getAudioHandler()->setMusicVolume(0.0f);
    this->getAudioHandler()->playMusic();
	
    // If we are not multiplayer we do this by ourself
    if (!networkHandler->isConnected())
    {
        // Ai management
        this->aiHandler = this->getAIHandler();
        this->aiHandler->init(this->getSceneHandler());
    
        spawnHandler.init(&this->roomHandler, this,
        this->getSceneHandler(),this->aiHandler,
        this->getResourceManager(),this->getUIRenderer());
    }
	
    this->createSystem<OrbSystem>(this->getSceneHandler());
	
    // Create particle systems for this scene
    ((NetworkHandlerGame*)this->getNetworkHandler())->initParticleSystems();

    this->setCurrentLevel(currentLevel);
}

void GameScene::update()
{
    ((NetworkHandlerGame*)this->getNetworkHandler())->deleteInitialParticleSystems();

    if (!networkHandler->isConnected() && networkHandler->getStatus() == ServerStatus::WAITING)
    {   
        this->aiHandler->update(Time::getDT());

        if (this->roomHandler.playerNewRoom(this->playerID, this->getPhysicsEngine()))
        {
            this->newRoomFrame = true;
            this->timeWhenEnteredRoom = Time::getTimeSinceStart();
            this->safetyCleanDone = false;

            this->spawnHandler.spawnEnemiesIntoRoom();
        }
        if(!this->safetyCleanDone)
        {
            
            if(this->timeWhenEnteredRoom + delayToSafetyDelete < Time::getTimeSinceStart())
            {
                this->spawnHandler.killAllEnemiesOutsideRoom();
                this->safetyCleanDone = true;
            }
        }
        if (this->spawnHandler.allDead() && this->newRoomFrame)
        {
            this->newRoomFrame = false;
            // Call when a room is cleared
            this->roomHandler.roomCompleted();
            this->numRoomsCleared++;

            if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
            {
                this->getComponent<MeshComponent>(this->portal).meshID = this->portalOnMesh;

                // Particle effects
                this->setComponent<ParticleSystem>(this->portal);
                //this->getComponent<ParticleSystem>(this->portal) = this->portalParticleSystemSide0.getParticleSystem();
                this->getComponent<ParticleSystem>(this->portal) = ((NetworkHandlerGame*)this->getNetworkHandler())->getPortalParticleSystem0();

                Entity side1Entity = this->createEntity();
                this->getComponent<Transform>(side1Entity) = this->getComponent<Transform>(this->portal);
                this->setComponent<ParticleSystem>(side1Entity);
                this->getComponent<ParticleSystem>(side1Entity) = ((NetworkHandlerGame*)this->getNetworkHandler())->getPortalParticleSystem1();
            }
        }
        // Switch scene if the player is dead
        if (this->hasComponents<Combat>(this->playerID))
        {
            Script& playerScript = this->getComponent<Script>(this->playerID);
            int tempHealth = this->getComponent<HealthComp>(this->playerID).health;
            if (tempHealth <= 0.0f && !this->isDead)
            {
                this->isDead = true;
                this->getScriptHandler()->setScriptComponentValue(playerScript, this->isDead, "isDead");
            }
            else if (this->isDead)
            {
                if (this->deathTimer >= 0.f)
                {
                    this->deathTimer -= Time::getDT();
                    int currentAnim = -1;
                    this->getScriptHandler()->getScriptComponentValue(playerScript, currentAnim, "currentAnimation");
                    if (currentAnim != 7)
                    {
                        this->getScriptHandler()->setScriptComponentValue(playerScript, tempHealth, "currentHealth");
                    }
                }
                else
                {
                    this->switchScene(new GameOverScene(), "scripts/GameOverScene.lua");
                }
            }
        }
        this->spawnHandler.updateImgui();
        this->imguiUpdate();
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
        if (this->hasComponents<HealthComp>(this->playerID))
        {
            if (this->getComponent<HealthComp>(this->playerID).health <= 0.0f)
            {
                this->networkHandler->disconnectClient(); // TEMP: probably will be in game over scene later
                this->switchScene(new GameOverScene(), "scripts/GameOverScene.lua");
            }
        }
        if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
        {
            this->getComponent<MeshComponent>(this->portal).meshID = this->portalOnMesh;
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
        glm::vec2(-875.f, -455.f), glm::vec2(113.0f)
    );

    float perkXPos = -720.f;
    float perkYPos = -500.f;
    for (size_t i = 0; i < 4; i++)
    {
        this->getUIRenderer()->setTexture(
            this->perkTextures[playerCombat.perks[i].perkType]
        );
        this->getUIRenderer()->renderTexture(
            glm::vec2(-perkXPos - 142.f + i * 83.f, perkYPos + 25.f),
            glm::vec2(76.0f)
        );
    }
    // Render Level
    this->getUIRenderer()->renderString(this->levelString, glm::vec2(-750, 500), glm::vec2(60,60));

    // Render HP bar UI
    HealthComp& playerHealth = this->getComponent<HealthComp>(this->playerID);
    float hpPercent = playerHealth.health * 0.01f;
    float maxHpPercent = playerHealth.maxHealth * 0.01f;
    float xPos = -600.f;
    float yPos = -472.f;
    float xSize = 1200.f * 0.35f;
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
   ImGui::Begin("Directional light");
   ImGui::SliderFloat("XZ direction", &dirLight.direction.x, 0.0f, 1.0f );
   ImGui::SliderFloat("Cascade size 0", &dirLight.cascadeSizes[0], 0.0f, 1000.0f);
   ImGui::SliderFloat("Cascade size 1", &dirLight.cascadeSizes[1], 0.0f, 1000.0f);
   ImGui::SliderFloat("Cascade size 2", &dirLight.cascadeSizes[2], 0.0f, 1000.0f);
   ImGui::SliderFloat("Cascade depth", &dirLight.cascadeDepthScale, 1.0f, 50.0f);
   ImGui::Checkbox("Visualize cascades", &dirLight.cascadeVisualization);
   ImGui::SliderFloat("Shadow map angle bias", &dirLight.shadowMapAngleBias, 0.0f, 0.005f);
   ImGui::End();
   dirLight.direction.z = dirLight.direction.x;

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
    
        if (!networkHandler->isConnected())
        {
		    if (other == this->portal && this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1) // -1 not counting start room            
		    {
                networkHandler->cleanUp();
		    	this->switchScene(new GameScene(this->setNewLevel()), "scripts/gamescene.lua");
		    }
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
            abilityTrans.position.y = 8.f;
            this->setScriptComponent(ability, "scripts/spin.lua");
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
          swarmComp.inAttack = false;
          swarmComp.touchedPlayer = true;
          //aiCombat.timer = aiCombat.lightAttackTime;
          HealthComp& playerHealth = this->getComponent<HealthComp>(player);
          playerHealth.health -=
              (int)swarmComp.lightHit;
          playerHealth.srcDmgEntity = other;
            
          Log::write("WAS HIT", BT_FILTER);
        }
    }
    else if (this->hasComponents<TankComponent>(other))
    {
      auto& tankComp = this->getComponent<TankComponent>(other);
      if (tankComp.canAttack)
      {
        tankComp.canAttack = false;
        HealthComp& playerHealth = this->getComponent<HealthComp>(player);
        playerHealth.health -=
            (int)tankComp.directHit;
        playerHealth.srcDmgEntity = other;
            
        Log::write("WAS HIT", BT_FILTER);
      }
    }
    else if (this->hasComponents<Orb>(other)) 
    {
        auto& orb = this->getComponent<Orb>(other);
        HealthComp& playerHealth = this->getComponent<HealthComp>(player);
        playerHealth.health -=
            orb.orbPower->damage;
        playerHealth.srcDmgEntity = other;
        orb.onCollision(other, this->getSceneHandler());
    }
  }
  else 
  { // Collision between two things that isnt player
    
    if(this->hasComponents<Orb>(e1) || this->hasComponents<Orb>(e2))
    {
        Entity collidingOrb = this->hasComponents<Orb>(e1) ? e1 : e2; 
        
        auto& orb = this->getComponent<Orb>(collidingOrb);        
        orb.onCollision(collidingOrb, this->getSceneHandler());
        
    }
  }

    //Swarm collides with swarm
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

void GameScene::imguiUpdate()
{
    ImGui::Begin("Game Scene");
    std::string playerString = "playerID";
    int playerID;
    getScriptHandler()->getGlobal(playerID, playerString);
    auto& playerHealthComp = getComponent<HealthComp>(playerID);
    if(ImGui::Button("INVINCIBLE Player")){
        playerHealthComp.health = INT_MAX;
    }
    if(ImGui::Button("Kill Player")){
        playerHealthComp.health = 0; 
    }

    ImGui::End();
}

void GameScene::createPortal()
{
    glm::vec3 portalTriggerDims(6.f, 18.f, 1.f);
    glm::vec3 portalBlockDims(3.f, 18.f, 3.f);

    portalOffMesh = this->getResourceManager()->addMesh("assets/models/PortalOff.obj");
    portalOnMesh = this->getResourceManager()->addMesh("assets/models/PortalOn.obj");

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
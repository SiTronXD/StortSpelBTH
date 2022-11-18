#include "GameScene.h"

#include "../Systems/AiCombatSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/HealthBarSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "GameOverScene.h"

#ifdef _CONSOLE
// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);
#endif

GameScene::GameScene() :
    playerID(-1), portal(-1), numRoomsCleared(0), newRoomFrame(false), perk(-1),
    perk1(-1), perk2(-1), perk3(-1), perk4(-1), ability(-1)
{
}

GameScene::~GameScene()
{
    for (auto& p : swarmGroups)
    {
        delete p;
    }
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
        this->getResourceManager(),
        this->getConfigValue<int>("room_size"),
        this->getConfigValue<int>("tile_types"));
    roomHandler.generate();
    createPortal();
    // simon
    ResourceManager* resourceMng = this->getResourceManager();
    this->perkMeshes[0] = resourceMng->addMesh("assets/models/Perk_Hp.obj");
    this->perkMeshes[1] = resourceMng->addMesh("assets/models/Perk_Dmg.obj");
    this->perkMeshes[2] = resourceMng->addMesh("assets/models/Perk_AtkSpeed.obj");
    this->perkMeshes[3] = resourceMng->addMesh("assets/models/Perk_Movement.obj");
    this->perkMeshes[4] = resourceMng->addMesh("assets/models/Perk_Stamina.obj");

    this->abilityTextures[0] =
        resourceMng->addTexture("assets/textures/UI/knockbackAbility.png");
    this->abilityTextures[1] =
        resourceMng->addTexture("assets/textures/UI/knockbackAbility.png");
    this->abilityTextures[2] = resourceMng->addTexture("assets/textures/UI/empty.png");
    this->perkTextures[0] = resourceMng->addTexture("assets/textures/UI/hpUp.png");
    this->perkTextures[1] = resourceMng->addTexture("assets/textures/UI/dmgUp.png");
    this->perkTextures[2] = resourceMng->addTexture("assets/textures/UI/atkSpeedUp.png");
    this->perkTextures[3] = resourceMng->addTexture("assets/textures/UI/moveUp.png");
    this->perkTextures[4] = resourceMng->addTexture("assets/textures/UI/staminaUp.png");
    this->perkTextures[5] = resourceMng->addTexture("assets/textures/UI/empty.png");
    this->hpBarBackgroundTextureID =
        resourceMng->addTexture("assets/textures/UI/hpBarBackground.png");
    this->hpBarTextureID =
        resourceMng->addTexture("assets/textures/UI/hpBar.png");

  // Temporary light
  Entity directionalLightEntity = this->createEntity();
  this->setComponent<DirectionalLight>(
      directionalLightEntity, 
      glm::vec3(-1.0f, -1.0f, -1.0f), 
      glm::vec3(0.6f)
  );
  DirectionalLight& dirLight = this->getComponent<DirectionalLight>(directionalLightEntity);
  dirLight.shadowMapFrustumHalfWidth = 200.0f;
  dirLight.shadowMapFrustumHalfHeight = 200.0f;
  dirLight.shadowMapFrustumDepth = 800.0f;
  dirLight.shadowMapMinBias = 0.0001f;
  dirLight.shadowMapAngleBias = 0.001f;

    this->createSystem<HealthBarSystem>(
        this->hpBarBackgroundTextureID,
        this->hpBarTextureID,
        this,
        this->getUIRenderer()
        );
}

void GameScene::start()
{
    std::string playerName = "playerID";
    this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);

    this->getAudioHandler()->setMusic("assets/Sounds/GameMusic.ogg");
    this->getAudioHandler()->setMasterVolume(0.5f);
    this->getAudioHandler()->setMusicVolume(1.f);
    this->getAudioHandler()->playMusic();

    this->setComponent<Combat>(playerID);
    this->createSystem<CombatSystem>(
        this,
        this->getResourceManager(),
        this->playerID,
        this->getPhysicsEngine(),
        this->getUIRenderer(),
        this->getScriptHandler());

    this->ability = this->createEntity();
    int knockback =
        this->getResourceManager()->addMesh("assets/models/KnockbackAbility.obj");
    this->setComponent<MeshComponent>(this->ability, knockback);
    Transform& abilityTrans = this->getComponent<Transform>(this->ability);
    abilityTrans.position = glm::vec3(50.f, 10.f, 0.f);
    abilityTrans.scale = glm::vec3(4.f, 4.f, 4.f);
    this->setComponent<Collider>(
        this->ability, Collider::createSphere(4.f, glm::vec3(0), true));
    this->setComponent<Abilities>(this->ability, healAbility);
    this->setComponent<PointLight>(this->ability, { glm::vec3(0.f), glm::vec3(7.f, 9.f, 5.f) });

    this->perk = this->createEntity();
    this->setComponent<MeshComponent>(this->perk, this->perkMeshes[hpUpPerk]);
    Transform& perkTrans = this->getComponent<Transform>(this->perk);
    perkTrans.position = glm::vec3(30.f, 7.f, 20.f);
    perkTrans.scale = glm::vec3(3.f, 3.f, 3.f);
    this->setComponent<Collider>(
        this->perk, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
    this->setComponent<PointLight>(this->perk, { glm::vec3(0.f), glm::vec3(5.f, 7.f, 9.f) });
    this->setComponent<Perks>(this->perk);
    Perks& perkSetting = this->getComponent<Perks>(this->perk);
    perkSetting.multiplier = 0.5f;
    perkSetting.perkType = hpUpPerk;

    this->perk1 = this->createEntity();
    this->setComponent<MeshComponent>(this->perk1, this->perkMeshes[dmgUpPerk]);
    Transform& perkTrans1 = this->getComponent<Transform>(this->perk1);
    perkTrans1.position = glm::vec3(30.f, 7.f, -20.f);
    perkTrans1.scale = glm::vec3(3.f, 3.f, 3.f);
    this->setComponent<Collider>(
        this->perk1, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
    this->setComponent<PointLight>(this->perk1, { glm::vec3(0.f), glm::vec3(5.f, 7.f, 9.f) });
    this->setComponent<Perks>(this->perk1);
    Perks& perkSetting1 = this->getComponent<Perks>(this->perk1);
    perkSetting1.multiplier = 0.5f;
    perkSetting1.perkType = dmgUpPerk;
    this->setScriptComponent(this->perk1, "scripts/spin.lua");

    this->perk2 = this->createEntity();
    this->setComponent<MeshComponent>(this->perk2, this->perkMeshes[attackSpeedUpPerk]);
    Transform& perkTrans2 = this->getComponent<Transform>(this->perk2);
    perkTrans2.position = glm::vec3(30.f, 7.f, 0.f);
    perkTrans2.scale = glm::vec3(3.f, 3.f, 3.f);
    this->setComponent<Collider>(
        this->perk2, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
    this->setComponent<PointLight>(this->perk2, { glm::vec3(0.f), glm::vec3(5.f, 7.f, 9.f) });
    this->setComponent<Perks>(this->perk2);
    Perks& perkSetting2 = this->getComponent<Perks>(this->perk2);
    perkSetting2.multiplier = 0.5f;
    perkSetting2.perkType = attackSpeedUpPerk;
    this->setScriptComponent(this->perk2, "scripts/spin.lua");

    this->perk3 = this->createEntity();
    this->setComponent<MeshComponent>(this->perk3, this->perkMeshes[movementUpPerk]);
    Transform& perkTrans3 = this->getComponent<Transform>(this->perk3);
    perkTrans3.position = glm::vec3(30.f, 5.f, -40.f);
    perkTrans3.scale = glm::vec3(3.f, 3.f, 3.f);
    this->setComponent<Collider>(
        this->perk3, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
    this->setComponent<PointLight>(this->perk3, { glm::vec3(0.f), glm::vec3(5.f, 7.f, 9.f) });
    this->setComponent<Perks>(this->perk3);
    Perks& perkSetting3 = this->getComponent<Perks>(this->perk3);
    perkSetting3.multiplier = 1.f;
    perkSetting3.perkType = movementUpPerk;
    this->setScriptComponent(this->perk3, "scripts/spin.lua");

    this->perk4 = this->createEntity();
    this->setComponent<MeshComponent>(this->perk4, this->perkMeshes[staminaUpPerk]);
    Transform& perkTrans4 = this->getComponent<Transform>(this->perk4);
    perkTrans4.position = glm::vec3(30.f, 5.f, -60.f);
    perkTrans4.scale = glm::vec3(3.f, 3.f, 3.f);
    this->setComponent<Collider>(
        this->perk4, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
    this->setComponent<PointLight>(this->perk4, { glm::vec3(0.f), glm::vec3(5.f, 7.f, 9.f) });
    this->setComponent<Perks>(this->perk4);
    Perks& perkSetting4 = this->getComponent<Perks>(this->perk4);
    perkSetting4.multiplier = 0.5f;
    perkSetting4.perkType = staminaUpPerk;
    this->setScriptComponent(this->perk4, "scripts/spin.lua");

    // Ai management
    this->aiHandler = this->getAIHandler();
    this->aiHandler->init(this->getSceneHandler());

    aiExample();
}

void GameScene::update()
{
    this->aiHandler->update(Time::getDT());

    if (allDead() && this->newRoomFrame)
    {
        this->newRoomFrame = false;

        // Call when a room is cleared
        roomHandler.roomCompleted();
        this->numRoomsCleared++;

        if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
        {
            this->getComponent<MeshComponent>(portal).meshID = portalOnMesh;
        }
    }

    /*if (this->hasComponents<Collider, Rigidbody>(this->playerID))
      {
          Rigidbody& rb = this->getComponent<Rigidbody>(this->playerID);
          glm::vec3 vec = glm::vec3(Input::isKeyDown(Keys::LEFT) - Input::isKeyDown(Keys::RIGHT), 0.0f, Input::isKeyDown(Keys::UP) - Input::isKeyDown(Keys::DOWN));
          float y = rb.velocity.y;
          rb.velocity = vec * 10.0f;
          rb.velocity.y = y + Input::isKeyPressed(Keys::SPACE) * 5.0f;
      }*/

      // Switch scene if the player is dead
    if (this->hasComponents<Combat>(this->playerID))
    {
        if (this->getComponent<Combat>(this->playerID).health <= 0.0f)
        {
            this->switchScene(new GameOverScene(), "scripts/GameOverScene.lua");
        }
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

#ifdef _CONSOLE

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

    roomHandler.imgui();

    decreaseFps();
#endif
}

void GameScene::aiExample() 
{



	auto a = [&](FSM* fsm, uint32_t entityId) -> void {
		SwarmFSM* swarmFSM = (SwarmFSM*)fsm;
    
        auto entityImguiWindow = [&](SwarmFSM* swarmFsm, uint32_t entityId)->void 
        {
            auto& entitySwarmComponent = this->getSceneHandler()->getScene()->getComponent<SwarmComponent>(entityId);
            auto& entityAiCombatComponent = this->getSceneHandler()->getScene()->getComponent<AiCombatSwarm>(entityId);
            auto& entiyFSMAgentComp = this->getSceneHandler()->getScene()->getComponent<FSMAgentComponent>(entityId);
            auto& entityRigidBody = this->getSceneHandler()->getScene()->getComponent<Rigidbody>(entityId);
            int& health            = entitySwarmComponent.life;
			float& jumpForce		=entitySwarmComponent.jumpForce;
			float& jumpForceY		=entitySwarmComponent.jumpY;
            float& speed           = entitySwarmComponent.speed;
            float& attackRange     = entitySwarmComponent.attackRange;
            float& sightRange      = entitySwarmComponent.sightRadius;
            bool& inCombat         = entitySwarmComponent.inCombat;
            float& attackPerSec    = entityAiCombatComponent.lightAttackTime;
            float& lightAttackDmg  = entityAiCombatComponent.lightHit;
			float& gravity 			= entityRigidBody.gravityMult;
            std::string& status    = entiyFSMAgentComp.currentNode->status;   
            ImGui::Text(status.c_str());
            ImGui::SliderInt("health", &health, 0, 100);
            ImGui::SliderFloat("speed", &speed, 0, 100);
            ImGui::SliderFloat("jumpForce", &jumpForce, 0, 100);
            ImGui::SliderFloat("jumpForceY", &jumpForceY, 0, 100);
            ImGui::SliderFloat("gravity", &gravity, 0, 10);
            ImGui::SliderFloat("attackRange", &attackRange, 0, 100);
            ImGui::SliderFloat("sightRange", &sightRange, 0, 100);		
            ImGui::InputFloat("attack/s", &attackPerSec);		
            ImGui::InputFloat("lightattackDmg", &lightAttackDmg);		 
            ImGui::Checkbox("inCombat", &inCombat);		            
        };
        //TEMP             

        static bool showEntityId = false;
        ImGui::Checkbox("Show Entity ID", &showEntityId);
        if(showEntityId)
        {
            
            // Show all entity ID over entitties             
            glm::vec3 entityPos3 =this->getSceneHandler()->getScene()->getComponent<Transform>(entityId).position;
            glm::vec4 entityPos4 = glm::vec4(entityPos3, 1.f);

            auto screenPos = this->getMainCamera()->projection * this->getMainCamera()->view * entityPos4;
            glm::vec3 realScreenPos; 
            realScreenPos.x = (screenPos.x / screenPos.w) * 1920/2;
            realScreenPos.y = (screenPos.y / screenPos.w) * 1080/2;
            realScreenPos.z = screenPos.z / screenPos.w;

            Scene::getUIRenderer()->setTexture(this->fontTextureIndex);
            //Scene::getUIRenderer()->renderString(std::to_string(entityId), realScreenPos.x, realScreenPos.y, 20, 20); 
            Scene::getUIRenderer()->renderString(std::to_string(entityId), entityPos3, glm::vec2(20, 20)); 
        }    

        if(ImGui::Button("SWITCHsCENE")){
            this->switchScene(new GameScene(), "scripts/gamescene.lua");            
        }
        std::string playerString = "playerID";
        int playerID;
        this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerString);
        auto& playerCombat = this->getSceneHandler()->getScene()->getComponent<Combat>(playerID);
        if(ImGui::Button("Kill Player")){
            playerCombat.health = 0; 
        }
        if(ImGui::Button("INVINCIBLE Player")){
            playerCombat.health = INT_MAX; 
        }
		ImGui::Separator();
		ImGui::Separator();
		entityImguiWindow(swarmFSM, entityId);

        auto& entitySwarmComponent = this->getSceneHandler()->getScene()->getComponent<SwarmComponent>(entityId);
        auto& entityAiCombatComponent = this->getSceneHandler()->getScene()->getComponent<AiCombatSwarm>(entityId);
        auto& entiyFSMAgentComp = this->getSceneHandler()->getScene()->getComponent<FSMAgentComponent>(entityId);

        std::string groupName = "GroupMembers["+std::to_string(entitySwarmComponent.group->myId)+"]";
        if(ImGui::TreeNode(groupName.c_str()))
        {
            if(ImGui::Button("Kill All")){
                entitySwarmComponent.life = 0; 
                for(auto& ent : entitySwarmComponent.group->members)
                {              
                    auto& entSwarmComp = this->getSceneHandler()->getScene()->getComponent<SwarmComponent>(ent);                    
                    entSwarmComp.life = 0; 
                }
            }

            static int selected_friend = -1; 

            for(auto& ent : entitySwarmComponent.group->members)
            {              
                std::string entityName = "entity["+std::to_string(ent)+"]";
                if(ImGui::Button(entityName.c_str())){selected_friend = ent;}
            }
            if(selected_friend != -1)
            {
                std::string entityName = "entity["+std::to_string(selected_friend)+"]";
                ImGui::Begin((entityName + "_popup").c_str());
                entityImguiWindow(swarmFSM, selected_friend);
                ImGui::End();
            }            
            ImGui::TreePop();
        }       
        
	};
	auto b = [&](FSM* fsm, uint32_t entityId) -> void {
		TankFSM* tankFSM = (TankFSM*)fsm;
    
        auto entityImguiWindow = [&](TankFSM* tankFsm, uint32_t entityId)->void 
        {
            auto& entityTankComponent	= this->getSceneHandler()->getScene()->getComponent<TankComponent>(entityId);
            auto& entiyFSMAgentComp		= this->getSceneHandler()->getScene()->getComponent<FSMAgentComponent>(entityId);
            auto& entityRigidBody		= this->getSceneHandler()->getScene()->getComponent<Rigidbody>(entityId);
            int& health					= entityTankComponent.life;
            std::string fis				= "Friends in sight: "+std::to_string(entityTankComponent.friendsInSight.size());
            std::string af				= "All friends alive: "+std::to_string(entityTankComponent.allFriends.size());
			float& gravity 				= entityRigidBody.gravityMult;
			float& humpForce 			= entityTankComponent.humpForce;
			float& humpYForce 			= entityTankComponent.humpYForce;
			float& humpShockwaveSpeed 	= entityTankComponent.humpShockwaveSpeed;
			float& humpSpeed 			= entityTankComponent.groundHumpTimerOrig;
            std::string& status			= entiyFSMAgentComp.currentNode->status;   
            ImGui::Text(status.c_str());
            ImGui::Text(fis.c_str());
            ImGui::Text(af.c_str());
            ImGui::SliderInt("health",                  &health,               0,      entityTankComponent.FULL_HEALTH);
            ImGui::SliderFloat("humpForce",             &humpForce,            0.0f,   200.0f);
            ImGui::SliderFloat("humpYForce",            &humpYForce,           0.0f,   200.0f);
            ImGui::SliderFloat("humpShockwaveSpeed",    &humpShockwaveSpeed,   0.0f,   200.0f);
            ImGui::SliderFloat("humpSpeed",             &humpSpeed,            0.1f,   10.0f);
            ImGui::SliderFloat("gravity",               &gravity,              0,      10);
        };
        //TEMP             

        static bool showEntityId = false;
        ImGui::Checkbox("Show Entity ID", &showEntityId);
        if(showEntityId)
        {
            
            // Show all entity ID over entitties             
            glm::vec4 entityPos = glm::vec4(this->getSceneHandler()->getScene()->getComponent<Transform>(entityId).position, 1.f);

            auto screenPos = this->getMainCamera()->projection * this->getMainCamera()->view * entityPos;
            glm::vec3 realScreenPos; 
            realScreenPos.x = (screenPos.x / screenPos.w) * 1920/2;
            realScreenPos.y = (screenPos.y / screenPos.w) * 1080/2;
            realScreenPos.z = screenPos.z / screenPos.w;

            Scene::getUIRenderer()->setTexture(this->fontTextureIndex);
            //Scene::getUIRenderer()->renderString(std::to_string(entityId), realScreenPos.x, realScreenPos.y, 20, 20); 
            Scene::getUIRenderer()->renderString(std::to_string(entityId), glm::vec3(entityPos), glm::vec2(20, 20)); 
        }    

		entityImguiWindow(tankFSM, entityId);
        
	};

	//SWARM
	static SwarmFSM swarmFSM;
	this->aiHandler->addFSM(&swarmFSM, "swarmFSM");
	
//TODO: Cause crash on second run, therefore disabled in distribution... 
#ifdef _CONSOLE 
    this->aiHandler->addImguiToFSM("swarmFSM", a);
#endif 

	int swarm = this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
    int numOfGroups = 4;
	int group_size = 3;
    for(size_t j = 0; j < numOfGroups; j++)
    {
        this->swarmGroups.push_back(new SwarmGroup);
        for (size_t i = 0; i < group_size; i++)
        {
            this->swarmIDs.push_back(this->createEntity());
            this->setComponent<MeshComponent>(this->swarmIDs.back(), swarm);
            this->setComponent<AiCombatSwarm>(this->swarmIDs.back());
            this->setComponent<Collider>(this->swarmIDs.back(), Collider::createSphere(4.0f));
            this->setComponent<Rigidbody>(this->swarmIDs.back());
			Rigidbody& rb = this->getComponent<Rigidbody>(this->swarmIDs.back());
            rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
			rb.gravityMult = 5.0f;
			rb.friction = 1.5f;
            this->aiHandler->createAIEntity(this->swarmIDs.back(), "swarmFSM");
            this->swarmGroups.back()->members.push_back(this->swarmIDs.back());
            this->setInactive(this->swarmIDs.back());
            this->getSceneHandler()->getScene()->getComponent<SwarmComponent>(this->swarmIDs.back()).group = this->swarmGroups.back();
            SwarmComponent& swarmComp = this->getComponent<SwarmComponent>(this->swarmIDs.back());
            swarmComp.life = 0;
        }
    }

	//TANK
	static TankFSM tankFSM;
	this->aiHandler->addFSM(&tankFSM, "tankFSM");
#ifdef _CONSOLE 
    this->aiHandler->addImguiToFSM("tankFSM", b);
#endif 
	for(int i = 0; i < 1; i++)
	{
		int tank = this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
		this->tankIDs.push_back(this->createEntity());
		this->setComponent<MeshComponent>(this->tankIDs.back(), tank);
		this->setComponent<AiCombatTank>(this->tankIDs.back());
		this->setComponent<Rigidbody>(this->tankIDs.back());
		Rigidbody& rb = this->getComponent<Rigidbody>(this->tankIDs.back());
		rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
		rb.gravityMult = 5.0f;
		rb.friction = 3.0f;
		rb.mass = 10.0f;
		Transform& transform = this->getComponent<Transform>(this->tankIDs.back());
		transform.scale = glm::vec3(3.0f, 3.0f, 3.0f);
		this->setComponent<Collider>(this->tankIDs.back(), Collider::createSphere(4.0f*transform.scale.x));
		this->aiHandler->createAIEntity(this->tankIDs.back(), "tankFSM");
        TankComponent& tankComp = this->getComponent<TankComponent>(this->tankIDs.back());
        tankComp.origScaleY = transform.scale.y;
		this->setInactive(this->tankIDs.back());
	}
	//stnky LICH
	static LichFSM lichFSM;
	this->aiHandler->addFSM(&lichFSM, "lichFSM");
	for(int i = 0; i < 1; i++)
	{
		int lich = this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
		this->lichIDs.push_back(this->createEntity());
		this->setComponent<MeshComponent>(this->lichIDs.back(), lich);
		this->setComponent<AiCombatLich>(this->lichIDs.back());
		this->setComponent<Rigidbody>(this->lichIDs.back());
		Rigidbody& rb = this->getComponent<Rigidbody>(this->lichIDs.back());
		rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
		rb.gravityMult = 5.0f;
		rb.friction = 3.0f;
		rb.mass = 10.0f;
		Transform& transform = this->getComponent<Transform>(this->lichIDs.back());
		transform.scale = glm::vec3(1.0f, 3.0f, 1.0f);
		this->setComponent<Collider>(this->lichIDs.back(), Collider::createCapsule(4.0f, 4.0f*transform.scale.y));
		this->aiHandler->createAIEntity(this->lichIDs.back(), "lichFSM");
		this->setInactive(this->lichIDs.back());
	}
	
}

bool GameScene::allDead()
{
	bool ret = true;
	for(auto p: swarmIDs)
	{
		if(this->isActive(p))
		{
			ret = false;
			break;
		}
	}
	if(ret)
	{
		for(auto p: tankIDs)
		{
			if(this->isActive(p))
			{
				ret = false;
				break;
			}
		}
		if(ret)
		{
			for(auto p: lichIDs)
			{
				if(this->isActive(p))
				{
					ret = false;
					break;
				}
			}
		}
		
	}
	
	return ret;
}

void GameScene::onTriggerStay(Entity e1, Entity e2)
{
	Entity player = e1 == playerID ? e1 : e2 == playerID ? e2 : -1;
	
	if (player == playerID) // player triggered a trigger :]
	{
		Entity other = e1 == player ? e2 : e1;
		if (roomHandler.onPlayerTrigger(other))
		{
			this->newRoomFrame = true;

            //Num to spawn
            int numTanks        = 1;
            int numLich         = 0;
            int numSwarm        = 3;

			int swarmIdx        = 0;
			int lichIdx         = 0;
			int tankIdx         = 0;
			int randNumEnemies  = 10;
			int counter         = 0;
			const std::vector<Entity>& tiles = roomHandler.getFreeTiles();
			for (Entity tile : tiles)
			{
				if (randNumEnemies - counter != 0)
				{
					
					if(tankIdx < numTanks)
					{
						this->setActive(this->tankIDs[tankIdx]);
						Transform& transform = this->getComponent<Transform>(this->tankIDs[tankIdx]);
						Transform& tileTrans = this->getComponent<Transform>(tile);
						float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
						transform.position = tileTrans.position;
						transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

                        //Reset
                        TankComponent& tankComp = this->getComponent<TankComponent>(this->tankIDs[tankIdx]);
                        tankComp.life = tankComp.FULL_HEALTH;
                        transform.scale.y = tankComp.origScaleY;

						tankIdx++;
					}
					else if(lichIdx < numLich)
					{
						this->setActive(this->lichIDs[lichIdx]);
						Transform& transform = this->getComponent<Transform>(this->lichIDs[lichIdx]);
						Transform& tileTrans = this->getComponent<Transform>(tile);
						float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
						transform.position = tileTrans.position;
						transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

                        //Reset
                        LichComponent& lichComp = this->getComponent<LichComponent>(this->lichIDs[tankIdx]);
                        lichComp.life = lichComp.FULL_HEALTH;

						lichIdx++;
					}
					else if(swarmIdx < numSwarm)
					{
						this->setActive(this->swarmIDs[swarmIdx]);
						Transform& transform = this->getComponent<Transform>(this->swarmIDs[swarmIdx]);
						Transform& tileTrans = this->getComponent<Transform>(tile);
						float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
						transform.position = tileTrans.position;
						transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

						//Temporary enemie reset
						SwarmComponent& swarmComp = this->getComponent<SwarmComponent>(this->swarmIDs[swarmIdx]);
						transform.scale.y = 1.0f;
						swarmComp.life = swarmComp.FULL_HEALTH;
						swarmComp.group->inCombat = false;
					
						swarmComp.group->aliveMembers.push(0); 

						swarmIdx++;
					}
					
					counter++;
				
				}
			}

			for(SwarmGroup* group: this->swarmGroups)
			{
					//Set idle mid pos
					group->idleMidPos = glm::vec3(0.0f, 0.0f, 0.0f);
					int numAlive = 0;
					for(auto b: group->members)
					{
						if(isActive(b) && this->getComponent<SwarmComponent>(b).life > 0)
						{
							group->idleMidPos += this->getComponent<Transform>(b).position;
							numAlive++;
						}
					}
					group->idleMidPos /= numAlive;
					//Set ilde radius
					for(auto b: group->members)
					{
						if(isActive(b) && this->getComponent<SwarmComponent>(b).life > 0)
						{
							float len = glm::length(group->idleMidPos - this->getComponent<Transform>(b).position);
							if(len > group->idleRadius)
							{
								group->idleRadius = len;
							}
						}
					}
					//Set move to
					for(auto b: group->members)
					{
						SwarmComponent& swarmComp = this->getComponent<SwarmComponent>(b);
						swarmComp.idleMoveTo = group->idleMidPos;
						glm::vec3 dir = glm::normalize(glm::vec3(rand() * (rand() % 2 == 0 ? - 1 : 1), 0.0f, rand() * (rand() % 2 == 0 ? - 1 : 1)));
						swarmComp.idleMoveTo = swarmComp.group->idleMidPos + dir * swarmComp.group->idleRadius;
					}
					for(auto t: tankIDs)
			        {
			        	TankComponent& tankComp = this->getComponent<TankComponent>(t);
			        	tankComp.setFriends(this, t);
			        }
			}
		}        

		if (other == portal && numRoomsCleared >= this->roomHandler.getNumRooms() - 1) // -1 not counting start room
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

    //Entity swarm = this->hasComponents<SwarmComponent>(e1) ? e1 : this->hasComponents<SwarmComponent>(e2) ? e2 : -1;

    //if (e1 == this->swordID && swarm != -1)
    //{
    //}
    //else if (e2 == this->swordID && swarm != -1)
    //{

    //}

    if (this->entityValid(ground))
    {
        if (this->entityValid(perk))
        {
            this->removeComponent<Rigidbody>(perk);
            Transform& perkTrans = this->getComponent<Transform>(perk);
            perkTrans.position.y = 2.f;
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
          std::cout << "WAS HIT\n";
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
        std::cout << "WAS HIT\n";
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
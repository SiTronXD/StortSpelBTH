#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/AiCombatSystem.hpp"
#include "GameOverScene.h"

#ifdef _CONSOLE
// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);
#endif

GameScene::GameScene() 
	:playerID(-1), portal(-1), numRoomsCleared(0), newRoomFrame(false), perk(-1)
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
	int swarm = this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
	
	roomHandler.init(this, this->getResourceManager(), this->getConfigValue<int>("room_size"), this->getConfigValue<int>("tile_types"));
	roomHandler.generate();
	createPortal();

	perkTextures[0] = 
		this->getResourceManager()->addTexture("assets/textures/UI/hpUp.png");
	perkTextures[1] = 
		this->getResourceManager()->addTexture("assets/textures/UI/dmgUp.png");
	perkTextures[2] = 
		this->getResourceManager()->addTexture("assets/textures/UI/atkSpeedUp.png");
	perkTextures[3] = 
		this->getResourceManager()->addTexture("assets/textures/UI/empty.png");
	this->hpBarBackgroundTextureID =
		this->getResourceManager()->addTexture("assets/textures/UI/hpBarBackground.png");
	this->hpBarTextureID = 
		this->getResourceManager()->addTexture("assets/textures/UI/hpBar.png");

    // Add textures for ui renderer
	TextureSamplerSettings samplerSettings{};
	samplerSettings.filterMode = vk::Filter::eNearest;	
	samplerSettings.unnormalizedCoordinates = VK_TRUE;
    this->fontTextureIndex = Scene::getResourceManager()->addTexture("assets/textures/testBitmapFont.png", { samplerSettings, true });

	Scene::getUIRenderer()->setBitmapFont(
		{
			"abcdefghij",
			"klmnopqrst",
			"uvwxyz+-.'",
			"0123456789",
			"!?,<>:()#^",
			"@         "
		},
		fontTextureIndex,
		16, 16
	);
}

void GameScene::start()
{
	std::string playerName = "playerID";
	this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);
	
	this->setComponent<Combat>(playerID);
	this->createSystem<CombatSystem>(this, this->playerID, this->getPhysicsEngine(), this->getDebugRenderer());

	this->perk = this->createEntity();
	int perkHp = this->getResourceManager()->addMesh("assets/models/Perk_Hp.obj");
	this->setComponent<MeshComponent>(this->perk, perkHp);
	Transform& perkTrans = this->getComponent<Transform>(this->perk);
	perkTrans.position = glm::vec3(30.f, 5.f, 20.f);
	perkTrans.scale = glm::vec3(2.f, 2.f, 2.f);
    this->setComponent<Collider>(
        this->perk, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true)
    );
	this->setComponent<Perks>(this->perk);
	Perks& perkSetting = this->getComponent<Perks>(this->perk);
	perkSetting.multiplier = 1.f;
	perkSetting.perkType = hpUp;

	this->perk1 = this->createEntity();
	int perkDmg = this->getResourceManager()->addMesh("assets/models/Perk_Dmg.obj");
	this->setComponent<MeshComponent>(this->perk1, perkDmg);
	Transform& perkTrans1 = this->getComponent<Transform>(this->perk1);
	perkTrans1.position = glm::vec3(30.f, 5.f, -20.f);
	perkTrans1.scale = glm::vec3(2.f, 2.f, 2.f);
    this->setComponent<Collider>(
        this->perk1, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true)
    );
	this->setComponent<Perks>(this->perk1);
	Perks& perkSetting1 = this->getComponent<Perks>(this->perk1);
	perkSetting1.multiplier = 1.f;
	perkSetting1.perkType = dmgUp;

	this->perk2 = this->createEntity();
	int perkAtkSpeed = this->getResourceManager()->addMesh("assets/models/Perk_AtkSpeed.obj");
	this->setComponent<MeshComponent>(this->perk2, perkAtkSpeed);
	Transform& perkTrans2 = this->getComponent<Transform>(this->perk2);
	perkTrans2.position = glm::vec3(30.f, 5.f, 0.f);
	perkTrans2.scale = glm::vec3(2.f, 2.f, 2.f);
    this->setComponent<Collider>(
        this->perk2, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true)
    );
	this->setComponent<Perks>(this->perk2);
	Perks& perkSetting2 = this->getComponent<Perks>(this->perk2);
	perkSetting2.multiplier = 1.f;
	perkSetting2.perkType = attackSpeedUp;

    // Ai management 
    this->aiHandler = this->getAIHandler();
	this->aiHandler->init(this->getSceneHandler());
    
	aiExample();
}

void GameScene::update()
{
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
	float perkXPos = -720.f;
	float perkYPos = -500.f;
	for (size_t i = 0; i < 4; i++)
	{
		switch (playerCombat.perks[i].perkType)
		{
		case hpUp:
			this->getUIRenderer()->setTexture(perkTextures[hpUp]);
			this->getUIRenderer()->renderTexture(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f, 70.f, 70.f);
			break;
		case dmgUp:
			this->getUIRenderer()->setTexture(perkTextures[dmgUp]);
			this->getUIRenderer()->renderTexture(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f, 70.f, 70.f);
			break;
		case attackSpeedUp:
			this->getUIRenderer()->setTexture(perkTextures[attackSpeedUp]);
			this->getUIRenderer()->renderTexture(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f, 70.f, 70.f);
			break;
		case empty:
			this->getUIRenderer()->setTexture(perkTextures[empty]);
			this->getUIRenderer()->renderTexture(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f, 70.f, 70.f);
			break;
		}
	}

	// Render HP bar UI
	float hpPercent = 1.0f;
	float maxHpPercent = 1.0f;
	if (this->hasComponents<Combat>(this->playerID))
	{
		hpPercent = 
			playerCombat.health * 0.01f;
		maxHpPercent =
			playerCombat.maxHealth * 0.01f;

	}
	float xPos = -720.f;
	float yPos = -500.f;
	float xSize = 1024.f * 0.35f;
	float ySize = 64.f * 0.35f;

	this->getUIRenderer()->setTexture(this->hpBarBackgroundTextureID);
	this->getUIRenderer()->renderTexture(xPos - (1.0f - maxHpPercent) * xSize * 0.5f, yPos + 20.f, (xSize * maxHpPercent) + 10, ySize + 10);
	this->getUIRenderer()->setTexture(this->hpBarTextureID);
	this->getUIRenderer()->renderTexture(xPos - (1.0f - hpPercent) * xSize * 0.5f, yPos + 20.f, xSize * hpPercent, ySize);
	
#ifdef _CONSOLE

	static bool renderDebug = false;
	if (ImGui::Begin("Debug"))
	{
		if (ImGui::Checkbox("Render debug shapes", &renderDebug))
		{
			this->getPhysicsEngine()->renderDebugShapes(renderDebug);
		}
		const glm::vec3& playerPos = this->getComponent<Transform>(playerID).position;
		ImGui::Text("Player pos: (%d, %d, %d)", (int)playerPos.x, (int)playerPos.y, (int)playerPos.z);
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
            glm::vec4 entityPos = glm::vec4(this->getSceneHandler()->getScene()->getComponent<Transform>(entityId).position, 1.f);

            auto screenPos = this->getMainCamera()->projection * this->getMainCamera()->view * entityPos;
            glm::vec3 realScreenPos; 
            realScreenPos.x = (screenPos.x / screenPos.w) * 1920/2;
            realScreenPos.y = (screenPos.y / screenPos.w) * 1080/2;
            realScreenPos.z = screenPos.z / screenPos.w;

            Scene::getUIRenderer()->setTexture(this->fontTextureIndex);
            Scene::getUIRenderer()->renderString(std::to_string(entityId), realScreenPos.x, realScreenPos.y, 20, 20); 
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

			int swarmIdx = 0;
			int lichIdx = 0;
			int tankIdx = 0;
			int randNumEnemies = 10;
			int counter = 0;
			const std::vector<Entity>& tiles = roomHandler.getFreeTiles();
			for (Entity tile : tiles)
			{
				if (randNumEnemies - counter != 0)
				{
					
					if(tankIdx < 1)
					{
						this->setActive(this->tankIDs[tankIdx]);
						Transform& transform = this->getComponent<Transform>(this->tankIDs[tankIdx]);
						Transform& tileTrans = this->getComponent<Transform>(tile);
						float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
						transform.position = tileTrans.position;
						transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

						tankIdx++;
					}
					else if(lichIdx < 0)
					{
						this->setActive(this->lichIDs[lichIdx]);
						Transform& transform = this->getComponent<Transform>(this->lichIDs[lichIdx]);
						Transform& tileTrans = this->getComponent<Transform>(tile);
						float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
						transform.position = tileTrans.position;
						transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

						lichIdx++;
					}
					else if(swarmIdx < 0)
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
					
			}
		}        

		if (other == portal && numRoomsCleared >= this->roomHandler.getNumRooms() - 1) // -1 not counting start room
		{
			this->switchScene(new GameScene(), "scripts/gamescene.lua");
		}
	}
}

void GameScene::onCollisionEnter(Entity e1, Entity e2)
{
   
	if(this->hasComponents<SwarmComponent>(e1) && this->hasComponents<SwarmComponent>(e2))
	{
		SwarmComponent& s1 = this->getComponent<SwarmComponent>(e1);
		SwarmComponent& s2 = this->getComponent<SwarmComponent>(e2);
		s1.touchedFriend = true;
		s1.friendTouched = this->getComponent<Transform>(e2).position;
		s2.touchedFriend = true;
		s2.friendTouched = this->getComponent<Transform>(e1).position;
	}

}

void GameScene::onCollisionStay(Entity e1, Entity e2)
{
    Entity player = e1 == playerID ? e1 : e2 == playerID ? e2 : -1;
	
	if (player == playerID) // player triggered a trigger :]
	{
		Entity other = e1 == player ? e2 : e1;
        if(this->hasComponents<SwarmComponent>(other))
        {
            auto& swarmComp = this->getComponent<SwarmComponent>(other);
            if(swarmComp.inAttack)
            {
                auto& aiCombat = this->getComponent<AiCombatSwarm>(other);
                swarmComp.inAttack = false; 
                swarmComp.touchedPlayer = true; 
				//aiCombat.timer = aiCombat.lightAttackTime;
                this->getComponent<Combat>(player).health -= (int)aiCombat.lightHit;
                std::cout << "WAS HIT\n";
                
            }            
        }
    }
}

void GameScene::onCollisionExit(Entity e1, Entity e2)
{
   
	if(this->hasComponents<SwarmComponent>(e1) && this->hasComponents<SwarmComponent>(e2))
	{
		this->getComponent<SwarmComponent>(e1).touchedFriend = false;
		this->getComponent<SwarmComponent>(e2).touchedFriend = false;
	}

}

void GameScene::createPortal()
{
	glm::vec3 portalTriggerDims(6.f, 18.f, 1.f);
	glm::vec3 portalBlockDims(3.f, 18.f, 3.f);

	portalOffMesh = this->getResourceManager()->addMesh("assets/models/PortalOff.obj");
	portalOnMesh = this->getResourceManager()->addMesh("assets/models/PortalOn.obj");

	portal = this->createEntity();
	this->getComponent<Transform>(portal).position = this->roomHandler.getExitRoom().position;
    this->setComponent<Collider>(
        portal, Collider::createBox(portalTriggerDims, glm::vec3(0, 0, 0), true)
    );

	this->setComponent<MeshComponent>(portal);
	this->getComponent<MeshComponent>(portal).meshID = portalOffMesh;

	Entity collider1 = this->createEntity();
	this->getComponent<Transform>(collider1).position = this->getComponent<Transform>(portal).position;
	this->getComponent<Transform>(collider1).position.x += 9.f;
	this->getComponent<Transform>(collider1).position.y += 9.f;
	this->setComponent<Collider>(collider1, Collider::createBox(portalBlockDims));

	Entity collider2 = this->createEntity();
	this->getComponent<Transform>(collider2).position = this->getComponent<Transform>(portal).position;
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

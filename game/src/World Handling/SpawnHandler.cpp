#include "SpawnHandler.hpp"
#include <functional>

void SpawnHandler::spawnEnemiesIntoRoom()
{
    int swarmIdx        = 0;
    int lichIdx         = 0;
    int tankIdx         = 0;
    int randNumEnemies  = 10; //TODO: Make random when not debugging
    int counter         = 0;
    
    const std::vector<glm::vec3>& tiles = this->roomHandler->getFreeTiles();
    for (const glm::vec3& tilePos : tiles)
    {
        if (randNumEnemies - counter != 0)
        {
            
            if(tankIdx < nrOfTanks)
            {
                this->spawnTank(tankIdx, tilePos);
                tankIdx++;
            }
            else if(lichIdx < nrOfLichs)
            {
                this->spawnLich(lichIdx, tilePos);
                lichIdx++;
            }
            else if(swarmIdx < nrOfSwarms)
            {
                this->spawnSwarm(swarmIdx, tilePos);                
                swarmIdx++;
            }        
            counter++;        
        }
    }
}

void SpawnHandler::spawnTank(const int tankIdx, const glm::vec3& pos)
{
    currScene->setActive(this->tankIDs[tankIdx]);
    Transform& transform = currScene->getComponent<Transform>(this->tankIDs[tankIdx]);
    
    float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
    transform.position = pos + glm::vec3(tileWidth, 0.f, tileWidth);

    //Reset
    TankComponent& tankComp = currScene->getComponent<TankComponent>(this->tankIDs[tankIdx]);
    tankComp.life = tankComp.FULL_HEALTH;
    transform.scale.y = tankComp.origScaleY;
}

void SpawnHandler::spawnLich(int lichIdx, const glm::vec3& pos)
{
    currScene->setActive(this->lichIDs[lichIdx]);
    Transform& transform = currScene->getComponent<Transform>(this->lichIDs[lichIdx]);
    float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
    transform.position = pos + glm::vec3(tileWidth, 0.f, tileWidth);

    //Reset
    LichComponent& lichComp = currScene->getComponent<LichComponent>(this->lichIDs[lichIdx]);
    lichComp.life = lichComp.FULL_HEALTH;
    transform.scale.y = lichComp.origScaleY;
}

void SpawnHandler::spawnSwarm(int swarmIdx, const glm::vec3& pos)
{
    currScene->setActive(this->swarmIDs[swarmIdx]);
    Transform& transform = currScene->getComponent<Transform>(this->swarmIDs[swarmIdx]);
    float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;

    transform.position = pos + glm::vec3(tileWidth, 0.f, tileWidth);

    //Temporary enemie reset
    SwarmComponent& swarmComp = currScene->getComponent<SwarmComponent>(this->swarmIDs[swarmIdx]);
    transform.scale.y = 1.0f;
    swarmComp.life = swarmComp.FULL_HEALTH;
    swarmComp.group->inCombat = false;

    swarmComp.group->aliveMembers.push(0); 
}


void SpawnHandler::createEntities() 
{
    //SWARM
    static SwarmFSM swarmFSM;
    this->aiHandler->addFSM(&swarmFSM, "swarmFSM");

    static LichFSM lichFSM;
    this->aiHandler->addFSM(&lichFSM, "lichFSM");
    
    static TankFSM tankFSM;
    this->aiHandler->addFSM(&tankFSM, "tankFSM");

    //TODO: Cause crash on second run, therefore disabled in distribution... 
#ifdef _CONSOLE 
    if (dynamic_cast<NetworkScene*>(currScene) == nullptr)
    {
        this->aiHandler->addImguiToFSM("swarmFSM", this->SwarmImgui());
        this->aiHandler->addImguiToFSM("lichFSM", this->LichImgui());
        this->aiHandler->addImguiToFSM("tankFSM", this->TankImgui());
    }

#endif 

    // Swarm        
    for(size_t j = 0; j < numOfGroups; j++)
    {
        this->createSwarmGroup();   
    }

    // Tank
    for(int i = 0; i < 1; i++)
    {
        this->createTank();
    }

    // Lich
    for(int i = 0; i < 1; i++)
    {
        this->createLich();
    }
    
}

void SpawnHandler::createTank()
{
    this->tankIDs.push_back(this->currScene->createEntity());
    this->allEntityIDs.push_back(this->tankIDs.back());
    if (dynamic_cast<NetworkScene*>(currScene) == nullptr)
    {
        static int tank = this->resourceManager->addMesh("assets/models/Swarm_Model.obj");
        this->currScene->setComponent<MeshComponent>(this->tankIDs.back(), tank);
    }
    this->currScene->setComponent<AiCombatTank>(this->tankIDs.back());
    this->currScene->setComponent<Rigidbody>(this->tankIDs.back());
    Rigidbody& rb = this->currScene->getComponent<Rigidbody>(this->tankIDs.back());
    rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
    rb.gravityMult = 5.0f;
    rb.friction = 3.0f;
    rb.mass = 10.0f;
    Transform& transform = this->currScene->getComponent<Transform>(this->tankIDs.back());
    transform.scale = glm::vec3(3.0f, 3.0f, 3.0f);
    this->currScene->setComponent<Collider>(this->tankIDs.back(), Collider::createSphere(4.0f*transform.scale.x));
    this->aiHandler->createAIEntity(this->tankIDs.back(), "tankFSM");
    TankComponent& tankComp = this->currScene->getComponent<TankComponent>(this->tankIDs.back());
    tankComp.origScaleY = transform.scale.y;
    this->currScene->setInactive(this->tankIDs.back());
}

void SpawnHandler::createLich()
{
    
    this->lichIDs.push_back(this->currScene->createEntity());
    this->allEntityIDs.push_back(this->lichIDs.back());
    if (dynamic_cast<NetworkScene*>(currScene) == nullptr)
    {
        static int lich = this->resourceManager->addMesh("assets/models/Swarm_Model.obj");
        this->currScene->setComponent<MeshComponent>(this->lichIDs.back(), lich);
    }
    this->currScene->setComponent<Rigidbody>(this->lichIDs.back());
    Rigidbody& rb = this->currScene->getComponent<Rigidbody>(this->lichIDs.back());
    rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
    rb.gravityMult = 5.0f;
    rb.friction = 3.0f;
    rb.mass = 10.0f;
    Transform& transform = this->currScene->getComponent<Transform>(this->lichIDs.back());
    transform.scale = glm::vec3(1.0f, 3.0f, 1.0f);
    this->currScene->setComponent<Collider>(this->lichIDs.back(), Collider::createCapsule(4.0f, 4.0f*transform.scale.y));
    this->aiHandler->createAIEntity(this->lichIDs.back(), "lichFSM");
    LichComponent& lichComp = this->currScene->getComponent<LichComponent>(this->lichIDs.back());
    lichComp.origScaleY = transform.scale.y;
    this->currScene->setInactive(this->lichIDs.back());
}

void SpawnHandler::createSwarmGroup()
{
    
    
    this->swarmGroups.push_back(new SwarmGroup); //TODO: Does this work as expected? Do we need to clear (delete contents) this on every init? 
    for (size_t i = 0; i < this->group_size; i++)
    {
        this->swarmIDs.push_back(this->currScene->createEntity());
        this->allEntityIDs.push_back(this->swarmIDs.back());
        if (dynamic_cast<NetworkScene*>(currScene) == nullptr)
        {
            static int swarm = this->resourceManager->addMesh("assets/models/Swarm_Model.obj");
            this->currScene->setComponent<MeshComponent>(this->swarmIDs.back(), swarm);
        }
        this->currScene->setComponent<AiCombatSwarm>(this->swarmIDs.back());
        this->currScene->setComponent<Collider>(this->swarmIDs.back(), Collider::createSphere(4.0f));
        this->currScene->setComponent<Rigidbody>(this->swarmIDs.back());
        Rigidbody& rb = this->currScene->getComponent<Rigidbody>(this->swarmIDs.back());
        rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
        rb.gravityMult = 5.0f;
        rb.friction = 1.5f;
        this->aiHandler->createAIEntity(this->swarmIDs.back(), "swarmFSM");
        this->swarmGroups.back()->members.push_back(this->swarmIDs.back());
        this->currScene->setInactive(this->swarmIDs.back());
        this->sceneHandler->getScene()->getComponent<SwarmComponent>(this->swarmIDs.back()).group = this->swarmGroups.back();
        SwarmComponent& swarmComp = this->currScene->getComponent<SwarmComponent>(this->swarmIDs.back());
        swarmComp.life = 0;
    }
}

bool SpawnHandler::allDead()
{
    bool ret = true;

    for(auto p: allEntityIDs)
    {
        if(this->currScene->isActive(p))
        {
            ret = false;
            break;
        }
    }
    
    return ret;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////
///////////////[[ IMGUI FUNCTIONS ]]//////////////////
//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

ImguiLambda SpawnHandler::TankImgui()
{
   
    return [&](FSM* fsm, uint32_t entityId) -> void{
        TankFSM* tankFSM = (TankFSM*)fsm;

        auto entityImguiWindow = [&](TankFSM* tankFsm, uint32_t entityId)->void 
        {
            auto& entityTankComponent	= this->sceneHandler->getScene()->getComponent<TankComponent>(entityId);
            auto& entiyFSMAgentComp		= this->sceneHandler->getScene()->getComponent<FSMAgentComponent>(entityId);
            auto& entityRigidBody		= this->sceneHandler->getScene()->getComponent<Rigidbody>(entityId);
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
            glm::vec4 entityPos = glm::vec4(this->sceneHandler->getScene()->getComponent<Transform>(entityId).position, 1.f);

            auto screenPos = this->currScene->getMainCamera()->projection * this->currScene->getMainCamera()->view * entityPos;
            glm::vec3 realScreenPos; 
            realScreenPos.x = (screenPos.x / screenPos.w) * 1920/2;
            realScreenPos.y = (screenPos.y / screenPos.w) * 1080/2;
            realScreenPos.z = screenPos.z / screenPos.w;

            uiRenderer->setTexture(this->fontTextureIndex);
            //Scene::getUIRenderer()->renderString(std::to_string(entityId), realScreenPos.x, realScreenPos.y, 20, 20); 
            uiRenderer->renderString(std::to_string(entityId), glm::vec3(entityPos), glm::vec2(20, 20)); 
        }    

        entityImguiWindow(tankFSM, entityId);
    };
}

ImguiLambda SpawnHandler::LichImgui()
{
    return [&](FSM* fsm, uint32_t entityId) -> void{
        // Imgui Lich 
        LichFSM* swarmFsm = (LichFSM*)fsm;

        auto& lichComponent     = this->sceneHandler->getScene()->getComponent<LichComponent>(entityId);
        auto& entiyFSMAgentComp = this->sceneHandler->getScene()->getComponent<FSMAgentComponent>(entityId);
        auto& entityRigidBody   = this->sceneHandler->getScene()->getComponent<Rigidbody>(entityId);
        int& health             = lichComponent.life;
        float& mana             = lichComponent.mana;
        float& speed            = lichComponent.speed;
        float& attackRange      = lichComponent.attackRadius;
        float& sightRange       = lichComponent.sightRadius;
        float& gravity 		    = entityRigidBody.gravityMult;
        bool& tempAttack        = lichComponent.tempAttack;
        std::string tempStrat   = lichComponent.getStrat();
        std::string& status     = entiyFSMAgentComp.currentNode->status;
        ImGui::Text(status.c_str());
        ImGui::Text(tempStrat.c_str());
        ImGui::Checkbox("Attack", &tempAttack);
        ImGui::SliderFloat("mana", &mana, 0, 100);
        ImGui::SliderInt("health", &health, 0, lichComponent.FULL_HEALTH);
        ImGui::SliderFloat("speed", &speed, 0, 100);
        ImGui::SliderFloat("gravity", &gravity, 0, 10);
        ImGui::SliderFloat("attackRange", &attackRange, 0, 100);
        ImGui::SliderFloat("sightRange", &sightRange, 0, 100);
    };
}

ImguiLambda SpawnHandler::SwarmImgui()
{
    return [&](FSM* fsm, uint32_t entityId) -> void{
        // Imgui Swarm
        SwarmFSM* swarmFSM = (SwarmFSM*)fsm;

        auto entityImguiWindow = [&](SwarmFSM* swarmFsm, uint32_t entityId)->void 
        {
            auto& entitySwarmComponent      = this->sceneHandler->getScene()->getComponent<SwarmComponent>(entityId);
            auto& entityAiCombatComponent   = this->sceneHandler->getScene()->getComponent<AiCombatSwarm>(entityId);
            auto& entiyFSMAgentComp         = this->sceneHandler->getScene()->getComponent<FSMAgentComponent>(entityId);
            auto& entityRigidBody           = this->sceneHandler->getScene()->getComponent<Rigidbody>(entityId);
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
            glm::vec3 entityPos3 =this->sceneHandler->getScene()->getComponent<Transform>(entityId).position;
            glm::vec4 entityPos4 = glm::vec4(entityPos3, 1.f);

            auto screenPos = this->currScene->getMainCamera()->projection * this->currScene->getMainCamera()->view * entityPos4;
            glm::vec3 realScreenPos; 
            realScreenPos.x = (screenPos.x / screenPos.w) * 1920/2;
            realScreenPos.y = (screenPos.y / screenPos.w) * 1080/2;
            realScreenPos.z = screenPos.z / screenPos.w;

            uiRenderer->setTexture(this->fontTextureIndex);
            //Scene::getUIRenderer()->renderString(std::to_string(entityId), realScreenPos.x, realScreenPos.y, 20, 20); 
            uiRenderer->renderString(std::to_string(entityId), entityPos3, glm::vec2(20, 20)); 
        }    

        std::string playerString = "playerID";
        int playerID;
        this->sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
        auto& playerCombat = this->sceneHandler->getScene()->getComponent<Combat>(playerID);
        if(ImGui::Button("Kill Player")){
            playerCombat.health = 0; 
        }
        if(ImGui::Button("INVINCIBLE Player")){
            playerCombat.health = INT_MAX; 
        }
        ImGui::Separator();
        ImGui::Separator();
        entityImguiWindow(swarmFSM, entityId);

        auto& entitySwarmComponent    = this->sceneHandler->getScene()->getComponent<SwarmComponent>(entityId);
        auto& entityAiCombatComponent = this->sceneHandler->getScene()->getComponent<AiCombatSwarm>(entityId);
        auto& entiyFSMAgentComp       = this->sceneHandler->getScene()->getComponent<FSMAgentComponent>(entityId);

        std::string groupName = "GroupMembers["+std::to_string(entitySwarmComponent.group->myId)+"]";
        if(ImGui::TreeNode(groupName.c_str()))
        {
            if(ImGui::Button("Kill All")){
                entitySwarmComponent.life = 0; 
                for(auto& ent : entitySwarmComponent.group->members)
                {              
                    auto& entSwarmComp = this->sceneHandler->getScene()->getComponent<SwarmComponent>(ent);                    
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
}


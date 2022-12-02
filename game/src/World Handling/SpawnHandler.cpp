#include "SpawnHandler.hpp"
#include "../Scenes/GameScene.h"
#include "../Ai/Behaviors/HelperFuncs.hpp"
#include "../Network/ServerGameMode.h"
#include <functional>
#include <stack>
#include <random>
#include <algorithm>

void SpawnHandler::spawnEnemiesIntoRoom()
{
    this->aiHandler->resetEventSystemLastReturn();

    int swarmIdx        = 0;
    int lichIdx         = 0;
    int tankIdx         = 0;
    int counter         = 0;
    this->nrOfEnemiesPerRoom  = enemiesPerTiles;

    this->tilePicker.init(this->roomHandler->getFreeTileInfos());
    
    this->nrOfEnemiesPerRoom *= this->tilePicker.size();
    this->nrOfEnemiesPerRoom = std::clamp((int)this->nrOfEnemiesPerRoom, 0 , SpawnHandler::MAX_NR_OF_ENEMIES);

    this->nrOfTanks_inRoom  = std::clamp((int)(this->tilePicker.size() * enemiesPerTiles *PERCENTAGE_TANKS), 0, MAX_NR_TANKS);
    this->nrOfLichs_inRoom  = std::clamp((int)(this->tilePicker.size() * enemiesPerTiles *PERCENTAGE_LICHS), 0, MAX_NR_LICHS);
    int tempNrOfSwarms      = std::clamp((int)(this->tilePicker.size() * enemiesPerTiles *PERCENTAGE_SWARMS),0, NR_BLOBS_IN_GROUP*MAX_NR_SWARMGROUPS);
    this->nrOfGroups_inRoom = tempNrOfSwarms / SpawnHandler::NR_BLOBS_IN_GROUP;
    this->nrOfSwarms_inRoom = nrOfGroups_inRoom * SpawnHandler::NR_BLOBS_IN_GROUP;
    this->nrOfEnemiesPerRoom = (float)(this->nrOfSwarms_inRoom+this->nrOfLichs_inRoom+this->nrOfTanks_inRoom);

    // Imgui data...
    this->nrOfTilesInRoom = (int)this->tilePicker.size();

    if(SpawnHandler::USE_DEBUG)
    {
        // Spawn Tanks
        for(size_t i = 0; i < NR_TANK_DBG; i++){
            this->spawnTank(tankIdx, this->tilePicker.getRandomEmptyTile()->getPos());
            tankIdx++;
        }

        // Spawn Lichs
        for(size_t i = 0; i < NR_LICH_DBG; i++){
                 
            lichIdx += this->spawnLich(lichIdx,this->tilePicker.getRandomEmptyNeighbouringTiles(2));        
        }

        // Spawn Swarms
        for(size_t i = 0; i < NR_SWARM_GROUPS_DBG; i++){
            swarmIdx += this->spawnSwarmGroup(swarmIdx, this->tilePicker.getRandomEmptyNeighbouringTiles(SpawnHandler::NR_BLOBS_IN_GROUP));        
        }

    }
    else 
    {
        // Spawn Tanks
        for(size_t i = 0; i < nrOfTanks_inRoom; i++){
            this->spawnTank(tankIdx, this->tilePicker.getRandomEmptyTile()->getPos());
            tankIdx++;
        }

        // Spawn Lichs
        for(size_t i = 0; i < nrOfLichs_inRoom; i++){
            
            lichIdx += this->spawnLich(lichIdx,this->tilePicker.getRandomEmptyNeighbouringTiles(2));        
        }

        // Spawn Swarms
        for(size_t i = 0; i < nrOfGroups_inRoom; i++){
            swarmIdx += this->spawnSwarmGroup(swarmIdx, this->tilePicker.getRandomEmptyNeighbouringTiles(SpawnHandler::NR_BLOBS_IN_GROUP));        
        }

    }

    initTanks();
    this->tilePicker.clean();
}

void SpawnHandler::spawnTank(const int tankIdx, const glm::vec3& pos)
{
    currScene->setActive(this->tankIDs[tankIdx]);
    Transform& transform = currScene->getComponent<Transform>(this->tankIDs[tankIdx]);
        
    transform.position = pos;

    debugRays.push_back({pos, {0.1f,0.5f,0.5f}});

    //Reset
    TankComponent& tankComp = currScene->getComponent<TankComponent>(this->tankIDs[tankIdx]);
    tankComp.life = tankComp.FULL_HEALTH;
    transform.scale.y = tankComp.origScaleY;

    if (dynamic_cast<NetworkScene*>(currScene) != nullptr)
    {
        ((NetworkScene*)currScene)->addEvent({(int)GameEvent::ACTIVATE, this->tankIDs[tankIdx]});       
    }
}

uint32_t SpawnHandler::spawnLich(int lichIdx, std::vector<const TileInfo*> tileInfos)
{
    //Make sure we only try to create Lich if we have one tile for Lich, and one for alter
    if(tileInfos.size() == 2)
    {
        currScene->setActive(this->lichIDs[lichIdx]);
        const int alterID = this->lichObjects[this->lichIDs[lichIdx]].alterID;
        const int graveID = this->lichObjects[this->lichIDs[lichIdx]].graveID;
        currScene->setActive(alterID);
        currScene->setActive(graveID);        

        const auto& lichPos = tileInfos[0];
        const auto& alterPos = tileInfos[1];

        Transform& transform = currScene->getComponent<Transform>(this->lichIDs[lichIdx]);
        
        transform.position = lichPos->getPos();

        debugRays.push_back({lichPos->getPos(), {1.f,1.f,0.f}});

        //Reset
        LichComponent& lichComp = currScene->getComponent<LichComponent>(this->lichIDs[lichIdx]);
        lichComp.life = lichComp.FULL_HEALTH;
        transform.scale.y = lichComp.origScaleY;

        // Place Alter 
        Transform& alterTransform = currScene->getComponent<Transform>(alterID);
        alterTransform.position = alterPos->getPos();

        // Place Grave 
        Transform& graveTransform = currScene->getComponent<Transform>(graveID);
        graveTransform.position = this->tilePicker.getRandomFreeTileFarAwayFrom(alterPos)->getPos(); 

        if (dynamic_cast<NetworkScene*>(currScene) != nullptr)
        {
            ((NetworkScene*)currScene)->addEvent({(int)GameEvent::ACTIVATE, this->lichIDs[lichIdx]});
            ((NetworkScene*)currScene)->addEvent({(int)GameEvent::ACTIVATE, alterID});
            ((NetworkScene*)currScene)->addEvent({(int)GameEvent::ACTIVATE, graveID});
            ((NetworkScene*)currScene)->addEvent({(int)GameEvent::SET_POS_OBJECT, alterID}, {alterTransform.position.x, alterTransform.position.y, alterTransform.position.z});
            ((NetworkScene*)currScene)->addEvent({(int)GameEvent::SET_POS_OBJECT, graveID}, {graveTransform.position.x, graveTransform.position.y, graveTransform.position.z});
        }

        debugRays.push_back({alterPos->getPos(), {1.f,0.f,1.f}});
        debugRays.push_back({graveTransform.position, {0.5f,0.f,0.5f}});
        

        // Returns number of created Liches...
        return 1;
    }
    else
    {
        return 0;
    }
}

uint32_t SpawnHandler::spawnSwarmGroup(const int swarmStartIdx, std::vector<const TileInfo*> tileInfos)
{
    int swarmIdx = swarmStartIdx;

    for(auto tile : tileInfos)
    {
        spawnSwarm(swarmIdx, tile->getPos());
        swarmIdx++;
    }
    return (uint32_t)tileInfos.size();
}

void SpawnHandler::spawnSwarm(int swarmIdx, const glm::vec3& pos)
{
    currScene->setActive(this->swarmIDs[swarmIdx]);

    Transform& transform = currScene->getComponent<Transform>(this->swarmIDs[swarmIdx]);
    
    transform.position = pos;

    debugRays.push_back({pos, {1.f,0.f,0.f}});
    
    //Temporary enemie reset
    SwarmComponent& swarmComp = currScene->getComponent<SwarmComponent>(this->swarmIDs[swarmIdx]);
    transform.scale.y = 1.0f;
    swarmComp.life = swarmComp.FULL_HEALTH;
    swarmComp.group->inCombat = false;    

    transform.rotation.y = lookAtY(transform.position, transform.position + genRandomDir({1.f,0.f,1.f}));

    swarmComp.group->aliveMembers.push(0); 

    swarmComp.setGroupMidPos(this->currScene);
    swarmComp.setGroupRadius(this->currScene);
    if (currScene->getSceneType() == SceneType::NetworkScene)
    {
        ((NetworkScene*)currScene)->addEvent({(int)GameEvent::ACTIVATE, this->swarmIDs[swarmIdx]});    
    }
}

void SpawnHandler::initTanks()
{
    for(auto t: tankIDs)
    {
        TankComponent& tankComp = this->currScene->getComponent<TankComponent>(t);
		tankComp.setFriends(this->currScene, t);
    }
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
    for(size_t j = 0; j < SpawnHandler::MAX_NR_SWARMGROUPS; j++)
    {
        this->createSwarmGroup();   
    }

    // Tank
    for(int i = 0; i < SpawnHandler::MAX_NR_TANKS; i++)
    {
        this->createTank();
    }

    // Lich
    for(int i = 0; i < SpawnHandler::MAX_NR_LICHS; i++)
    {
        this->createLich();
    }
    
}

void SpawnHandler::killAllEnemiesOutsideRoom()
{
    
#ifdef _CONSOLE 
Log::write("Killing all enemies outside room...");
#endif 

    this->sceneHandler->getScene()->getSceneReg().view<FSMAgentComponent,Transform>(entt::exclude<Inactive>).each(
        [&](const auto& entity, FSMAgentComponent, Transform& transform)
        {
            bool enemyUnderFloor    = transform.position.y < 0;
            bool enemyOverRoof      = transform.position.y > RoomHandler::BORDER_COLLIDER_HEIGHT; 
            bool enemyOutsideRoom   = false;

            glm::vec3 tempPos = transform.position;
            tempPos.y += RoomHandler::TILE_WIDTH * 3;

            Ray rayToMiddle{tempPos, glm::normalize(this->roomHandler->getRoomPos() - tempPos)};

            RayPayload rp = this->sceneHandler->getPhysicsEngine()->raycast(rayToMiddle,glm::length(this->roomHandler->getRoomPos() - tempPos));

            if(rp.hit)
            {
                if(this->sceneHandler->getScene()->hasComponents<EdgeTile>(rp.entity))
                {
                    enemyOutsideRoom  = true; 
                }
            }        

            if(enemyUnderFloor || enemyOverRoof || enemyOutsideRoom)
            {
                int id = static_cast<int>(entity);
                
                if(this->sceneHandler->getScene()->hasComponents<SwarmComponent>(id))
                {
                    auto& comp = this->sceneHandler->getScene()->getComponent<SwarmComponent>(id);
                    comp.life = 0;
#ifdef _CONSOLE 
                    Log::write("Killing Swarm["+std::to_string(id)+"] enemies outside room...");
#endif 
                }
                else if(this->sceneHandler->getScene()->hasComponents<LichComponent>(id))
                {
                    auto& comp = this->sceneHandler->getScene()->getComponent<LichComponent>(id);
                    comp.life = 0;
#ifdef _CONSOLE 
                    Log::write("Killing Lich["+std::to_string(id)+"] enemies outside room...");
#endif 
                }
                else if(this->sceneHandler->getScene()->hasComponents<TankComponent>(id))
                {
                    auto& comp = this->sceneHandler->getScene()->getComponent<TankComponent>(id);
                    comp.life = 0;
#ifdef _CONSOLE 
                    Log::write("Killing Tank["+std::to_string(id)+"] enemies outside room...");
#endif 
                }
            }               
        }
    );
}

void SpawnHandler::createTank()
{
    ServerGameMode* netScene = dynamic_cast<ServerGameMode*>(currScene);
    if (netScene == nullptr)
    {
        static int tank = this->resourceManager->addMesh("assets/models/golem.obj");
        
        this->tankIDs.push_back(this->currScene->createEntity());
        this->allEntityIDs.push_back(this->tankIDs.back());
        this->currScene->setComponent<MeshComponent>(this->tankIDs.back(), tank);        
    }
    else
    {
        this->tankIDs.push_back(netScene->spawnEnemy(0));
        this->allEntityIDs.push_back(this->tankIDs.back());
    }
    this->currScene->setComponent<AiCombatTank>(this->tankIDs.back());
    this->currScene->setComponent<Rigidbody>(this->tankIDs.back());
    Rigidbody& rb = this->currScene->getComponent<Rigidbody>(this->tankIDs.back());
    rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
    rb.gravityMult = 5.0f;
    rb.friction = 3.0f;
    rb.mass = 10.0f;
    Transform& transform = this->currScene->getComponent<Transform>(this->tankIDs.back());
    transform.scale = glm::vec3(3.0f, 3.0f, 3.0f); //TODO: Remove this line when we have real model 
    this->currScene->setComponent<Collider>(this->tankIDs.back(), Collider::createSphere(TankComponent::colliderRadius));
    this->aiHandler->createAIEntity(this->tankIDs.back(), "tankFSM");
    TankComponent& tankComp = this->currScene->getComponent<TankComponent>(this->tankIDs.back());
    tankComp.origScaleY = transform.scale.y;
    this->currScene->setInactive(this->tankIDs.back());
    tankComp.life = 0;

    static int nrOfHumps = 10;
    if (netScene != nullptr) 
    {
        netScene->addEvent({(int)GameEvent::INACTIVATE, this->tankIDs.back()});
        netScene->addEvent({(int)GameEvent::SPAWN_GROUND_HUMP, nrOfHumps});
        for(int i = 0; i < nrOfHumps; i++){
            int ent = netScene->createEntity();
            this->currScene->setInactive(ent);
            tankComp.humpEnteties.push_back(ent);
            netScene->addEvent({ent});
        }
    }
    else 
    {
        static int tankHump = this->resourceManager->addMesh("assets/models/hump.obj");
        for(int i = 0; i < nrOfHumps; i++)
        {
            int ent = this->currScene->createEntity();
            this->currScene->setComponent<MeshComponent>(ent, tankHump);
            this->currScene->setInactive(ent);
            tankComp.humpEnteties.push_back(ent);
        }
    }

}

void SpawnHandler::createLich()
{
    
    ServerGameMode* netScene = dynamic_cast<ServerGameMode*>(currScene);

    // Create Lich
    if(netScene == nullptr)
    {        
        this->lichIDs.push_back(this->currScene->createEntity());
        this->allEntityIDs.push_back(this->lichIDs.back());
        //TODO :  Move createEntities stuff in here
    }
    else
    {
        this->lichIDs.push_back(netScene->spawnEnemy(0));
    }

    this->currScene->setComponent<Rigidbody>(this->lichIDs.back());
    Rigidbody& rb = this->currScene->getComponent<Rigidbody>(this->lichIDs.back());
    rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
    rb.gravityMult = 5.0f;
    rb.friction = 3.0f;
    rb.mass = 10.0f;
    Transform& transform = this->currScene->getComponent<Transform>(this->lichIDs.back());
    transform.scale = glm::vec3(1.0f, 3.0f, 1.0f); //TODO: Remove this line when we have real model 
    this->currScene->setComponent<Collider>(this->lichIDs.back(), Collider::createCapsule(LichComponent::colliderRadius, LichComponent::colliderHeight));
    this->aiHandler->createAIEntity(this->lichIDs.back(), "lichFSM");
    LichComponent& lichComp = this->currScene->getComponent<LichComponent>(this->lichIDs.back());
    lichComp.origScaleY = transform.scale.y;
    this->currScene->setInactive(this->lichIDs.back());
    lichComp.life = 0;

    //Create Grave
    auto& graveID = this->lichObjects[this->lichIDs.back()].graveID = this->currScene->createEntity();
    this->currScene->setComponent<Collider>(graveID, Collider::createBox(
        glm::vec3{LichComponent::graveWidth,LichComponent::graveHeight,LichComponent::graveDepth})
        );

    this->currScene->getComponent<LichComponent>(this->lichIDs.back()).graveID = graveID;
    this->currScene->setInactive(graveID);

    //Create Alter
    auto& alterID = this->lichObjects[this->lichIDs.back()].alterID = this->currScene->createEntity();    
    this->currScene->setComponent<Collider>(alterID, Collider::createBox(
        glm::vec3{LichComponent::alterWidth,LichComponent::alterHeight,LichComponent::alterDepth})
        );
    
    this->currScene->getComponent<LichComponent>(this->lichIDs.back()).alterID = alterID;
    this->currScene->setInactive(alterID);

    
    

    if(netScene == nullptr)
    {
        static int lich = this->resourceManager->addMesh("assets/models/Swarm_Model.obj");
        static int grave = this->resourceManager->addMesh("assets/models/grave.obj");
        static int alter = this->resourceManager->addMesh("assets/models/alter.obj");
        static int fireOrb_mesh = this->resourceManager->addMesh("assets/models/fire_orb.obj");
        static int lightOrb_mesh = this->resourceManager->addMesh("assets/models/light_orb.obj");
        static int iceOrb_mesh = this->resourceManager->addMesh("assets/models/ice_orb.obj");

        this->currScene->setComponent<MeshComponent>(this->lichIDs.back(), lich);

        this->currScene->setComponent<MeshComponent>(graveID, grave);
        this->currScene->setComponent<MeshComponent>(alterID, alter);                

        // Create Orbs
        for(size_t i = 0; i < LichComponent::NR_FIRE_ORBS;i++)
        {
            lichComp.fireOrbs[i] = this->currScene->createEntity();
            this->currScene->setComponent<Collider>(lichComp.fireOrbs[i], Collider::createSphere(LichComponent::orbRadius));
            this->currScene->setComponent<Orb>(lichComp.fireOrbs[i]);
            this->currScene->setInactive(lichComp.fireOrbs[i]);
            this->currScene->setComponent<Rigidbody>(lichComp.fireOrbs[i]);
            Rigidbody& rb = this->currScene->getComponent<Rigidbody>(lichComp.fireOrbs[i]);
            rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
            rb.gravityMult = 0.0f;
            rb.friction = 3.0f;
            rb.mass = 10.0f;
        }
        for(size_t i = 0; i < LichComponent::NR_ICE_ORBS;i++)
        {
            lichComp.iceOrbs[i] = this->currScene->createEntity();
            this->currScene->setComponent<Collider>(lichComp.iceOrbs[i], Collider::createSphere(LichComponent::orbRadius));
            this->currScene->setComponent<Orb>(lichComp.iceOrbs[i]);
            this->currScene->setInactive(lichComp.iceOrbs[i]);
            this->currScene->setComponent<Rigidbody>(lichComp.iceOrbs[i]);
            Rigidbody& rb = this->currScene->getComponent<Rigidbody>(lichComp.iceOrbs[i]);
            rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
            rb.gravityMult = 0.0f;
            rb.friction = 3.0f;
            rb.mass = 10.0f;
        }
        for(size_t i = 0; i < LichComponent::NR_LIGHT_ORBS;i++)
        {
            lichComp.lightOrbs[i] = this->currScene->createEntity();
            this->currScene->setComponent<Collider>(lichComp.lightOrbs[i], Collider::createSphere(LichComponent::orbRadius));
            this->currScene->setComponent<Orb>(lichComp.lightOrbs[i]);
            this->currScene->setInactive(lichComp.lightOrbs[i]);
            this->currScene->setComponent<Rigidbody>(lichComp.lightOrbs[i]);
            Rigidbody& rb = this->currScene->getComponent<Rigidbody>(lichComp.lightOrbs[i]);
            rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
            rb.gravityMult = 0.0f;
            rb.friction = 3.0f;
            rb.mass = 10.0f;
        }

        // Create Orbs
        for(size_t i = 0; i < LichComponent::NR_FIRE_ORBS;i++)
        {
            this->currScene->setComponent<MeshComponent>(lichComp.fireOrbs[i], fireOrb_mesh);            
        }
        for(size_t i = 0; i < LichComponent::NR_ICE_ORBS;i++)
        {
            this->currScene->setComponent<MeshComponent>(lichComp.iceOrbs[i], iceOrb_mesh);
        }
        for(size_t i = 0; i < LichComponent::NR_LIGHT_ORBS;i++)
        {
            this->currScene->setComponent<MeshComponent>(lichComp.lightOrbs[i], lightOrb_mesh);
        }
    }
    else
    {
        netScene->addEvent({(int)GameEvent::INACTIVATE, lichIDs.back()});


        netScene->addEvent({(int)GameEvent::SPAWN_OBJECT, (int)graveID, (int)ObjectTypes::LICH_GRAVE},{0.f,0.f,0.f,   0.f,0.f,0.f,   1.f,1.f,1.f});
        netScene->addEvent({(int)GameEvent::SPAWN_OBJECT, (int)alterID, (int)ObjectTypes::LICH_ALTER},{0.f,0.f,0.f,   0.f,0.f,0.f,   1.f,1.f,1.f});
        netScene->addEvent({(int)GameEvent::INACTIVATE, (int)graveID});
        netScene->addEvent({(int)GameEvent::INACTIVATE, (int)alterID});
        
        // Create Orbs
        for(size_t i = 0; i < LichComponent::NR_FIRE_ORBS;i++)
        {
            lichComp.fireOrbs[i] = this->currScene->createEntity();
            this->currScene->setInactive(lichComp.fireOrbs[i]);
            this->currScene->setComponent<Orb>(lichComp.fireOrbs[i]);
            this->currScene->setComponent<Rigidbody>(lichComp.fireOrbs[i]);
            netScene->addEvent({(int)GameEvent::SPAWN_ORB, lichComp.fireOrbs[i], (int)ATTACK_STRATEGY::FIRE});
        }
        for(size_t i = 0; i < LichComponent::NR_ICE_ORBS;i++)
        {
            lichComp.iceOrbs[i] = this->currScene->createEntity();
            this->currScene->setInactive(lichComp.iceOrbs[i]);
            this->currScene->setComponent<Orb>(lichComp.iceOrbs[i]);
            this->currScene->setComponent<Rigidbody>(lichComp.iceOrbs[i]);
            netScene->addEvent({(int)GameEvent::SPAWN_ORB, lichComp.iceOrbs[i], (int)ATTACK_STRATEGY::ICE});

        }
        for(size_t i = 0; i < LichComponent::NR_LIGHT_ORBS;i++)
        {
            lichComp.lightOrbs[i] = this->currScene->createEntity();
            this->currScene->setInactive(lichComp.lightOrbs[i]);
            this->currScene->setComponent<Orb>(lichComp.lightOrbs[i]);
            this->currScene->setComponent<Rigidbody>(lichComp.lightOrbs[i]);
            netScene->addEvent({(int)GameEvent::SPAWN_ORB, lichComp.lightOrbs[i], (int)ATTACK_STRATEGY::LIGHT});

        }      
    }
        
}

void SpawnHandler::createSwarmGroup()
{
    ServerGameMode* netScene = dynamic_cast<ServerGameMode*>(currScene);
    this->swarmGroups.push_back(new SwarmGroup); //TODO: Does this work as expected? Do we need to clear (delete contents) this on every init? 
    for (size_t i = 0; i < SpawnHandler::NR_BLOBS_IN_GROUP; i++)
    {
        if (netScene == nullptr)
        {
            this->swarmIDs.push_back(this->currScene->createEntity());
            static int swarm = this->resourceManager->addMesh("assets/models/Swarm_Model.obj");
            this->currScene->setComponent<MeshComponent>(this->swarmIDs.back(), swarm);
        }
        else
        {
            this->swarmIDs.push_back(netScene->spawnEnemy(0));
        }
        this->allEntityIDs.push_back(this->swarmIDs.back());
        this->currScene->setComponent<AiCombatSwarm>(this->swarmIDs.back());
        this->currScene->setComponent<Collider>(this->swarmIDs.back(), Collider::createSphere(SwarmComponent::colliderRadius));
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

        if (netScene != nullptr) 
        {
            netScene->addEvent({(int)GameEvent::INACTIVATE, this->swarmIDs.back()});
        }
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

void SpawnHandler::updateImgui()
{
    ImGui::Begin("SpawnHandler Info");
        if(ImGui::BeginTabBar("Enemies")){


            if(ImGui::BeginTabItem("Enemies")){
                if(ImGui::Button("Kill all"))
                {
                    this->sceneHandler->getScene()->getSceneReg().view<SwarmComponent>(entt::exclude<Inactive>).each(
                        [&](SwarmComponent& swarmComp){
                            swarmComp.life = 0;
                        }
                    );
                    this->sceneHandler->getScene()->getSceneReg().view<LichComponent>(entt::exclude<Inactive>).each(
                        [&](LichComponent& lichComp){
                            lichComp.life = 0;
                        }
                    );
                    this->sceneHandler->getScene()->getSceneReg().view<TankComponent>(entt::exclude<Inactive>).each(
                        [&](TankComponent& tankComp){
                            tankComp.life = 0;
                        }
                    );                         
                }
                ImGui::Separator();
                ImGui::Text("# Enimies Total : %d", (int)this->nrOfEnemiesPerRoom);
                ImGui::Text("# Swarm groups: %d", this->nrOfGroups_inRoom);
                ImGui::Text("# Swarms      : %d", this->nrOfSwarms_inRoom);
                ImGui::Text("# Tanks       : %d", this->nrOfTanks_inRoom );
                ImGui::Text("# Lichs       : %d", this->nrOfLichs_inRoom );
                
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("Tiles")){
                for(auto ray : debugRays)
                {
                    this->sceneHandler->getDebugRenderer()->renderLine(
                        ray.pos,
                        ray.pos + glm::vec3(0.0f, 1.0f, 0.0f),
                        ray.col);

                }
                ImGui::Text("# Active Tiles : %d", this->nrOfTilesInRoom);
                
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

    ImGui::End();
}

ImguiLambda SpawnHandler::TankImgui()
{
   
    return [&](FSM* fsm, uint32_t entityId) -> void{
        TankFSM* tankFSM = (TankFSM*)fsm;

        auto entityImguiWindow = [&](TankFSM* tankFsm, uint32_t entityId)->void 
        {
            auto& entityTankComponent	= this->sceneHandler->getScene()->getComponent<TankComponent>(entityId);
            auto& entiyFSMAgentComp		= this->sceneHandler->getScene()->getComponent<FSMAgentComponent>(entityId);
            auto& entityRigidBody		= this->sceneHandler->getScene()->getComponent<Rigidbody>(entityId);
            const glm::vec3& tankPos    = this->sceneHandler->getScene()->getComponent<Transform>(entityId).position;
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
        
            ImGui::Text("pos: (%d, %d, %d)",
                (int)tankPos.x,
                (int)tankPos.y,
                (int)tankPos.z
            );
            ImGui::Separator();

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
        std::string tempStrat   = lichComponent.lastAttack;
        std::string& status     = entiyFSMAgentComp.currentNode->status;

        ImGui::Text(status.c_str());

        const glm::vec3& lichPos =
            this->sceneHandler->getScene()->getComponent<Transform>(entityId).position;
        ImGui::Text(
            "pos: (%d, %d, %d)",
            (int)lichPos.x,
            (int)lichPos.y,
            (int)lichPos.z
        );
        ImGui::Separator();    

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
            auto& entityTransform           = this->sceneHandler->getScene()->getComponent<Transform>(entityId);
            float& posY                     = entityTransform.position.y;
            float& rotSpeed                 = entitySwarmComponent.idleRotSpeed;
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
            const glm::vec3& blobPos = this->sceneHandler->getScene()->getComponent<Transform>(entityId).position;

            ImGui::Text(status.c_str());            
            ImGui::Text(
                "pos: (%d, %d, %d)",
                (int)blobPos.x,
                (int)blobPos.y,
                (int)blobPos.z
            );
            ImGui::Separator();    
            ImGui::SliderInt("health", &health, 0, 100);
            ImGui::SliderFloat("speed", &speed, 0, 100);
            ImGui::SliderFloat("pos y", &posY, 0, 15);
            ImGui::SliderFloat("rot speed", &rotSpeed, 0, 200);
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

void TilePicker::init(const std::vector<TileInfo>& freeTileInfos)
{
    static auto randomEngine = std::default_random_engine{};
    std::deque<const TileInfo*> tempTilePtrs;

    // Store temp ptrs to free Tiles, store old array structure
    for (size_t i = 0; i < freeTileInfos.size(); i++)
    {
        tempTilePtrs.push_back(&freeTileInfos[i]);
        ogNeighbourhood.insert({i, &freeTileInfos[i]});
    }

    // Randomize order of temp ptrs
    std::shuffle(tempTilePtrs.begin(), tempTilePtrs.end(), randomEngine);

    // Store randomized ptrs to unusedTileInfos
    for (auto& tileInfo : tempTilePtrs)
    {
        this->unusedTileInfos.push_back(tileInfo);
    }

    // Store status of tiles
    for (auto& tileInfo : this->unusedTileInfos)
    {
        this->freeTiles.insert({tileInfo, true});
    }
}
size_t TilePicker::size() const
{
    size_t c = 0;
    for (auto t : unusedTileInfos)
    {
        if (t)
            c++;
    }
    return c;
}
const TileInfo* TilePicker::getRandomEmptyTile()
{
    const TileInfo* ret = nullptr;

    if (this->size() > 0)
    {
        ret = getSpreadTile();
        usedTiles.push_back(ret);
        unusedTileInfos.remove(ret);
        this->freeTiles[ret] = false;
    }
    return ret;
}
std::vector<const TileInfo*>
TilePicker::getRandomEmptyNeighbouringTiles(const int nr)
{

    //TODO do not use while loop here...
    std::vector<const TileInfo*> neigbhourhood;
    std::vector<const TileInfo*> newPossibleNeighbours;
    std::unordered_map<const TileInfo*, bool> possibleNeigbhours;
    
    const TileInfo* currNeighbour = getSpreadTile();    
    possibleNeigbhours[currNeighbour] = true;
    neigbhourhood.push_back(currNeighbour);
    unusedTileInfos.remove(currNeighbour);  

    while (neigbhourhood.size() < nr)
    {

        newPossibleNeighbours =
            getPossibleNeighbours(currNeighbour, possibleNeigbhours);

        // abort if no possible neigbhour exists...
        if (newPossibleNeighbours.size() == 0)
        {
            Log::warning("Could not retrieve required amount of tiles...");
            break;
        }

        currNeighbour =
            newPossibleNeighbours[rand() % newPossibleNeighbours.size()];
        possibleNeigbhours[currNeighbour] = true;
        neigbhourhood.push_back(currNeighbour);
        unusedTileInfos.remove(currNeighbour);        
    }
    usedTiles.insert(usedTiles.end(), neigbhourhood.begin(),neigbhourhood.end());

    this->updateFreeTiles();

    return neigbhourhood;
}
std::vector<const TileInfo*> TilePicker::getPossibleNeighbours(
    const TileInfo* currentNeighbour,
    std::unordered_map<const TileInfo*, bool>& possibleNeigbhours
)
{

    // Set true if picked
    possibleNeigbhours.insert({currentNeighbour, true});

    for (auto n : getFreeNeighbours(currentNeighbour))
    {
        // Set false if not part of neigbhours yet
        possibleNeigbhours.insert({n, false});
    }

    // Get only Possible Neighbours
    std::vector<const TileInfo*> pickNeighbor;
    for (auto n : possibleNeigbhours)
    {
        if (!n.second)
            {
                pickNeighbor.push_back(n.first);
            }
    }

    return pickNeighbor;
}
std::vector<const TileInfo*> TilePicker::getFreeNeighbours(const TileInfo* tile)
{
    std::vector<const TileInfo*> freeNeighbours;

    if (this->freeTiles[this->ogNeighbourhood[tile->idRightOf()]])
    {
        freeNeighbours.push_back(this->ogNeighbourhood[tile->idRightOf()]);
    }

    if (this->freeTiles[this->ogNeighbourhood[tile->idLeftOf()]])
    {
        freeNeighbours.push_back(this->ogNeighbourhood[tile->idLeftOf()]);
    }

    if (this->freeTiles[this->ogNeighbourhood[tile->idDownOf()]])
    {
        freeNeighbours.push_back(this->ogNeighbourhood[tile->idDownOf()]);
    }

    if (this->freeTiles[this->ogNeighbourhood[tile->idUpOf()]])
    {
        freeNeighbours.push_back(this->ogNeighbourhood[tile->idUpOf()]);
    }

    return freeNeighbours;
}
void TilePicker::clean()
{
    unusedTileInfos.clear();
    freeTiles.clear();
    ogNeighbourhood.clear();
}
void TilePicker::calcEnemiesMidpoint()
{
    glm::vec3 tempMid{0.f, 0.f, 0.f};
    if (usedTiles.size() != 0)
    {
        for (auto t : usedTiles)
            {
                tempMid += t->getPos();
            }
        enemiesMidpoint = {
            tempMid.x / usedTiles.size(),
            tempMid.y / usedTiles.size(),
            tempMid.z / usedTiles.size()};
    }
}

const TileInfo* TilePicker::getSpreadTile()
{
    calcEnemiesMidpoint(); 

    std::vector<const TileInfo*> possibleTiles{this->unusedTileInfos.begin(), this->unusedTileInfos.end()}; 
    std::shuffle(possibleTiles.begin(),possibleTiles.end(),this->randomDev);
    
    // pick furthest from 4 random tiles
    const uint32_t samples = 4;  

    const TileInfo* furthest = possibleTiles.front();
    float prevFurthest = 0.f;
        
    int c = 0; 
    for(auto p : possibleTiles)
    {
        float dist = glm::length(p->getPos() - this->enemiesMidpoint);
        if( dist > prevFurthest)
        {
            prevFurthest = dist;
            furthest = p;
        }
        if(samples < c){break;}
        c++; 
    }
    return furthest;
}

void TilePicker::updateFreeTiles()
{
    for(auto usedTile : this->usedTiles)
    {
        this->freeTiles[usedTile] = false; 
    }
}
const TileInfo* TilePicker::getRandomFreeTileFarAwayFrom(const TileInfo* tile)
{
    std::vector<const TileInfo*> possibleTiles{
        this->unusedTileInfos.begin(), this->unusedTileInfos.end()};

    std::shuffle(possibleTiles.begin(),possibleTiles.end(),this->randomDev);
    
    // pick furthest from 4 random tiles
    const uint32_t samples = 4;

    const TileInfo* furthest = possibleTiles.front();
    float prevFurthest = 0.f;

    int c = 0; 
    for (auto p : possibleTiles)
    {
        float dist = glm::length(p->getPos() - tile->getPos());
        if (dist > prevFurthest)
        {
            prevFurthest = dist;
            furthest = p;
        }
        if(samples < c){break;}
        c++; 
    }
    usedTiles.push_back(furthest);
    unusedTileInfos.remove(furthest);

    return furthest;
}

#include "ServerGameMode.h"
#include "../Components/HealthComp.h"

//#define ROOMDEBUG

ServerGameMode::ServerGameMode(int level) {
    this->level = level;
}

ServerGameMode::~ServerGameMode()
{
    aiHandler.clean();
    this->sceneType = SceneType::GameModeScene;
}

void ServerGameMode::init()
{
	// Create seed
	#ifdef _CONSOLE
        this->roomSeed = time(0);
	#else
        this->roomSeed = time(0);
	#endif
	srand(this->roomSeed);

    aiHandler.init(this->getSceneHandler());
    this->getSceneHandler()->setAIHandler(&aiHandler);

    roomHandler.init(this, this->getResourceManager(), this->getPhysicsEngine(), false);
    roomHandler.generate(this->roomSeed, (uint16_t)this->level);
    createPortal();
    spawnHandler.init(
        &this->roomHandler,
        this,
        this->getSceneHandler(),
        &this->aiHandler,
        this->getResourceManager(),
        this->getUIRenderer()//this shall be nullptr
    );

    lastPlayerHps.resize(this->getPlayerSize());
    for (int i = 0; i < this->getPlayerSize(); i++)
    {
        this->setComponent<HealthComp>(getPlayer(i));
    }

    this->createSystem<OrbSystem>(this->getSceneHandler());

    auto* tankFSM = this->aiHandler.FSMs["tankFSM"];
    auto* lichFSM = this->aiHandler.FSMs["lichFSM"];
    auto* swarmFSM = this->aiHandler.FSMs["swarmFSM"];

    lastSwarmHp.resize(this->aiHandler.FSMsEntities[swarmFSM].size());
    lastLichHp.resize(this->aiHandler.FSMsEntities[lichFSM].size());
    lastTankHp.resize(this->aiHandler.FSMsEntities[tankFSM].size());
}

void ServerGameMode::update(float dt)
{
    aiHandler.update(dt);

    if (!this->newRoomFrame && roomHandler.playersInPathway(this->players))
    {
        addEvent({(int)GameEvent::CLOSE_OLD_DOORS}, {roomHandler.serverGetNextRoomIndex()});

#ifdef _CONSOLE
        printf("Server Active Room: %d\n", roomHandler.getActiveIndex());
        std::cout << "Server: player in new room" << std::endl;
#endif
        
        roomHandler.serverActivateCurrentRoom();
        spawnHandler.spawnEnemiesIntoRoom(this->level);

        this->newRoomFrame = true;
        this->timeWhenEnteredRoom = (uint32_t)Time::getTimeSinceStart();
        this->safetyCleanDone = false; 
    }
    else if (this->newRoomFrame)
    {
        if (!this->doorsClosed && roomHandler.playersInsideNewRoom(this->players))
        {
            addEvent({(int)GameEvent::CLOSE_NEW_DOORS});
            this->doorsClosed = true;
            roomHandler.serverDeactivateSurrounding();
        }
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
        std::cout << "Server" << ": all dead" << std::endl;
        this->addEvent({(int)GameEvent::ROOM_CLEAR});
        // Call when a room is cleared
        roomHandler.roomCompleted();
        this->doorsClosed = false;
        if (this->roomHandler.isPortalRoomDone() && !this->portalActivated)
        {
            std::cout << "Server: Spawn portal" << std::endl;
            this->addEvent({(int)GameEvent::SPAWN_PORTAL});
            this->portalActivated = true;
        }
    }
    //check if all players are at portal
    if (this->roomHandler.isPortalRoomDone())
    {
            
        std::vector<int> colWithPortal = this->getPhysicsEngine()->testContact(
            this->getComponent<Collider>(portal),
            this->getComponent<Transform>(portal).position
        );
        if (colWithPortal.size() > 0)
        {
            int rights = 0;
            for (int i = 0; i < colWithPortal.size(); i++) 
            {
                for (int p = 0; p < getPlayerSize(); p++){
                    if(colWithPortal[i] == getPlayer(p))
                    {
                        rights++;
                    }
                }
            }
            if (rights == getPlayerSize())
            {
                std::cout << "r:" << rights << std::endl;
                std::cout << "To a new World" << std::endl;
                addEvent({(int)GameEvent::NEXT_LEVEL});
                ((NetworkSceneHandler*)this->getSceneHandler())->sendPacketNow();
                ((NetworkSceneHandler*)this->getSceneHandler())->setScene(new ServerGameMode(++this->level));
            }
        }
    }
	// Send data to player
    makeDataSendToClient();

	//DEBUG ONLY
#ifdef ROOMDEBUG
    const auto& rooms = roomHandler.getRooms();
    for (int i = 0; i <rooms.size(); i++)
    {
        /*for (int d = 0; d < 4; d++)
        {
            if (rooms[i].doors[d] != -1)
            {
                glm::vec3 dp = this->getComponent<Transform>(rooms[i].doors[d]).position;
                 addEvent({(int)NetworkEvent::DEBUG_DRAW_BOX}, {dp.x, dp.y, dp.z, 0.f, 0.f, 0.f, 10.f, 10.f, 10.f});
            }    
        }*/
		
        for (auto ent : rooms[i].objects)
        {
            if (hasComponents<Collider>(ent) && isActive(ent))
            {
                glm::vec3 dp = this->getComponent<Transform>(ent).position;
                auto& col = getComponent<Collider>(ent);
                auto ex = col.extents * 2.f;
                switch (col.type)
                {
                default:
                    break;
                case ColType::BOX:
                    addEvent({(int)NetworkEvent::DEBUG_DRAW_BOX}, {dp.x, dp.y, dp.z, 0.f, 0.f, 0.f, ex.x, ex.y, ex.z});
                    break;
                case ColType::SPHERE:
                    addEvent({(int)NetworkEvent::DEBUG_DRAW_SPHERE}, {dp.x, dp.y, dp.z, col.radius});
                    break;
                }
            }
        }

        for (auto ent : rooms[i].doors)
        {
            if (ent != -1 && isActive(ent))
            {
                if (this->hasComponents<Collider>(ent))
                {
                    glm::vec3 dp = this->getComponent<Transform>(ent).position;
                    auto& col = getComponent<Collider>(ent);
                    auto ex = col.extents * 2.f;
                    addEvent({(int)NetworkEvent::DEBUG_DRAW_BOX}, {dp.x, dp.y, dp.z, 0.f, 0.f, 0.f, ex.x, ex.y, ex.z});
                }
            }
        }
    }
    auto& paths = this->roomHandler.getPaths();
    for (size_t i = 0; i < paths.size(); i++)
    {
        for (auto ent : paths[i].entities)
        {
            if (hasComponents<Collider>(ent) && isActive(ent))
            {
                glm::vec3 dp = this->getComponent<Transform>(ent).position;
                auto& col = getComponent<Collider>(ent);
                auto ex = col.extents * 2.f;
                addEvent({(int)NetworkEvent::DEBUG_DRAW_BOX}, {dp.x, dp.y, dp.z, 0.f, 0.f, 0.f, ex.x, ex.y, ex.z});
                
            }
        }
    }
    // Don't remove, used for debug
    /*for (auto ent : spawnHandler.allEntityIDs)
    {
        if (hasComponents<Collider>(ent))
        {
            glm::vec3 dp = this->getComponent<Transform>(ent).position;
            auto& col = getComponent<Collider>(ent);
            auto ex = col.extents * 2.f;
            switch (col.type)
            {
            default:
                break;
            case ColType::BOX:
                addEvent({(int)NetworkEvent::DEBUG_DRAW_BOX}, {dp.x, dp.y, dp.z, 0.f, 0.f, 0.f, ex.x, ex.y, ex.z});
                break;
            case ColType::SPHERE:
                addEvent({(int)NetworkEvent::DEBUG_DRAW_SPHERE}, {dp.x, dp.y, dp.z, col.radius});
                break;
            }
        }
    }*/
    for (int i = 0; i < this->getPlayerSize(); i++)
    {
		glm::vec3 p = this->getComponent<Transform>(this->getPlayer(i)).position;
		Collider cp = this->getComponent<Collider>(this->getPlayer(i));
        p += cp.offset;
        addEvent({(int)NetworkEvent::DEBUG_DRAW_CYLINDER}, {
			p.x, 
			p.y, 
			p.z, 
			0.f, 
			0.f, 
			0.f, 
			11.f,
			2.f
			});
	}
#endif  // ROOMDEBUG
}

void ServerGameMode::makeDataSendToClient() 
{
    int nrOfMonsters = 0;
    auto& reg = getSceneReg();

    reg.view<FSMAgentComponent>(entt::exclude<Inactive>).each([&](const auto& entity, FSMAgentComponent& t) 
    {
        nrOfMonsters++;
    });
    this->addEventUdp({(int)GameEvent::UPDATE_MONSTER});
    this->addEventUdp({(int)nrOfMonsters});
    
    // Get the position and rotation of monsters
    int i = 0;
    reg.view<FSMAgentComponent>(entt::exclude<Inactive>).each([&](const auto& entity, FSMAgentComponent& FSMAC) 
    {
        const Transform &t = this->getComponent<Transform>(static_cast<int>(entity));
        this->addEventUdp({static_cast<int>(entity)} ,{
            t.position.x,
            t.position.y,
            t.position.z,
            t.rotation.x,
            t.rotation.y,
            t.rotation.z,
            t.scale.x,
            t.scale.y,
            t.scale.z
            });
        //update hp if needed
        if (this->hasComponents<SwarmComponent>(static_cast<int>(entity)))
        {
            if (lastSwarmHp[i].health != this->getComponent<SwarmComponent>(static_cast<int>(entity)).life)
            {
                this->addEvent(
                    {(int)GameEvent::ENTITY_SET_HP,
                     (int)static_cast<int>(entity),
                     (int)this->getComponent<SwarmComponent>(static_cast<int>(entity)).life}
                 );
                lastSwarmHp[i].health = this->getComponent<SwarmComponent>(static_cast<int>(entity)).life;
            }
        }
        else if (this->hasComponents<LichComponent>(static_cast<int>(entity)))
        {
            if (lastLichHp[i].health != this->getComponent<LichComponent>(static_cast<int>(entity)).life)
            {
                this->addEvent(
                    {(int)GameEvent::ENTITY_SET_HP,
                    (int)static_cast<int>(entity),
                    (int)this->getComponent<LichComponent>(static_cast<int>(entity)).life}
                 );
                lastLichHp[i].health = this->getComponent<LichComponent>(static_cast<int>(entity)).life;
            }
        }
        else if (this->hasComponents<TankComponent>(static_cast<int>(entity)))
        {
            if (lastTankHp[i].health != this->getComponent<TankComponent>(static_cast<int>(entity)).life)
            {
                this->addEvent(
                    {(int)GameEvent::ENTITY_SET_HP,
                    (int)static_cast<int>(entity),
                     (int)this->getComponent<TankComponent>(static_cast<int>(entity)).life}
                 );
                lastTankHp[i].health = this->getComponent<TankComponent>(static_cast<int>(entity)).life;
            }
        }
        i++;
            
    });

    //Check for updates in player hp and change it it should
    bool allDead = true;
    for (int i = 0; i < getPlayerSize(); i++)
    {
        HealthComp& healthComp = this->getComponent<HealthComp>(getPlayer(i));
        if (healthComp.health != lastPlayerHps[i].health)
        {
            //send that player new hp
            this->addEvent(
                {(int)GameEvent::PLAYER_SETHP,
                 getPlayer(i),
                (int)this->getComponent<HealthComp>(getPlayer(i)).health}
            );
            if (this->getComponent<HealthComp>(getPlayer(i)).health < lastPlayerHps[i].health) 
            {
                this->addEvent({
                    (int)GameEvent::PLAY_PARTICLE_P,
                    (int)ParticleTypes::BLOOD,
                    i
                });
            }
            //change lastPlayerHps
            lastPlayerHps[i].health = this->getComponent<HealthComp>(getPlayer(i)).health;
        }
        // If someone is alive set to false
        if (allDead && healthComp.health > 0)
        {
            allDead = false;
        }
    }
    if (allDead)
    {
        this->addEvent({ (int)GameEvent::END_GAME });
    }

    //DEBUG
    // Get the position and rotation of monsters
    //for (auto& it : aiHandler.FSMsEntities)
    //{
    //    for (int i = 0; i < it.second.size(); ++i)
    //    {
    //        const Transform &t = this->getComponent<Transform>(it.second[i]);
    //        const Collider &c = this->getComponent<Collider>(it.second[i]);
    //        this->addEvent({
    //            (int)NetworkEvent::DEBUG_DRAW_SPHERE
    //            },
    //            {
    //            t.position.x,
    //            t.position.y,
    //            t.position.z,
    //            c.radius
    //            });
    //    }
    //}
}

void ServerGameMode::createPortal() 
{
    int colliderID = (int)this->getResourceManager()->addCollisionShapeFromMesh("assets/models/Portal.fbx");
    std::vector<ColliderDataRes> colliders = this->getResourceManager()->getCollisionShapeFromMesh(colliderID);


    portal = this->createEntity();
    Transform& portalTransform = this->getComponent<Transform>(portal);
    portalTransform.position = this->roomHandler.getPortalPosition();
    this->setComponent<Collider>(
        portal, Collider::createSphere(18.f, glm::vec3(0.f, 15.f, 0.f), true)
    );

    Entity collisionEntity;

    for (size_t i = 0; i < colliders.size(); i++)
    {
        collisionEntity = this->createEntity();
        this->setComponent<Collider>(collisionEntity, colliders[i].col);
        Transform& t = this->getComponent<Transform>(collisionEntity);
        t.position = portalTransform.position + colliders[i].position;
        t.rotation = portalTransform.rotation + colliders[i].rotation;
    }
}

void ServerGameMode::onDisconnect(int index)
{

}

void ServerGameMode::onTriggerStay(Entity e1, Entity e2) {
    Entity player = isAPlayer(e1) ? e1 : isAPlayer(e2) ? e2 : -1;
	
	if (player -= -1) // player triggered a trigger :]
	{
		Entity other = e1 == player ? e2 : e1;
    
		//if (other == this->portal && this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1) // -1 not counting start room
		//{
		//	this->switchScene(new GameScene(), "scripts/gamescene.lua");
		//}
	}}

void ServerGameMode::onTriggerEnter(Entity e1, Entity e2) {
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
}

void ServerGameMode::onCollisionEnter(Entity e1, Entity e2) {
  if (this->hasComponents<SwarmComponent>(e1) &&
  this->hasComponents<SwarmComponent>(e2))
  {
    this->getComponent<SwarmComponent>(e1).touchedFriend = true;
    this->getComponent<SwarmComponent>(e2).touchedFriend = true;
  }
}

void ServerGameMode::onCollisionStay(Entity e1, Entity e2) {
    Entity player = isAPlayer(e1) ? e1 : isAPlayer(e2) ? e2 : -1;

  if (player != -1)  // player triggered a trigger :]
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
  }

  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
  {
    this->getComponent<SwarmComponent>(e1).touchedFriend = true;
    this->getComponent<SwarmComponent>(e2).touchedFriend = true;
  }
}

void ServerGameMode::onCollisionExit(Entity e1, Entity e2) {
  if (this->hasComponents<SwarmComponent>(e1) &&
  this->hasComponents<SwarmComponent>(e2))
  {
    this->getComponent<SwarmComponent>(e1).touchedFriend = false;
    this->getComponent<SwarmComponent>(e2).touchedFriend = false;
  }
}

int ServerGameMode::getNearestPlayer(const int& ent)
{
    int returnIndex = 0;
    float nearestLenght = glm::length(this->getComponent<Transform>(ent).position - this->getComponent<Transform>(players[0]).position);
    for (int i = 1; i < this->players.size(); i++)
    {
        float nnl = glm::length(this->getComponent<Transform>(ent).position - this->getComponent<Transform>(players[i]).position);
        if (nnl < nearestLenght && this->getComponent<HealthComp>(players[i]).health > 0.0f)
        {
            nearestLenght = nnl;
            returnIndex = i;
        }
    }
    return returnIndex;
}

void ServerGameMode::updatePlayerHp(int id, int health)
{
    // Check this, is kinda weird when done via UDP
    HealthComp& healthComp = this->getComponent<HealthComp>(getPlayer(id));
    if (healthComp.health == this->lastPlayerHps[id].health) // No current change
    {
        healthComp.health = (float)health;
        this->lastPlayerHps[id].health = (float)health;
    }
}

int ServerGameMode::spawnItem(ItemType type, int otherType, float multiplier)
{
	this->curItems.push_back({ type, otherType, multiplier });
	return (int)this->curItems.size() - 1;
}

void ServerGameMode::deleteItem(int playerID, int index, ItemType type, int otherType, float multiplier)
{
    int size = (int)this->curItems.size();
    if (index >= size || index < 0)
    {
        return;
    }
    else if (this->curItems[index].type == type && this->curItems[index].otherType == otherType && this->curItems[index].multiplier == multiplier)
    {
        sf::Packet packet;
        for (int i = 0; i < this->server->getClientCount(); i++)
        {
            packet.clear();
            int gameEvent = i == playerID ? (int)GameEvent::PICKUP_ITEM : (int)GameEvent::DELETE_ITEM;
            packet << (int)gameEvent << index << (int)this->curItems[index].type;
            this->server->sendToClientTCP(packet, i);
        }

        std::swap(this->curItems[index], this->curItems[size - 1]);
        this->curItems.pop_back();
    }
}

int ServerGameMode::spawnEnemy(int type, glm::vec3 position) 
{
    int e = this->createEntity();
    this->getComponent<Transform>(e).position = position;
    
    this->addEvent({(int)GameEvent::SPAWN_ENEMY, type, e}, {position.x, position.y, position.z});
    return e;
}

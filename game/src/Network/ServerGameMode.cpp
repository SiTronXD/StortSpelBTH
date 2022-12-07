#include "ServerGameMode.h"
#include "../Components/HealthComp.h"

//#define ROOMDEBUG false

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

    roomHandler.init(this, this->getResourceManager(), false);
    roomHandler.generate(this->roomSeed);
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

    // For now we only look at player 0
    if (this->roomHandler.playerNewRoom(this->getPlayer(0), this->getPhysicsEngine()))
    {
        std::cout << "Server: player in new(__FILE__, __LINE__) room" << std::endl;
        this->newRoomFrame = true;
        this->timeWhenEnteredRoom = Time::getTimeSinceStart();
        this->safetyCleanDone = false; 
        spawnHandler.spawnEnemiesIntoRoom();
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
        this->numRoomsCleared++;
        
        if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
        {
            std::cout << "Server: Spawn portal" << std::endl;
            this->addEvent({(int)GameEvent::SPAWN_PORTAL});
        }
    }
    //check if all players are at portal
    if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
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
                //addEvent({(int)NetworkEvent::DEBUG_DRAW_BOX}, {
                //    this->getComponent<Transform>(portal).position.x,
                //    this->getComponent<Transform>(portal).position.y,
                //    this->getComponent<Transform>(portal).position.z,
                //    this->getComponent<Transform>(portal).rotation.x,
                //    this->getComponent<Transform>(portal).rotation.y,
                //    this->getComponent<Transform>(portal).rotation.z,
                //    this->getComponent<Collider>(portal).extents.x,
                //    this->getComponent<Collider>(portal).extents.y,
                //    this->getComponent<Collider>(portal).extents.z,
                //    }
                //);
                //addEvent({(int)NetworkEvent::DEBUG_DRAW_CYLINDER}, {
                //    (float)this->getComponent<Transform>(getPlayer(0)).position.x,
                //    (float)this->getComponent<Transform>(getPlayer(0)).position.y,
                //    (float)this->getComponent<Transform>(getPlayer(0)).position.z,
                //    (float)this->getComponent<Transform>(getPlayer(0)).rotation.x,
                //    (float)this->getComponent<Transform>(getPlayer(0)).rotation.y,
                //    (float)this->getComponent<Transform>(getPlayer(0)).rotation.z,
                //    (float)this->getComponent<Collider>(getPlayer( 0)).height,
                //    (float)this->getComponent<Collider>(getPlayer( 0)).radius,
                //    }
                //);
            }
        }
    }
	// Send data to player
    makeDataSendToClient();

	//DEBUG ONLY
#ifdef ROOMDEBUG
    for (int i = 0; i < roomHandler.rooms.size(); i++)
    {
        for (int d = 0; d < 4; d++)
        {
            if (roomHandler.rooms[i].doors[d] != -1)
            {
                glm::vec3 dp = this->getComponent<Transform>(roomHandler.rooms[i].doors[d]).position;
                 addEvent({(int)NetworkEvent::DEBUG_DRAW_BOX}, {dp.x, dp.y, dp.z, 0.f, 0.f, 0.f, 10.f, 10.f, 10.f});
            }    
        }
    }
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
    for (auto& it : aiHandler.FSMsEntities)
    {
        nrOfMonsters += it.second.size();
    }
    this->addEventUdp({(int)GameEvent::UPDATE_MONSTER});
    this->addEventUdp({(int)nrOfMonsters});
    
    // Get the position and rotation of monsters
    for (auto& it : aiHandler.FSMsEntities)
    {
        for (int i = 0; i < it.second.size(); ++i)
        {
            const Transform &t = this->getComponent<Transform>(it.second[i]);
            this->addEventUdp({(int)it.second[i]} ,{
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
            if (this->hasComponents<SwarmComponent>(it.second[i]))
            {
                if (lastSwarmHp[i].health != this->getComponent<SwarmComponent>(it.second[i]).life)
                {
                    this->addEvent(
                        {(int)GameEvent::ENTITY_SET_HP,
                        (int)it.second[i],
                         this->getComponent<SwarmComponent>(it.second[i]).life}
                     );
                    lastSwarmHp[i].health = this->getComponent<SwarmComponent>(it.second[i]).life;
                }
            }
            else if (this->hasComponents<LichComponent>(it.second[i]))
            {
                if (lastLichHp[i].health != this->getComponent<LichComponent>(it.second[i]).life)
                {
                    this->addEvent(
                        {(int)GameEvent::ENTITY_SET_HP,
                        (int)it.second[i],
                        this->getComponent<LichComponent>(it.second[i]).life}
                     );
                    lastLichHp[i].health = this->getComponent<LichComponent>(it.second[i]).life;
                }
            }
            else if (this->hasComponents<TankComponent>(it.second[i]))
            {
                if (lastTankHp[i].health != this->getComponent<TankComponent>(it.second[i]).life)
                {
                    this->addEvent(
                        {(int)GameEvent::ENTITY_SET_HP,
                        (int)it.second[i],
                         this->getComponent<TankComponent>(it.second[i]).life}
                     );
                    lastTankHp[i].health = this->getComponent<TankComponent>(it.second[i]).life;
                }
            }
            
        }
    }
    //Check for updates in player hp and change it it should
    for (int i = 0; i < getPlayerSize(); i++)
    {
        if (this->getComponent<HealthComp>(getPlayer(i)).health != lastPlayerHps[i].health)
        {
            //send that player new(__FILE__, __LINE__) hp
            this->addEvent(
                {(int)GameEvent::PLAYER_SETHP,
                 getPlayer(i),
                 this->getComponent<HealthComp>(getPlayer(i)).health}
            );
            //change lastPlayerHps
            lastPlayerHps[i].health = this->getComponent<HealthComp>(getPlayer(i)).health;
        }
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
    glm::vec3 portalTriggerDims(6.f, 18.f, 1.f);
    glm::vec3 portalBlockDims(3.f, 18.f, 3.f);

    portal = this->createEntity();
    this->getComponent<Transform>(portal).position =
        this->roomHandler.getExitRoom().position;
    this->setComponent<Collider>(
        portal, Collider::createBox(portalTriggerDims, glm::vec3(0, 0, 0), true)
        );

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
		//	this->switchScene(new(__FILE__, __LINE__) GameScene(), "scripts/gamescene.lua");
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

int ServerGameMode::spawnItem(ItemType type, int otherType, float multiplier)
{
	this->curItems.push_back({ type, otherType, multiplier });
	return this->curItems.size() - 1;
}

void ServerGameMode::deleteItem(int playerID, int index, ItemType type, int otherType, float multiplier)
{
    int size = this->curItems.size();
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

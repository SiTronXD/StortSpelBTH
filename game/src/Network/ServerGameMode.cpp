#include "ServerGameMode.h"
#include "../Components/HealthComp.h"

ServerGameMode::~ServerGameMode()
{
    aiHandler.clean();
}

void ServerGameMode::init()
{
	//create seed
	#ifdef _CONSOLE
	    srand(69);
	#else
	    srand(time(NULL));
	#endif

    this->roomSeed = 123;//rand();
    aiHandler.init(this->getSceneHandler());
    this->getSceneHandler()->setAIHandler(&aiHandler);
    roomHandler.init(this, this->getResourceManager(), false);
    roomHandler.generate(this->roomSeed);
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
}

void ServerGameMode::update(float dt)
{
    aiHandler.update(dt);

    // For now we only look at player 0
    if (this->roomHandler.playerNewRoom(this->getPlayer(0), this->getPhysicsEngine()))
    {
        std::cout << "Server: player in new room" << std::endl;
        this->newRoomFrame = true;
        spawnHandler.spawnEnemiesIntoRoom();
    }

    if (this->spawnHandler.allDead() && this->newRoomFrame)
    {
        this->newRoomFrame = false;
        std::cout << "all dead" << std::endl;
        this->addEvent({(int)GameEvent::ROOM_CLEAR});
        // Call when a room is cleared
        roomHandler.roomCompleted();
        this->numRoomsCleared++;
        
        if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
        {
            this->addEvent({(int)GameEvent::SPAWN_PORTAL});
        }
    }
	// Send data to player
    makeDataSendToClient();

	//DEBUG ONLY
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
                t.rotation.z
                });
        }
    }
    //Check for updates in player hp and change it it should
    for (int i = 0; i < getPlayerSize(); i++)
    {
        if (this->getComponent<HealthComp>(getPlayer(i)).health != lastPlayerHps[i].health)
        {
            //send that player new hp
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
    for (auto& it : aiHandler.FSMsEntities)
    {
        for (int i = 0; i < it.second.size(); ++i)
        {
            const Transform &t = this->getComponent<Transform>(it.second[i]);
            const Collider &c = this->getComponent<Collider>(it.second[i]);
            this->addEvent({
                (int)NetworkEvent::DEBUG_DRAW_SPHERE
                },
                {
                t.position.x,
                t.position.y,
                t.position.z,
                c.radius
                });
        }
    }
}

void ServerGameMode::onDisconnect(int index)
{
	for (int i = 0; i < this->itemIDs.size(); i++)
	{
		this->itemIDs[i].erase(this->itemIDs[i].begin() + index);
	}
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
          auto& aiCombat = this->getComponent<AiCombatSwarm>(other);
          swarmComp.inAttack = false;
          swarmComp.touchedPlayer = true;
          //aiCombat.timer = aiCombat.lightAttackTime;
          this->getComponent<HealthComp>(player).health -=
              (int)aiCombat.lightHit;
          //this->addEvent({GameEvent::});
            
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
        this->getComponent<HealthComp>(player).health -=
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
	this->itemIDs.push_back(std::vector<Entity>(this->server->getClientCount(), -1));
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
        std::swap(this->itemIDs[index], this->itemIDs[size - 1]);
        this->curItems.pop_back();
        this->itemIDs.pop_back();
    }
}

void ServerGameMode::setEntityID(int itemID, int playerID, Entity ID)
{
	this->itemIDs[itemID][playerID] = ID;
}

Entity ServerGameMode::getEntityID(int itemID, int playerID)
{
	return this->itemIDs[itemID][playerID];
}

int ServerGameMode::spawnEnemy(int type, glm::vec3 position) 
{
    int e = this->createEntity();
    this->getComponent<Transform>(e).position = position;
    
    this->addEvent({(int)GameEvent::SPAWN_ENEMY, type, e}, {position.x, position.y, position.z});
    return e;
}

#include "ServerGameMode.h"
#include "../Components/NetworkCombat.h"

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

    this->roomSeed = rand();
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

    for (int i = 0; i < this->getPlayerSize(); i++)
    {
        this->setComponent<NetworkCombat>(getPlayer(i));
    }
}

void ServerGameMode::update(float dt)
{
    aiHandler.update(dt);

    //for now we only look at player 0
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
	//Send data to player
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
    size_t nrOfMonsters = 0;
    for (auto& it : aiHandler.FSMsEntities)
    {
        nrOfMonsters += it.second.size();
    }
    this->addEventUdp({(int)GameEvent::UPDATE_MONSTER});
    this->addEventUdp({nrOfMonsters});
    
     //get the position and rotation of monsters
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

    //DEBUG
     //get the position and rotation of monsters
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

int ServerGameMode::spawnItem(ItemType type, int otherType, float multiplier)
{
	this->curItems.push_back({ type, otherType, multiplier });
	this->itemIDs.push_back(std::vector<Entity>(this->server->getClientCount(), -1));
	return this->curItems.size() - 1;
}

void ServerGameMode::deleteItem(int playerID, Entity ID)
{
	int index = -1;
	int size = this->curItems.size();
	for (int i = 0; i < size; i++)
	{
		if (this->itemIDs[i][playerID] == ID)
		{
			index = i;
			break;
		}
	}
	if (index != -1)
	{
		sf::Packet packet;
		for (int i = 0; i < this->server->getClientCount(); i++)
		{
			packet.clear();
			int gameEvent = i == playerID ? (int)GameEvent::PICKUP_ITEM : (int)GameEvent::DELETE_ITEM;
			packet << (int)gameEvent << itemIDs[index][i] << (int)this->curItems[index].type;
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

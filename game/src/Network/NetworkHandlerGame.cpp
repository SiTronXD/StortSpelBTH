#include "NetworkHandlerGame.h"
#include "../Systems/CombatSystem.hpp"
#include "ServerGameMode.h"
#include "../Scenes/GameScene.h"

const float NetworkHandlerGame::UPDATE_RATE = ServerUpdateRate;

Entity NetworkHandlerGame::spawnItem(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir)
{
	Scene* scene = this->sceneHandler->getScene();
	Perks perk{ .multiplier = 0.2f, .perkType = type };

	Entity e = scene->createEntity();
	scene->setComponent<MeshComponent>(e, perkMeshes[type]);

	Transform& perkTrans = sceneHandler->getScene()->getComponent<Transform>(e);
	perkTrans.position = pos;
	perkTrans.scale = glm::vec3(2.0f);
	scene->setComponent<Collider>(e, Collider::createSphere(2.0f, glm::vec3(0.0f), true));

	if (shootDir != glm::vec3(0.0f))
	{
		scene->setComponent<Rigidbody>(e);
		Rigidbody& perkRb = sceneHandler->getScene()->getComponent<Rigidbody>(e);
		perkRb.gravityMult = 6.0f;
		perkRb.velocity = shootDir * 20.0f;
	}

	scene->setComponent<Perks>(e, perk);
	scene->setComponent<PointLight>(e, glm::vec3(0.0f), glm::vec3(5.0f, 7.0f, 9.0f));
	scene->setScriptComponent(e, "scripts/spin.lua");

	return e;
}

Entity NetworkHandlerGame::spawnItem(AbilityType type, glm::vec3 pos, glm::vec3 shootDir)
{
	Scene* scene = this->sceneHandler->getScene();
	Entity e = scene->createEntity();
	scene->setComponent<MeshComponent>(e, abilityMeshes[type]);

	Transform& perkTrans = sceneHandler->getScene()->getComponent<Transform>(e);
	perkTrans.position = pos;
	perkTrans.scale = glm::vec3(2.0f);
	scene->setComponent<Collider>(e, Collider::createSphere(4.0f, glm::vec3(0.0f), true));

	if (shootDir != glm::vec3(0.0f))
	{
		scene->setComponent<Rigidbody>(e);
		Rigidbody& perkRb = sceneHandler->getScene()->getComponent<Rigidbody>(e);
		perkRb.gravityMult = 6.0f;
		perkRb.velocity = shootDir * 20.0f;
	}

	scene->setComponent<Abilities>(e, type);
	scene->setComponent<PointLight>(e, glm::vec3(0.0f), glm::vec3(7.0f, 9.0f, 5.0f));
	scene->setScriptComponent(e, "scripts/spin.lua");

	return e;
}

Entity NetworkHandlerGame::spawnEnemy(const int& type, const glm::vec3& pos) {
    int e = sceneHandler->getScene()->createEntity();
    switch (type)
        {
            case 0:
				//load blob
                sceneHandler->getScene()->setScriptComponent(e, "scripts/loadBlob.lua");
                break;
            case 1:
				//load lich
                sceneHandler->getScene()->setScriptComponent(e, "scripts/loadBlob.lua");
                break;
            case 2:
				//load tank
                sceneHandler->getScene()->setScriptComponent(e, "scripts/loadBlob.lua");
                break;
            default:
                break;
        }
    return e;
}

void NetworkHandlerGame::setCombatSystem(CombatSystem* system)
{
	combatSystem = system;
}

void NetworkHandlerGame::init()
{
	this->perkMeshes[0] = this->resourceManger->addMesh("assets/models/Perk_Hp.obj");
	this->perkMeshes[1] = this->resourceManger->addMesh("assets/models/Perk_Dmg.obj");
	this->perkMeshes[2] = this->resourceManger->addMesh("assets/models/Perk_AtkSpeed.obj");
	this->perkMeshes[3] = this->resourceManger->addMesh("assets/models/Perk_Movement.obj");
	this->perkMeshes[4] = this->resourceManger->addMesh("assets/models/Perk_Stamina.obj");
	this->abilityMeshes[0] = this->resourceManger->addMesh("assets/models/KnockbackAbility.obj");
	this->abilityMeshes[1] = this->resourceManger->addMesh("assets/models/KnockbackAbility.obj");
}

void NetworkHandlerGame::cleanup()
{
}

void NetworkHandlerGame::handleTCPEventClient(sf::Packet& tcpPacket, int event)
{
	sf::Packet packet;
	Entity e;
	switch ((GameEvent)event)
	{
	case GameEvent::SPAWN_ITEM:
		tcpPacket >> i0 >> i1 >> i2 >> f0;
		v0 = this->getVec(tcpPacket);
		v1 = this->getVec(tcpPacket);
		if ((ItemType)i1 == ItemType::PERK)
		{
			e = this->spawnItem((PerkType)i2, f0, v0, v1);
		}
		else
		{
			e = this->spawnItem((AbilityType)i2, v0, v1);
		}

		packet << (int)GameEvent::SET_ITEM_ID << i0 << e;
		this->sendDataToServerTCP(packet);
		break;
	case GameEvent::PICKUP_ITEM:
		// EntityID -> ItemType
		tcpPacket >> i0 >> i1;
		if ((ItemType)i1 == ItemType::PERK)
		{
			this->combatSystem->pickupPerk(i0);
		}
		else
		{
			this->combatSystem->pickUpAbility(i0);
		}
		break;
	case GameEvent::DELETE_ITEM:
		// EntityID -> ItemType
		tcpPacket >> i0 >> i1;
		this->sceneHandler->getScene()->removeEntity(i0);
		break;
    case GameEvent::SPAWN_ENEMY:
        tcpPacket >> i0 >> i1;
        v0 = this->getVec(tcpPacket);
        serverEnteties.insert(std::pair<int, int>(i1, spawnEnemy(i0, v0)));
		break;
    case GameEvent::PUSH_PLAYER://can't confimr yet if this works
        tcpPacket >> i0; 
		v0 = this->getVec(tcpPacket);
        this->sceneHandler->getScene()->getComponent<Rigidbody>(player).velocity = v0;
        this->sceneHandler->getScriptHandler()->setScriptComponentValue(
            this->sceneHandler->getScene()->getComponent<Script>(player),
            1.0f,
            "pushTimer"
        );
        break;
    case GameEvent::PLAYER_SETHP:
        tcpPacket >> i0 >> i1;
        if (i0 == ID)
        {
			this->sceneHandler->getScene()->getComponent<Combat>(player).health = i1;   
		}
		//else give hp to other players visually
		break;
	default:
		break;
	}
}

void NetworkHandlerGame::handleUDPEventClient(sf::Packet& udpPacket, int event)
{
	sf::Packet packet;
	glm::vec3 vec;
	Transform* t;
	AnimationComponent* anim;
	switch ((GameEvent)event)
	{
	case GameEvent::UPDATE_PLAYER:
		this->timer = 0;
		udpPacket >> i0;
		i1 = -1;
		for (int i = 0; i < this->otherPlayersServerId.size(); i++)
		{
			if (this->otherPlayersServerId[i] == i0)
			{
				i1 = i;
				break;
			}
		}
		if (!this->playerEntities.size()) // Not ready
		{
			udpPacket.clear();
			break;
		}
		if (i1 == -1)
		{
			Log::warning("Client: invalid client id from server: " + std::to_string(i0));
			udpPacket.clear();
			break;
		}

		this->playerPosLast[i1] = this->playerPosCurrent[i1];
		this->playerPosCurrent[i1] = this->getVec(udpPacket);

		t = &this->sceneHandler->getScene()->getComponent<Transform>(this->playerEntities[i1]);
		t->rotation = this->getVec(udpPacket);

		anim = &this->sceneHandler->getScene()->getComponent<AnimationComponent>(this->playerEntities[i1]);
		udpPacket >> i0 >> anim->timer >> anim->timeScale;
		anim->animationIndex = (uint32_t)i0;
		break;
    case GameEvent::UPDATE_MONSTER:
        //how many monsters we shall update
        udpPacket >> i0;
        for (int i = 0; i < i0; i++)
        {
			//the monster id on server side
			udpPacket >> i1;

			if (serverEnteties.find(i1) == serverEnteties.end())
            {
				udpPacket.clear();
                break;
			}
			//get and set position and rotation
            v0 = getVec(udpPacket);
            v1 = getVec(udpPacket);
            sceneHandler->getScene()->getComponent<Transform>(serverEnteties.find(i1)->second).position = v0;
            sceneHandler->getScene()->getComponent<Transform>(serverEnteties.find(i1)->second).rotation = v1;

			//get and set animation // don't know how this should be made
			//anim = &this->sceneHandler->getScene()->getComponent<AnimationComponent>(serverEnteties.find(i1)->second);
			//udpPacket >> i2 >> anim->timer >> anim->timeScale;
			//anim->animationIndex = (uint32_t)i2;
		}
		break;
	default:
		break;
	}
}

void NetworkHandlerGame::handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event)
{
	sf::Packet packet;
	ServerGameMode* serverScene;
	switch ((GameEvent)event)
	{
	case GameEvent::SPAWN_ITEM:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0 >> si1 >> sf0;
		sv0 = this->getVec(tcpPacket);
		sv1 = this->getVec(tcpPacket);
		si2 = serverScene->spawnItem((ItemType)si0, si1, sf0);

		// event <- itemID <- ItemType <- otherType <- multiplier
		packet << (int)GameEvent::SPAWN_ITEM << si2 << si0 << si1 << sf0;
		this->sendVec(packet, sv0);
		this->sendVec(packet, sv1);
		server->sendToAllClientsTCP(packet);
		break;
	case GameEvent::SET_ITEM_ID:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0 >> si1;
		serverScene->setEntityID(si0, clientID, si1);
		break;
	case GameEvent::PICKUP_ITEM:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0;
		serverScene->deleteItem(clientID, si0);
		break;
    case GameEvent::MONSTER_TAKE_DAMAGE:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0 >> si1 >> sf0;
		//get how they should take damage
        std::cout << "monster take damage" << std::endl;
        if (serverScene->hasComponents<SwarmComponent>(si0))
        {
			serverScene->getComponent<SwarmComponent>(si0).life -= si1;  
		}
        else if (serverScene->hasComponents<TankComponent>(si0))
        {
			serverScene->getComponent<TankComponent>(si0).life -= si1;  
		}
        else if (serverScene->hasComponents<LichComponent>(si0))
        {
			serverScene->getComponent<LichComponent>(si0).life -= si1;  
		}
		break;
	default:
		packet << event;
		server->sendToAllClientsTCP(packet);
		break;
	}
}

void NetworkHandlerGame::handleUDPEventServer(Server* server, int clientID, sf::Packet& udpPacket, int event)
{
	sf::Packet packet;
	ServerGameMode* serverScene;
	NetworkScene* scene;
	switch ((GameEvent)event)
	{
	case GameEvent::UPDATE_PLAYER:
        scene = server->getScene<NetworkScene>();
		packet << (int)GameEvent::UPDATE_PLAYER << clientID;
		sv0 = this->getVec(udpPacket);
		this->sendVec(packet, sv0);
        
        scene->getComponent<Transform>(scene->getPlayer(clientID)).position = sv0;
		sv0 = this->getVec(udpPacket);
		this->sendVec(packet, sv0);

		udpPacket >> si0 >> sf0 >> sf1;
		packet << si0 << sf0 << sf1;

		server->sendToAllOtherClientsUDP(packet, clientID);
		break;
	default:
		packet << event;
		server->sendToAllClientsUDP(packet);
		break;
	}
}

void NetworkHandlerGame::onDisconnect(int index)
{
	if (this->getStatus() == ServerStatus::RUNNING)
	{
		Entity ID = this->playerEntities[index];
		this->playerEntities.erase(this->playerEntities.begin() + index);
		this->sceneHandler->getScene()->removeEntity(ID);
	}
}

void NetworkHandlerGame::sendHitOn(int entityID, int damage, float knockBack)
{
    for (auto it = serverEnteties.begin(); it != serverEnteties.end(); ++it)
    {
        if (it->second == entityID)
        {
			//probably need where from
            sf::Packet p;
            p << (int)GameEvent::MONSTER_TAKE_DAMAGE << it->first << damage << knockBack;
            sendDataToServerTCP(p);
		}
	}
}

void NetworkHandlerGame::setPlayerEntity(Entity player)
{
	this->player = player;
}

void NetworkHandlerGame::createOtherPlayers(int playerMesh)
{
	int size = this->otherPlayersServerId.size();
	this->playerEntities.resize(size);
	this->playerPosLast.resize(size);
	this->playerPosCurrent.resize(size);
	float angle = 360.0f / (size + 1);

	Scene* scene = this->sceneHandler->getScene();
	Transform& playerTrans = scene->getComponent<Transform>(this->player);
	playerTrans.position = SMath::rotateVector(glm::vec3(0.0f, angle * this->ID, 0.0f), glm::vec3(10.0f, 12.0f, 0.0f));
	for (int i = 0; i < size; i++)
	{
		this->playerEntities[i] = scene->createEntity();
		scene->setComponent<MeshComponent>(this->playerEntities[i], playerMesh);
		scene->setComponent<AnimationComponent>(this->playerEntities[i]);
		scene->setComponent<Collider>(this->playerEntities[i], Collider::createCapsule(2, 11, glm::vec3(0, 7.3, 0)));

		// Set Position
		Transform& t = scene->getComponent<Transform>(this->playerEntities[i]);
		t.position = playerTrans.position = SMath::rotateVector(glm::vec3(0.0f, angle * this->otherPlayersServerId[i], 0.0f), glm::vec3(10.0f, 12.0f, 0.0f));

		// Set tint color
		MeshComponent& mesh = scene->getComponent<MeshComponent>(this->playerEntities[i]);
		this->resourceManger->makeUniqueMaterials(mesh);
		mesh.overrideMaterials[0].tintColor = this->playerColors[i + 1];

		/*scene->setComponent<Rigidbody>(this->playerEntities[i]);

		Rigidbody& rb = scene->getComponent<Rigidbody>(this->playerEntities[i]);
		rb.gravityMult = 5;
		rb.friction = 0.1f;
		rb.rotFactor = glm::vec3(0);*/
	}
}

void NetworkHandlerGame::updatePlayer()
{
	if (this->getClient()->getAccumulatedTime() + Time::getDT() >= UPDATE_RATE)
	{
		sf::Packet packet;
		Transform& t = this->sceneHandler->getScene()->getComponent<Transform>(this->player);
		AnimationComponent& anim = this->sceneHandler->getScene()->getComponent<AnimationComponent>(this->player);

		packet << (int)GameEvent::UPDATE_PLAYER <<
			t.position.x << t.position.y << t.position.z <<
			t.rotation.x << t.rotation.y << t.rotation.z <<
			(int)anim.animationIndex << anim.timer << anim.timeScale;
		this->sendDataToServerUDP(packet);
	}
}

void NetworkHandlerGame::interpolatePositions()
{
	Scene* scene = this->sceneHandler->getScene();
	this->timer += Time::getDT();

	float percent = this->timer / UPDATE_RATE;
	for (int i = 0; i < this->playerEntities.size(); i++)
	{
		Transform& t = scene->getComponent<Transform>(this->playerEntities[i]);
		t.position = this->playerPosLast[i] + percent * (this->playerPosCurrent[i] - this->playerPosLast[i]);
	}
}

void NetworkHandlerGame::spawnItemRequest(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir)
{
	if (!this->playerEntities.empty()) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::SPAWN_ITEM << (int)ItemType::PERK << type << multiplier;
		this->sendVec(packet, pos);
		this->sendVec(packet, shootDir);
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		this->spawnItem(type, multiplier, pos, shootDir);
	}
}

void NetworkHandlerGame::spawnItemRequest(AbilityType type, glm::vec3 pos, glm::vec3 shootDir)
{
	if (!this->playerEntities.empty()) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::SPAWN_ITEM << (int)ItemType::ABILITY << type << 0.0f;
		this->sendVec(packet, pos);
		this->sendVec(packet, shootDir);
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		this->spawnItem(type, pos, shootDir);
	}
}

void NetworkHandlerGame::pickUpItemRequest(Entity itemEntity, ItemType type)
{
	if (!this->playerEntities.empty()) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::PICKUP_ITEM << itemEntity;
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		if (type == ItemType::PERK)
		{
			this->combatSystem->pickupPerk(itemEntity);
		}
		else
		{
			this->combatSystem->pickUpAbility(itemEntity);
		}
	}
}

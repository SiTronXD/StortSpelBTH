#include "NetworkHandlerGame.h"
#include "../Systems/CombatSystem.hpp"
#include "ServerGameMode.h"
#include "../Scenes/GameScene.h"

const float NetworkHandlerGame::UPDATE_RATE = ServerUpdateRate;

Entity NetworkHandlerGame::spawnItem(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir)
{
	Scene* scene = this->sceneHandler->getScene();
	Perks perk{ .multiplier = multiplier, .perkType = type };

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
		// Load blob
		this->sceneHandler->getScene()->setScriptComponent(e, "scripts/loadBlob.lua");
		this->sceneHandler->getScene()->setComponent<SwarmComponent>(e);
		break;
	case 1:
		// Load lich
		this->sceneHandler->getScene()->setScriptComponent(e, "scripts/loadBlob.lua");
		this->sceneHandler->getScene()->setComponent<LichComponent>(e);
		break;
	case 2:
		// Load tank
		this->sceneHandler->getScene()->setScriptComponent(e, "scripts/loadBlob.lua");
		this->sceneHandler->getScene()->setComponent<TankComponent>(e);
		break;
	default:
		break;
	}
    return e;
}

Entity NetworkHandlerGame::spawnHealArea(glm::vec3 pos)
{
	Scene* scene = this->sceneHandler->getScene();
	Entity heal = scene->createEntity();

	scene->setComponent<MeshComponent>(heal, this->healAreaMesh);
	scene->setComponent<PointLight>(heal, { glm::vec3(0.f, 10.f, 0.f), glm::vec3(9.f, 7.f, 9.f) });
	scene->setComponent<HealArea>(heal);
	scene->getComponent<Transform>(heal).position = pos;

	return heal;
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
	this->healAreaMesh = this->resourceManger->addMesh("assets/models/HealingAbility.obj");
	this->swordMesh = this->resourceManger->addMesh("assets/models/MainSword.fbx", "assets/textures");
}

void NetworkHandlerGame::cleanup()
{
}

void NetworkHandlerGame::handleTCPEventClient(sf::Packet& tcpPacket, int event)
{
	sf::Packet packet;
	switch ((GameEvent)event)
	{
	case GameEvent::SPAWN_ITEM:
		tcpPacket >> i0 >> i1 >> i2 >> f0;
		v0 = this->getVec(tcpPacket);
		v1 = this->getVec(tcpPacket);
		if ((ItemType)i1 == ItemType::PERK)
		{
			this->itemIDs.push_back(this->spawnItem((PerkType)i2, f0, v0, v1));
		}
		else
		{
			this->itemIDs.push_back(this->spawnItem((AbilityType)i2, v0, v1));
		}
		break;
	case GameEvent::PICKUP_ITEM:
		// Index -> ItemType
		tcpPacket >> i0 >> i1;
		if (this->sceneHandler->getScene()->entityValid(this->itemIDs[i0]))
		{
			if ((ItemType)i1 == ItemType::PERK)
			{
				this->combatSystem->pickupPerk(this->itemIDs[i0]);
			}
			else
			{
				this->combatSystem->pickUpAbility(this->itemIDs[i0]);
			}
			std::swap(this->itemIDs[i0], this->itemIDs[this->itemIDs.size() - 1]);
			this->itemIDs.pop_back();
		}
		break;
	case GameEvent::DELETE_ITEM:
		// Index -> ItemType
		tcpPacket >> i0 >> i1;
		this->sceneHandler->getScene()->removeEntity(this->itemIDs[i0]);
		std::swap(this->itemIDs[i0], this->itemIDs[this->itemIDs.size() - 1]);
		this->itemIDs.pop_back();
		break;
	case GameEvent::USE_HEAL :
		v0 = this->getVec(tcpPacket);
		this->spawnHealArea(v0);
		break;
    case GameEvent::SPAWN_ENEMY:
        tcpPacket >> i0 >> i1;
        v0 = this->getVec(tcpPacket);
		serverEntities.insert(std::pair<int, int>(i1, spawnEnemy(i0, v0)));
		break;
    case GameEvent::PUSH_PLAYER: // Can't confirm yet if this works
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
			this->sceneHandler->getScene()->getComponent<HealthComp>(player).health = i1;   
		}
		// Else give hp to other players visually
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
        udpPacket >> i0 >> anim->aniSlots[0].timer >> anim->aniSlots[0].timeScale;
		anim->aniSlots[0].animationIndex = (uint32_t)i0;
		break;
    case GameEvent::UPDATE_MONSTER:
        // How many monsters we shall update
        udpPacket >> i0;
        for (int i = 0; i < i0; i++)
        {
			// The monster id on server side
			udpPacket >> i1;

			if (serverEntities.find(i1) == serverEntities.end())
            {
				udpPacket.clear();
                break;
			}
			// Get and set position and rotation
            v0 = getVec(udpPacket);
            v1 = getVec(udpPacket);
            sceneHandler->getScene()->getComponent<Transform>(serverEntities.find(i1)->second).position = v0;
            sceneHandler->getScene()->getComponent<Transform>(serverEntities.find(i1)->second).rotation = v1;

			// Get and set animation // don't know how this should be made
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
	case GameEvent::PICKUP_ITEM:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0 >> si1 >> si2 >> sf0;
		serverScene->deleteItem(clientID, si0, (ItemType)si1, si2, sf0);
		break;

	case GameEvent::USE_HEAL:
		sv0 = this->getVec(tcpPacket);
		packet << (int)GameEvent::USE_HEAL;
		this->sendVec(packet, sv0);
		server->sendToAllClientsTCP(packet);
		break;
    case GameEvent::MONSTER_TAKE_DAMAGE:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> si0 >> si1 >> sf0;
		// Get how they should take damage
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

		Entity swordID = this->swords[index];
		this->swords.erase(this->swords.begin() + index);
		this->sceneHandler->getScene()->removeEntity(swordID);
	}
}

void NetworkHandlerGame::sendHitOn(int entityID, int damage, float knockBack)
{
    for (auto it = serverEntities.begin(); it != serverEntities.end(); ++it)
    {
        if (it->second == entityID)
        {
			//probably need where from
            sf::Packet p;
            p << (int)GameEvent::MONSTER_TAKE_DAMAGE << it->first << damage << knockBack;
            sendDataToServerTCP(p);
            return;
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
	this->swords.resize(size);
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

		// Sword
		this->swords[i] = scene->createEntity();
		scene->setComponent<MeshComponent>(this->swords[i], swordMesh);

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
			(int)anim.aniSlots[0].animationIndex << anim.aniSlots[0].timer << anim.aniSlots[0].timeScale;
		this->sendDataToServerUDP(packet);
	}
}

void NetworkHandlerGame::interpolatePositions()
{
	Scene* scene = this->sceneHandler->getScene();
	UIRenderer* UI = this->sceneHandler->getUIRenderer();
	this->timer += Time::getDT();

	float percent = this->timer / UPDATE_RATE;
	for (int i = 0; i < this->playerEntities.size(); i++)
	{
		Transform& t = scene->getComponent<Transform>(this->playerEntities[i]);
		t.position = this->playerPosLast[i] + percent * (this->playerPosCurrent[i] - this->playerPosLast[i]);
		UI->renderString(this->otherPlayers[i].second, t.position + glm::vec3(0.0f, 20.0f, 0.0f) + t.forward(), glm::vec2(150.0f));

		// Put sword in characters hand and keep updating it
		scene->getComponent<Transform>(this->swords[i]).setMatrix(
			this->resourceManger->getJointTransform(
				scene->getComponent<Transform>(this->playerEntities[i]),
				scene->getComponent<MeshComponent>(this->playerEntities[i]),
				scene->getComponent<AnimationComponent>(this->playerEntities[i]),
				"mixamorig:RightHand") * glm::translate(glm::mat4(1.f), glm::vec3(0.f, 1.f, 0.f)) *
			glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));
	}
}

void NetworkHandlerGame::spawnItemRequest(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir)
{
	if (this->isConnected()) // Multiplayer (send to server)
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
	if (this->isConnected()) // Multiplayer (send to server)
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
	if (this->isConnected()) // Multiplayer (send to server)
	{
		int index = -1;
		for (int i = 0; i < this->itemIDs.size(); i++)
		{
			if (this->itemIDs[i] == itemEntity)
			{
				index = i;
				break;
			}
		}
		sf::Packet packet;
		packet << (int)GameEvent::PICKUP_ITEM << index;

		Scene* scene = this->sceneHandler->getScene();
		if (scene->hasComponents<Perks>(itemEntity))
		{
			Perks& perk = scene->getComponent<Perks>(itemEntity);
			packet << (int)ItemType::PERK << perk.perkType << perk.multiplier;
			this->sendDataToServerTCP(packet);
		}
		else if (scene->hasComponents<Abilities>(itemEntity))
		{
			Abilities& ability = scene->getComponent<Abilities>(itemEntity);
			packet << (int)ItemType::ABILITY << ability.abilityType << 0.0f;
			this->sendDataToServerTCP(packet);
		}
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

void NetworkHandlerGame::useHealAbilityRequest(glm::vec3 position)
{
	if (this->isConnected()) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::USE_HEAL;
		this->sendVec(packet, position);
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		this->spawnHealArea(position);
	}
}

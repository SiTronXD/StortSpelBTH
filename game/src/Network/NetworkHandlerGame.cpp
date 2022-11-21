#include "NetworkHandlerGame.h"
#include "../Systems/CombatSystem.hpp"
#include "ServerGameMode.h"
#include "../Scenes/GameScene.h"

const float NetworkHandlerGame::UPDATE_RATE = ServerUpdateRate;

Entity NetworkHandlerGame::spawnItem(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 spawnDir)
{
	Scene* scene = this->sceneHandler->getScene();
	Perks perk{ .multiplier = 0.2f, .perkType = type };

	Entity perkEnt = scene->createEntity();
	scene->setComponent<MeshComponent>(perkEnt, perkMeshes[type]);

	Transform& perkTrans = sceneHandler->getScene()->getComponent<Transform>(perkEnt);
	perkTrans.position = pos;
	perkTrans.scale = glm::vec3(2.0f);
	scene->setComponent<Collider>(perkEnt, Collider::createSphere(2.0f, glm::vec3(0.0f), true));

	if (spawnDir != glm::vec3(0.0f))
	{
		scene->setComponent<Rigidbody>(perkEnt);
		Rigidbody& perkRb = sceneHandler->getScene()->getComponent<Rigidbody>(perkEnt);
		perkRb.gravityMult = 6.0f;
		perkRb.velocity = spawnDir * 20.0f;
	}

	scene->setComponent<Perks>(perkEnt, perk);
	scene->setComponent<PointLight>(perkEnt, glm::vec3(5.f, 7.f, 9.f));
	scene->setScriptComponent(perkEnt, "scripts/spin.lua");

	return perkEnt;
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
	this->abilityMeshes[1] = this->resourceManger->addMesh("assets/models/HealingAbility.obj");
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
		e = this->spawnItem((PerkType)i2, f0, v0, v1);

		packet << (int)GameEvent::SET_ITEM_ID << i0 << e;
		this->sendDataToServerTCP(packet);
		break;
	case GameEvent::PICKUP_ITEM:
		// EntityID -> ItemType
		tcpPacket >> i0 >> i1;
		if (i1 == (int)ItemType::PERK)
		{
			this->combatSystem->pickupPerk(i0);
		}
		/*else
		{

		}*/
		break;
	case GameEvent::DELETE_ITEM:
		// EntityID -> ItemType
		tcpPacket >> i0 >> i1;
		this->sceneHandler->getScene()->removeEntity(i0);
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
		tcpPacket >> i0 >> i1 >> f0;
		i2 = serverScene->spawnItem((ItemType)i0, i1, f0);
		v0 = this->getVec(tcpPacket);
		v1 = this->getVec(tcpPacket);

		// event <- itemID <- ItemType <- otherType <- multiplier
		packet << (int)GameEvent::SPAWN_ITEM << i2 << i0 << i1 << f0;
		this->sendVec(packet, v0);
		this->sendVec(packet, v1);
		server->sendToAllClientsTCP(packet);
		break;
	case GameEvent::SET_ITEM_ID:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> i0 >> i1;
		serverScene->setEntityID(i0, clientID, i1);
		break;
	case GameEvent::PICKUP_ITEM:
		serverScene = server->getScene<ServerGameMode>();
		tcpPacket >> i0;
		serverScene->deleteItem(clientID, i0);
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
	switch ((GameEvent)event)
	{
	case GameEvent::UPDATE_PLAYER:
		packet << (int)GameEvent::UPDATE_PLAYER << clientID;
		v0 = this->getVec(udpPacket);
		this->sendVec(packet, v0);
		v0 = this->getVec(udpPacket);
		this->sendVec(packet, v0);

		udpPacket >> i0 >> f0 >> f1;
		packet << i0 << f0 << f1;

		server->sendToAllOtherClientsUDP(packet, clientID);
		break;
	default:
		packet << event;
		server->sendToAllClientsUDP(packet);
		break;
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

	Scene* scene = this->sceneHandler->getScene();
	for (int i = 0; i < size; i++)
	{
		this->playerEntities[i] = scene->createEntity();
		scene->setComponent<MeshComponent>(this->playerEntities[i], playerMesh);
		scene->setComponent<AnimationComponent>(this->playerEntities[i]);
		scene->setComponent<Collider>(this->playerEntities[i], Collider::createCapsule(2, 11, glm::vec3(0, 7.3, 0)));
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
	float percent = this->getClient()->getAccumulatedTime() / UPDATE_RATE;
	for (int i = 0; i < this->playerEntities.size(); i++)
	{
		Transform& t = scene->getComponent<Transform>(this->playerEntities[i]);
		t.position = this->playerPosLast[i] + percent * (this->playerPosCurrent[i] - this->playerPosLast[i]);
	}
}

void NetworkHandlerGame::pickUpItem(PerkType type, float multiplier)
{
}

void NetworkHandlerGame::spawnItemRequest(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 spawnDir)
{
	if (/*!this->playerEntities.empty()*/true) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::SPAWN_ITEM << (int)ItemType::PERK << type << multiplier;
		this->sendVec(packet, pos);
		this->sendVec(packet, spawnDir);
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		this->spawnItem(type, multiplier, pos, spawnDir);
	}
}

void NetworkHandlerGame::pickUpItemRequest(Entity itemEntity)
{
	if (/*!this->playerEntities.empty()*/true) // Multiplayer (send to server)
	{
		sf::Packet packet;
		packet << (int)GameEvent::PICKUP_ITEM << itemEntity;
		this->sendDataToServerTCP(packet);
	}
	else // Singleplayer
	{
		this->combatSystem->pickupPerk(itemEntity);
	}
}

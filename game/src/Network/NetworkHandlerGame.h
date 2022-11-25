#pragma once

#include <vengine.h>
#include "../Components/Perks.h"
#include "../Components/Abilities.h"

class CombatSystem;
enum class GameEvent
{
	EMPTY = (int)NetworkEvent::END + 1,
	UPDATE_PLAYER, // Positions and animations
	UPDATE_MONSTER, // how many enemies, What enemy, Position, rotation and animation udp
	SPAWN_ITEM, // Client -> Server: Want to spawn item. Server -> Client: Spawn item in scene
	DELETE_ITEM, // Server -> Client: Remove item from scene
	PICKUP_ITEM, // Client -> Server: Want to pick up item. Server -> Client: Pick up the item
	SET_ITEM_ID, // Client -> Server: Set entity ID to store in server scene
	USE_HEAL, // Client -> Server: Want to use heal. Server -> Client: Spawn heal entity
	SPAWN_ENEMY,//Type, ServerID, Position,  

	ROOM_CLEAR,
	SPAWN_PORTAL,

};

enum class ItemType
{
	PERK,
	ABILITY,
};

class NetworkHandlerGame : public NetworkHandler
{
public:
	inline static const glm::vec4 playerColors[]
	{
		glm::vec4(1.0f, 1.0f, 1.0f, 0.25f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.25f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.25f),
		glm::vec4(1.0f, 1.0f, 0.0f, 0.25f),
	};
private:
	static const float UPDATE_RATE;
	float timer = 0.0f;

	CombatSystem* combatSystem;

	Entity player; // Own player
	std::vector<Entity> playerEntities; // Other players connected
    std::map<int, Entity> serverEnteties;

	// Interpolation of other transforms
	std::vector<glm::vec3> playerPosLast;
	std::vector<glm::vec3> playerPosCurrent;

	// Client helpers
	int i0, i1, i2;
	float f0, f1, f2;
	glm::vec3 v0, v1, v2;

	// Server helpers
	int si0, si1, si2;
	float sf0, sf1, sf2;
	glm::vec3 sv0, sv1, sv2;

	int perkMeshes[PerkType::emptyPerk];
	int abilityMeshes[AbilityType::emptyAbility];

	Entity spawnItem(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	Entity spawnItem(AbilityType type, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	Entity spawnEnemy(const int& type, const glm::vec3& pos);
public:
	void init();
	void cleanup();

	void setCombatSystem(CombatSystem* system);

	virtual void handleTCPEventClient(sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventClient(sf::Packet& udpPacket, int event) override;
	virtual void handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventServer(Server* server, int clientID, sf::Packet& udpPacket, int event) override;
	virtual void onDisconnect(int index) override;

	void setPlayerEntity(Entity player);
	void createOtherPlayers(int playerMesh);
	void updatePlayer();
	void interpolatePositions();

	void spawnItemRequest(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	void spawnItemRequest(AbilityType type, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	void pickUpItemRequest(Entity itemEntity, ItemType type);
};


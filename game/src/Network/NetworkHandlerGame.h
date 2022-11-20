#pragma once

#include <vengine.h>
#include "../Components/Perks.h"
#include "../Components/Abilities.h"

class CombatSystem;

enum class GameEvent
{
	EMPTY = (int)NetworkEvent::END + 1,
	UPDATE_PLAYER, // Positions and animations
	SPAWN_ITEM, // Client -> Server: Want to spawn item. Server -> Client: Spawn item in scene
	DELETE_ITEM, // Server -> Client: Remove item from scene
	PICK_ITEM, // Client -> Server: Want to pick up item. Server -> Client: Pick up the item
	SET_ITEM_ID, // Client -> Server: Set entity ID to store in server scene
};

enum class ItemType
{
	PERK,
	ABILITY,
};

class NetworkHandlerGame : public NetworkHandler
{
private:
	static const float UPDATE_RATE;

	CombatSystem* combatSystem;

	Entity player; // Own player
	std::vector<Entity> playerEntities; // Other players connected

	// Interpolation of other transforms
	std::vector<glm::vec3> playerPosLast;
	std::vector<glm::vec3> playerPosCurrent;

	int i0, i1, i2;
	float f0, f1, f2;
	glm::vec3 v0, v1, v2;

	int perkMeshes[PerkType::emptyPerk];
	int abilityMeshes[AbilityType::emptyAbility];

	Entity spawnItem(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 spawnDir = glm::vec3(0.0f));
	void pickUpItem(PerkType type, float multiplier);
public:
	void init();
	void cleanup();

	void setCombatSystem(CombatSystem* system);

	virtual void handleTCPEventClient(sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventClient(sf::Packet& udpPacket, int event) override;
	virtual void handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventServer(Server* server, int clientID, sf::Packet& udpPacket, int event) override;

	void setPlayerEntity(Entity player);
	void createOtherPlayers(int playerMesh);
	void updatePlayer();
	void interpolatePositions();

	void spawnItemRequest(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 spawnDir = glm::vec3(0.0f));
	void pickUpItemRequest(Entity itemEntity);
};


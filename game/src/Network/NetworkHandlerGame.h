#pragma once

#include <vengine.h>
#include "../Components/Perks.h"
#include "../Components/Abilities.h"
#include "../Ai/Behaviors/Lich/LichBTs.hpp"
#include "../Ai/Behaviors/Lich/LichFSM.hpp"
#include "../World Handling/Room Handler.h"

class CombatSystem;
enum class GameEvent
{
	EMPTY = (int)NetworkEvent::END + 1,
	SEED, // Client -> Server: Request seed, Server -> Client: Seed to use
	UPDATE_PLAYER, // Positions and animations
	UPDATE_MONSTER, // How many enemies, What enemy, Position, rotation and animation udp
	SPAWN_ITEM, // Client -> Server: Want to spawn item. Server -> Client: Spawn item in scene
	DELETE_ITEM, // Server -> Client: Remove item from scene
	PICKUP_ITEM, // Client -> Server: Want to pick up item. Server -> Client: Pick up the item
	USE_HEAL, // Client -> Server: Want to use heal. Server -> Client: Spawn heal entity
	SPAWN_ENEMY,// Type, ServerID, Position,
    SPAWN_ORB, // id, Type, Position
    SPAWN_OBJECT, // id, type, position, rotation, scale 
    SPAWN_GROUND_HUMP, // nrOf, id
    DO_HUMP, // id, position 
    UPDATE_HUMP, // id, position 
    SET_POS_OBJECT, // id, position
    THROW_ORB, // Id, initialPosition, Direction
	PLAYER_TAKE_DAMAGE, // What player, how much damage
	PLAYER_SETHP, // What player, how much hp
	ENTITY_SET_HP, //What entity, how much hp
	PUSH_PLAYER, // What player, direction
	MONSTER_TAKE_DAMAGE,
	INACTIVATE, //what entity
	ACTIVATE, //what entity
	PLAY_ENEMY_SOUND, // What entity, What component type

	ROOM_CLEAR,
	SPAWN_PORTAL,

};

enum class ItemType
{
	PERK,
	ABILITY,
};

enum class ObjectTypes
{
    LICH_GRAVE,
    LICH_ALTER
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
	int seed = -1;

	CombatSystem* combatSystem;

	Entity player; // Own player
	std::vector<Entity> playerEntities; // Other players connected
	std::vector<Entity> swords; // Other player swords

    std::map<int, Entity> serverEntities;
	std::vector<Entity> itemIDs;

	// Interpolation of other transforms
	std::vector<glm::vec3> playerPosLast;
	std::vector<glm::vec3> playerPosCurrent;
    std::map<int, std::pair<glm::vec3, glm::vec3>> entityToPosScale;
    std::map<int, std::pair<glm::vec3, glm::vec3>> entityLastPosScale;

	// Client helpers
	int i0, i1, i2, i3;
	float f0, f1, f2;
	glm::vec3 v0, v1, v2;

	// Server helpers
	int si0, si1, si2, si3, si4, si5;
	float sf0, sf1, sf2;
	glm::vec3 sv0, sv1, sv2;

	int perkMeshes[PerkType::emptyPerk];
	int abilityMeshes[AbilityType::emptyAbility];
	int healAreaMesh;
	int swordMesh;
    int graveMesh;
    int alterMesh;
    int humpMesh;

    bool newRoomFrame;
    int numRoomsCleared;
    RoomHandler* roomHandler;

    static LichAttack* lich_fire   ;
    static LichAttack* lich_ice    ;
    static LichAttack* lich_light  ;

    Entity spawnOrbs(int orbType);
    Entity spawnItem(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	Entity spawnItem(AbilityType type, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
    Entity spawnObject(
        const ObjectTypes& type, const glm::vec3& pos, const glm::vec3& rot,
        const glm::vec3& scale
    );
    Entity spawnHealArea(glm::vec3 pos);
    Entity createHump();

	Entity spawnEnemy(const int& type, const glm::vec3& pos);
public:
	void init();
	void cleanup();

	void setCombatSystem(CombatSystem* system);
	int getSeed();
    void setRoomHandler(RoomHandler& roomHandler);

	virtual void handleTCPEventClient(sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventClient(sf::Packet& udpPacket, int event) override;
	virtual void handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventServer(Server* server, int clientID, sf::Packet& udpPacket, int event) override;
	virtual void onDisconnect(int index) override;

	void sendHitOn(int entityID, int damage, float knockBack);

	void setPlayerEntity(Entity player);
	void createOtherPlayers(int playerMesh);
	void updatePlayer();
	void interpolatePositions();

	void spawnItemRequest(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	void spawnItemRequest(AbilityType type, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	void pickUpItemRequest(Entity itemEntity, ItemType type);
	void useHealAbilityRequest(glm::vec3 position);
};


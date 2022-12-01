#pragma once

#include <vengine.h>
#include "../Components/Perks.h"
#include "../Components/Abilities.h"
#include "../Ai/Behaviors/Lich/LichBTs.hpp"
#include "../Ai/Behaviors/Lich/LichFSM.hpp"

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
    SET_POS_OBJECT, // id, position
    THROW_ORB, // Id, initialPosition, Direction
	PLAYER_TAKE_DAMAGE, // What player, how much damage
	PLAYER_SETHP, // What player, how much hp
	PUSH_PLAYER, // What player, direction
	MONSTER_TAKE_DAMAGE,
	INACTIVATE, //what entity
	ACTIVATE, //what entity

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
	int healAreaMesh;
	int swordMesh;
    int graveMesh;
    int alterMesh;

    static LichAttack* lich_fire   ;
    static LichAttack* lich_ice    ;
    static LichAttack* lich_light  ;

    Entity spawnOrbs(int orbType);
    Entity spawnItem(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	Entity spawnItem(AbilityType type, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
    Entity spawnObject(const ObjectTypes& type, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale)
    {
        Entity entity = this->sceneHandler->getScene()->createEntity();
        switch(type) 
        {
            case ObjectTypes::LICH_ALTER:                 
                this->sceneHandler->getScene()->setComponent<MeshComponent>(entity, this->alterMesh);
                this->sceneHandler->getScene()->setComponent<Collider>(entity, 
                    Collider::createBox(
                        glm::vec3{LichComponent::alterWidth,LichComponent::alterHeight,LichComponent::alterDepth}));                                
                
                break;
            case ObjectTypes::LICH_GRAVE: 
                this->sceneHandler->getScene()->setComponent<MeshComponent>(entity, this->graveMesh);                            
                this->sceneHandler->getScene()->setComponent<Collider>(entity, Collider::createBox(
                        glm::vec3{LichComponent::graveWidth,LichComponent::graveHeight,LichComponent::graveDepth}));
                break;
            default: 
                break;
        }
        this->sceneHandler->getScene()->getComponent<Transform>(entity).position = pos;
        this->sceneHandler->getScene()->getComponent<Transform>(entity).rotation = rot;
        this->sceneHandler->getScene()->getComponent<Transform>(entity).scale = scale;
        return entity;
    }
	Entity spawnHealArea(glm::vec3 pos);

	Entity spawnEnemy(const int& type, const glm::vec3& pos);
public:
	void init();
	void cleanup();

	void setCombatSystem(CombatSystem* system);
	int getSeed();

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


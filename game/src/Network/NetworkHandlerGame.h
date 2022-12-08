#pragma once

#include <vengine.h>
#include "../Components/Perks.h"
#include "../Components/Abilities.h"
#include "../Ai/Behaviors/Lich/LichBTs.hpp"
#include "../Ai/Behaviors/Lich/LichFSM.hpp"
#include "../World Handling/Room Handler.h"
#include "../World Handling/ParticleSystemGenerator.hpp"

class CombatSystem;
enum class GameEvent
{
	EMPTY = (int)NetworkEvent::END + 1,
	SEED, // Client -> Server: Request seed, Server -> Client: Seed to use
	UPDATE_PLAYER, // Positions and animations (and health to server)
	UPDATE_MONSTER, // How many enemies, What enemy, Position, rotation and animation udp
	PLAY_PARTICLE, //What type, entity
	PLAY_PARTICLE_P, //What type, player
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
    THROW_ORB, // id, initialPosition, Direction
	PLAYER_TAKE_DAMAGE, // What player, how much damage
	PLAYER_SETHP, // What player, how much hp
	ENTITY_SET_HP, // What entity, how much hp
	PUSH_PLAYER, // What player, direction
	MONSTER_TAKE_DAMAGE,
	INACTIVATE, //what entity
	ACTIVATE, //what entity
	PLAY_ENEMY_SOUND, // What entity, What component type
	PLAY_PLAYER_SOUND, // client -> server : soundIndex, volume, // server -> client : playerID, soundIndex, volume
	UPDATE_ANIM, // What entity, type (tank/lich), animIndex, slot
	UPDATE_ANIM_TIMESCALE, // What entity, slot, timeScale
	PLAYER_SET_GHOST, // Player ID

	ROOM_CLEAR,
	SPAWN_PORTAL,
	NEXT_LEVEL,// CurrentLevel difficulty, 
	END_GAME, // All players dead
	CLOSE_OLD_DOORS,// Index of next room
	CLOSE_NEW_DOORS,// :)
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

enum class ParticleTypes
{
	HEAL,
	BLOOD,
	SWARM
};

class NetworkHandlerGame : public NetworkHandler
{
public:
	inline static const glm::vec4 playerColors[]
	{
		glm::vec4(1.0f, 1.0f, 1.0f, 0.15f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.15f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.15f),
		glm::vec4(1.0f, 1.0f, 0.0f, 0.15f),
	};

	inline static const std::string tankAnims[]
	{
		"Walk",
		"Charge",
		"GroundHump",
		"RaiseShield",
	};
private:
	static const float UPDATE_RATE;
	float timer = 0.0f;
	int seed = -1;

	CombatSystem* combatSystem;
	Material* ghostMat;
	Material origMat;

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
    std::vector<float> currDistToStepSound;
    inline static const float distToStepSound = 20.f;//I don't know why this is perfect but it is
    uint32_t moveSound;

	// Client helpers
	std::string str;
	int i0, i1, i2, i3;
	float f0, f1, f2;
	glm::vec3 v0, v1, v2;

	// Server helpers
	int si0, si1, si2, si3, si4, si5;
	float sf0, sf1, sf2;
	glm::vec3 sv0, sv1, sv2;
    
	// Meshes
	int perkMeshes[PerkType::emptyPerk];
	int abilityMeshes[AbilityType::emptyAbility];
	int healAreaMesh;
	int swordMesh;
    int graveMesh;
    int alterMesh;
    int humpMesh;

	// Particles
    bool deletedParticleSystems;
    ParticleSystemInstance healParticleSystem;
    ParticleSystemInstance bloodParticleSystems;
    ParticleSystemInstance swarmParticleSystems;
    ParticleSystemInstance portalParticleSystemSide0;
    ParticleSystemInstance portalParticleSystemSide1;
    void playParticle(const ParticleTypes& particleType, Entity& entity);

	// RoomHandler
    bool newRoomFrame;
    int* numRoomsCleared;
    RoomHandler* roomHandler;

    LichAttack* lich_fire;
    LichAttack* lich_ice;
    LichAttack* lich_light;

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
    ~NetworkHandlerGame();
	void init();
	void cleanUp() override;
    void initParticleSystems();
    void deleteInitialParticleSystems();

	void setCombatSystem(CombatSystem* system);
	void setGhostMat(Material* ghostMat);
	int getSeed();
    void setRoomHandler(RoomHandler& roomHandler, int& numRoomsCleared);

	virtual void handleTCPEventClient(sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventClient(sf::Packet& udpPacket, int event) override;
	virtual void handleTCPEventServer(Server* server, int clientIndex, sf::Packet& tcpPacket, int event) override;
	virtual void handleUDPEventServer(Server* server, int clientIndex, sf::Packet& udpPacket, int event) override;
	virtual void onDisconnect(int index) override;

	void sendHitOn(int entityID, int damage, float knockBack);

	void setPlayerEntity(Entity player);
	void createOtherPlayers(int playerMesh);

	void updatePlayer();
	void interpolatePositions();

	inline const ParticleSystem& getHealParticleSystem() { return this->healParticleSystem.getParticleSystem(); }
	inline const ParticleSystem& getBloodParticleSystem() { return this->bloodParticleSystems.getParticleSystem(); }
	inline const ParticleSystem& getSwarmParticleSystem() { return this->swarmParticleSystems.getParticleSystem(); }
	inline const ParticleSystem& getPortalParticleSystem0() { return this->portalParticleSystemSide0.getParticleSystem(); }
	inline const ParticleSystem& getPortalParticleSystem1() { return this->portalParticleSystemSide1.getParticleSystem(); }

	void spawnItemRequest(PerkType type, float multiplier, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	void spawnItemRequest(AbilityType type, glm::vec3 pos, glm::vec3 shootDir = glm::vec3(0.0f));
	void pickUpItemRequest(Entity itemEntity, ItemType type);
	void useHealAbilityRequest(glm::vec3 position);
	void setGhost();
    void setPerks(const Perks perk[]);
};


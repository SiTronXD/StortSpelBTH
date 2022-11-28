#pragma once

#include <vengine.h>
#include "../Components/Perks.h"
#include "../Components/Abilities.h"
#include "NetworkHandlerGame.h"

#include "../World Handling/Room Handler.h"
#include "../World Handling/SpawnHandler.hpp"

class ServerGameMode : public NetworkScene
{
private:
	struct Item
	{
		ItemType type;
		int otherType; // Perk type, ability type...
		float multiplier;
	};

	std::vector<Item> curItems;

	// itemIDs[ItemIndex][ClientID]
	std::vector<std::vector<Entity>> itemIDs;
    std::vector<int> entities;
    std::vector<NetworkCombat> lastPlayerHps;

	AIHandler aiHandler;
    SpawnHandler spawnHandler;
    RoomHandler roomHandler;
    int roomSeed;
    uint8_t numRoomsCleared;
    bool newRoomFrame;

	void makeDataSendToClient();
  public:
    virtual ~ServerGameMode();
	virtual void init() override;
	void update(float dt) override;
	virtual void onDisconnect(int index) override;

	virtual void onTriggerStay(Entity e1, Entity e2) override;
    virtual void onTriggerEnter(Entity e1, Entity e2) override;
    virtual void onCollisionEnter(Entity e1, Entity e2) override;
    virtual void onCollisionStay(Entity e1, Entity e2) override;
    virtual void onCollisionExit(Entity e1, Entity e2) override;

	// Returns itemID
	int spawnItem(ItemType type, int otherType, float multiplier = 0.0f);
	void deleteItem(int playerID, Entity ID);
	void setEntityID(int itemID, int playerID, Entity ID);
	Entity getEntityID(int itemID, int playerID);
    int spawnEnemy(int type = -1, glm::vec3 position = glm::vec3(0,-100,0));
};


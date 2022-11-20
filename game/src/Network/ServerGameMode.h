#pragma once

#include <vengine.h>
#include "../Components/Perks.h"
#include "../Components/Abilities.h"
#include "NetworkHandlerGame.h"

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
public:
	virtual void init() override;
	void update(float dt) override;

	// Returns itemID
	int spawnItem(ItemType type, int otherType, float multiplier);
	bool deleteItem(int playerID, Entity ID);
	void setEntityID(int itemID, int playerID, Entity ID);
	Entity getEntityID(int itemID, int playerID);
};


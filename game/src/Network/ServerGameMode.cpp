#include "ServerGameMode.h"

void ServerGameMode::init()
{
}

void ServerGameMode::update(float dt)
{
}

int ServerGameMode::spawnItem(ItemType type, int otherType, float multiplier)
{
	this->curItems.push_back({ type, otherType, multiplier });
	this->itemIDs.push_back(std::vector<Entity>(this->server->getClientCount(), -1));
	return this->curItems.size() - 1;
}

bool ServerGameMode::deleteItem(int playerID, Entity ID)
{
	int index = -1;
	int size = this->curItems.size();
	for (int i = 0; i < size; i++)
	{
		if (this->itemIDs[i][playerID] == ID)
		{
			index = i;
			break;
		}
	}
	if (index != -1)
	{
		for (int i = 0; i < this->server->getClientCount(); i++)
		{
			sf::Packet packet;
			packet << (int)GameEvent::DELETE_ITEM << itemIDs[index][i] <<
				(int)this->curItems[index].type <<
				this->curItems[index].otherType <<
				this->curItems[index].multiplier;
			this->server->sendToClientTCP(packet, i);
		}

		std::swap(this->curItems[index], this->curItems[size - 1]);
		std::swap(this->itemIDs[index], this->itemIDs[size - 1]);
		this->curItems.pop_back();
		this->itemIDs.pop_back();
	}

	return index;
}

void ServerGameMode::setEntityID(int itemID, int playerID, Entity ID)
{
	this->itemIDs[itemID][playerID] = ID;
}

Entity ServerGameMode::getEntityID(int itemID, int playerID)
{
	return this->itemIDs[itemID][playerID];
}

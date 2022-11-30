#pragma once
#include "vengine/network/ServerEngine/NetworkScene.h"
#include <unordered_map>

struct playerNetworkData
{
    int playerHP;
};

class NetworkDataFetcher
{
private:
    NetworkScene* netScene;

    std::unordered_map<uint32_t, playerNetworkData> playerData;

public:
    NetworkDataFetcher(){}
    void init(NetworkScene* netScene)
    {
        this->netScene = netScene;
    }
    void fetchData()
    {

    }
    void updateData()
    {
        int playerID;
        for(int i = 0; i < this->netScene->getPlayerSize(); i++){
            this->netScene->getPlayer(i);
        }
        
        // this->netScene->addEvent({(int)GameEvents::GetPlayerHP, playerID});
    }
};
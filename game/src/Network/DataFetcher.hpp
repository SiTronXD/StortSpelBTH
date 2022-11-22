#pragma once
#include "vengine/network/ServerEngine/NetworkScene.h"
#include "vengine/network/ServerEngine/NetworkSceneHandler.h"
#include "../Scenes/GameScene.h"
#include <unordered_map>

struct playerNetworkData
{
    int playerHP;
};


class DataFetcher
{
private:    
    static struct DataFetcherSingleton
    {
    private:    
        NetworkSceneHandler* netSceneHandler = nullptr;
        GameScene* gameScene                 = nullptr;
        SceneHandler* sceneHandler;

        std::unordered_map<uint32_t, playerNetworkData> playerData;

    public:        
        void initSingleplayer(GameScene* gameScene)
        {
            this->netSceneHandler = nullptr;
            this->gameScene = gameScene;
        }
        void initNetwork(NetworkSceneHandler* netSceneHandler)
        {
            this->gameScene = nullptr;
            this->netSceneHandler = netSceneHandler;
        }
        void fetchData()
        {

        }
        void updateData()
        {
            int playerID;
            for(int i = 0; i < this->netSceneHandler->getScene()->getPlayerSize(); i++){
                this->netSceneHandler->getScene()->getPlayer(i);
            }
            
            //this->netScene->addEvent({(int)GameEvents::GetPlayerHP, playerID});
        }

        template<typename T>
        T& getComponent(Entity entityID)
        {
            
            if(netSceneHandler)
            {
                std::cout << "Not Implemented!\n";
                //this->netSceneHandler->getCallFromClient();
                
            }
            else
            {
                return this->sceneHandler->getScene()->getComponent<T>(entityID);
            }
        }
    } instance;
    DataFetcher() = default;
public: 

    static void initSingleplayer(GameScene *gameScene)
    {
        DataFetcher::instance.initSingleplayer(gameScene);
    }
    static void initNetwork(NetworkSceneHandler* netSceneHandler)
    {
        DataFetcher::instance.initNetwork(netSceneHandler);
    }
    static void updateData()
    {
        DataFetcher::instance.updateData();
    }

    template<typename T>
    static T& getComponent(Entity entityID)
    {
        return DataFetcher::instance.getComponent<T>(entityID);
    }
    
};
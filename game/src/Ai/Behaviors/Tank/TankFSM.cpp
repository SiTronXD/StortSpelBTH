#include "TankFSM.hpp"
#include "../../../Components/Combat.h"
#define getTankComponent() FSM::sceneHandler->getScene()->getComponent<TankComponent>(entityID)
#define getPlayerID(playerID) std::string playerId_str = "playerID";FSM::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str)
#define getPlayerTrans(playerID) FSM::sceneHandler->getScene()->getComponent<Transform>(playerID) 
#define getPlayerCombat(playerID) FSM::sceneHandler->getScene()->getComponent<Combat>(playerID) 
#define getTankTrans() FSM::sceneHandler->getScene()->getComponent<Transform>(entityID) 
#define falseIfDead() TankComponent& tankComp_____macro = getTankComponent();if(tankComp_____macro.isDead()) {return false;}
#define getTheScene() FSM::sceneHandler->getScene()

void TankFSM::updateFriendsInSight(Entity entityID, Scene* scene)
{
    TankComponent& tankComp = scene->getComponent<TankComponent>(entityID);
    tankComp.friendsInSight.clear();
    Transform& tankTransform = scene->getComponent<Transform>(entityID);;
    auto viewSwarm = scene->getSceneReg().view<SwarmComponent, Transform>();
    auto viewLich = scene->getSceneReg().view<LichComponent, Transform>();
    auto swarmLamda = [&](const auto& entity, SwarmComponent& comp, Transform& trans) {
        int entityid = (int)entity;
        if(scene->isActive(entityid) && entityid != entityID)
        {
            bool found = false;
            float dist = glm::length(tankTransform.position - trans.position);
            if(dist < tankComp.sightRadius)
            {
                for(auto f: tankComp.friendsInSight)
                {
                    if(f == entityid)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    tankComp.friendsInSight.emplace_back(entityid);
                }
            }
        }        
    };
    auto lichLamda = [&](const auto& entity, LichComponent& comp, Transform& trans) {
        int entityid = (int)entity;
        if(scene->isActive(entityid) && entityid != entityID)
        {
            bool found = false;
            float dist = glm::length(tankTransform.position - trans.position);
            if(dist < tankComp.sightRadius)
            {
                for(auto f: tankComp.friendsInSight)
                {
                    if(f == entityid)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    tankComp.friendsInSight.push_back(entityid);
                }
            }
        }
    };
    viewSwarm.each(swarmLamda);
    viewLich.each(lichLamda);
}

bool TankFSM::idleToAler(Entity entityID)
{
    falseIfDead();

    updateFriendsInSight(entityID, getTheScene());
    std::cout<<"Oh I have "<<getTankComponent().friendsInSight.size()<<" Friends\n";

    return false;
}

bool TankFSM::alertToCombat(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::alertToShield(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::combatToIdel(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::combatToShield(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::shieldToCombat(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::shieldToIdle(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::toDead(Entity entityID)
{
    falseIfDead();
    return false;
}


#include "TankFSM.hpp"
#include "../../../Components/Combat.h"
#define getTankComponent() FSM::sceneHandler->getScene()->getComponent<TankComponent>(entityID)
#define getPlayerID(playerID) std::string playerId_str = "playerID";FSM::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str)
#define getPlayerTrans(playerID) FSM::sceneHandler->getScene()->getComponent<Transform>(playerID) 
#define getPlayerCombat(playerID) FSM::sceneHandler->getScene()->getComponent<Combat>(playerID) 
#define getTankTrans() FSM::sceneHandler->getScene()->getComponent<Transform>(entityID) 
#define falseIfDead() TankComponent& tankComp_____macro = getTankComponent();if(tankComp_____macro.isDead()) {return false;}if(!getTheScene()->isActive(entityID)){return false;}
#define getTheScene() FSM::sceneHandler->getScene()

void TankFSM::updateFriendsInSight(Entity entityID)
{
    Scene* scene = getTheScene();
    TankComponent& tankComp = scene->getComponent<TankComponent>(entityID);
    tankComp.friendsInSight.clear();
    Transform& tankTransform = scene->getComponent<Transform>(entityID);;
    auto viewSwarm = scene->getSceneReg().view<SwarmComponent, Transform>();
    auto viewLich = scene->getSceneReg().view<LichComponent, Transform>();
    auto swarmLamda = [&](const auto& entity, SwarmComponent& comp, Transform& trans) {
        int entityid = (int)entity;
        if(scene->isActive(entityid) && entityid != entityID)
        {
            tankComp.allFriends.insert({entityid, "Swarm"});
            bool found = false;
            float dist = glm::length(tankTransform.position - trans.position);
            if(dist < tankComp.sightRadius)
            {
                for(auto f: tankComp.friendsInSight)
                {
                    if(f.first == entityid)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    tankComp.friendsInSight.insert({entityid, "Swarm"});
                }
            }
        }        
    };
    auto lichLamda = [&](const auto& entity, LichComponent& comp, Transform& trans) {
        int entityid = (int)entity;
        if(scene->isActive(entityid) && entityid != entityID)
        {
            tankComp.allFriends.insert({entityid, "Lich"});
            bool found = false;
            float dist = glm::length(tankTransform.position - trans.position);
            if(dist < tankComp.sightRadius)
            {
                for(auto f: tankComp.friendsInSight)
                {
                    if(f.first == entityid)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    tankComp.friendsInSight.insert({entityid, "Lich"});
                }
            }
        }
    };
    viewSwarm.each(swarmLamda);
    viewLich.each(lichLamda);

    std::vector<int> groups;
    std::vector<int> groups_swarmID;
    std::vector<int> toRemove;
    for(auto f: tankComp.friendsInSight)
    {
        if(f.second == "Swarm")
        {
            bool found = false;
            int groupID = scene->getComponent<SwarmComponent>(f.first).group->myId;
            for(auto g: groups)
            {
                if(g == groupID)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                groups.push_back(groupID);
                groups_swarmID.push_back(f.first);
            }
            toRemove.push_back(f.first);
            
        }
    }
    for(auto tr: toRemove)
    {
        tankComp.friendsInSight.erase(tr);
    }
    for(auto g: groups_swarmID)
    {
        tankComp.friendsInSight.insert({g, "Swarm"});
    }

    groups.clear();
    groups_swarmID.clear();
    toRemove.clear();

    for(auto f: tankComp.allFriends)
    {
        if(f.second == "Swarm")
        {
            bool found = false;
            int groupID = scene->getComponent<SwarmComponent>(f.first).group->myId;
            for(auto g: groups)
            {
                if(g == groupID)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                groups.push_back(groupID);
                groups_swarmID.push_back(f.first);
            }
            toRemove.push_back(f.first);
            
        }
    }
    for(auto tr: toRemove)
    {
        tankComp.allFriends.erase(tr);
    }
    for(auto g: groups_swarmID)
    {
        tankComp.allFriends.insert({g, "Swarm"});
    }
}

bool TankFSM::playerInSight(Entity entityID)
{
    TankComponent& tankComp = getTankComponent();
    int playerID = -1;
    getPlayerID(playerID);
    Transform& playerTrans  = getPlayerTrans(playerID);
    Transform& tankTrans    = getTankTrans();
    float tank_player_dist = glm::length(playerTrans.position - tankTrans.position);
    if(tank_player_dist <= tankComp.sightRadius)
    {
        return true;
    }
    return false;
}

bool TankFSM::friendlysInFight(Entity entityID)
{
    TankComponent& tankComp = getTankComponent();
    Scene* theScene = getTheScene();
    for(auto f: tankComp.friendsInSight)
    {
        if(theScene->hasComponents<SwarmComponent>(f.first))
        {
            if(theScene->getComponent<SwarmComponent>(f.first).group->inCombat)
            {
                return true;
            }
        }
        else if(theScene->hasComponents<LichComponent>(f.first))
        {
             if(theScene->getComponent<LichComponent>(f.first).inCombat)
            {
                return true;
            }
        }
    }
    return false;
}

bool TankFSM::idleToAler(Entity entityID)
{
    falseIfDead();
    bool ret = false;

    updateFriendsInSight(entityID);
    if(playerInSight(entityID))
    {
        ret = true;
    }
    else if(friendlysInFight(entityID))
    {
        ret = true;
    }

    return ret;
}

bool TankFSM::alertToCombat(Entity entityID)
{
    falseIfDead();
    bool ret = false;
    updateFriendsInSight(entityID);  
    TankComponent& tankComp = getTankComponent();
    if (tankComp.alertTimer <= 0 && tankComp.friendsInSight.size() <= 0)
    {
        tankComp.alertTimer = tankComp.alertTimerOrig;
        ret = true;
    }
    else
    {
        tankComp.alertTimer -= Time::getDT();
    }

    return ret;
}

bool TankFSM::alertToShield(Entity entityID)
{
    falseIfDead();
    bool ret = false;
    updateFriendsInSight(entityID);   
    TankComponent& tankComp = getTankComponent();
    if (tankComp.alertTimer <= 0 && tankComp.friendsInSight.size() > 0)
    {
        tankComp.alertTimer = tankComp.alertTimerOrig;
        ret = true;
    }
    else
    {
        tankComp.alertTimer -= Time::getDT();
    }
    return ret;
}

bool TankFSM::combatToIdel(Entity entityID)
{
    falseIfDead();
    bool ret = false;
    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTankComponent();
    if(!playerInSight(entityID) && !tankComp.inCombat && tankComp.huntTimer <= 0)
    {
        tankComp.huntTimer = tankComp.huntTimerOrig;
        ret = true;
    }
    else
    {
        tankComp.huntTimer -= Time::getDT();
    }


    return ret;
}

bool TankFSM::combatToShield(Entity entityID)
{
    falseIfDead();
    bool ret = false;
    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTankComponent();
    if((playerInSight(entityID) || tankComp.inCombat) && tankComp.friendsInSight.size() > 0)
    {
        ret = true;
    }

    return ret;
}

bool TankFSM::shieldToCombat(Entity entityID)
{
    falseIfDead();
    bool ret = false;
    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTankComponent();
    if((playerInSight(entityID) || tankComp.inCombat) && tankComp.friendsInSight.size() <= 0)
    {
        ret = true;
    }
    return ret;
}

bool TankFSM::shieldToIdle(Entity entityID)
{
    falseIfDead();
    bool ret = false;
    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTankComponent();
    if(!playerInSight(entityID) && !tankComp.inCombat && !friendlysInFight(entityID))
    {
        ret = true;
    }
    return ret;
}

bool TankFSM::toDead(Entity entityID)
{
    bool ret = false;
    TankComponent& tankComp = getTankComponent();
    if(tankComp.life <= 0)
    {
        ret = true;
    }
    return ret;
}
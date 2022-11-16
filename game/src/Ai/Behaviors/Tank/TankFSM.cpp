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
    Transform& tankTransform = scene->getComponent<Transform>(entityID);

    std::vector<int> toRemove;
    std::vector<int> toAddID;
    std::vector<TankFriend> toAddData;
    for(auto f: tankComp.allFriends)
    {
        if(f.second.type == "Swarm")
        {
            SwarmComponent& swarmComp = scene->getComponent<SwarmComponent>(f.first);
            if(swarmComp.life <= 0)
            {
                
                for(auto g: swarmComp.group->members)
                {
                    if(scene->getComponent<SwarmComponent>(g).life > 0)
                    {
                        toAddID.push_back(g);
                        toAddData.push_back({f.second.type, f.second.visited});
                        break;
                    }
                }
                
                toRemove.push_back(f.first);
                continue;
            }
        }
        else if(f.second.type == "Lich")
        {
            LichComponent& lichComp = scene->getComponent<LichComponent>(f.first);
            if(lichComp.life <= 0)
            {
                toRemove.push_back(f.first);
                continue;
            }
        }

        Transform& transComp = scene->getComponent<Transform>(f.first);
        float dist = glm::length(transComp.position - tankTransform.position);
        if(dist < tankComp.sightRadius)
        {
            tankComp.friendsInSight.insert({f.first, {f.second.type, f.second.visited}});
        }

    }
    for(auto r: toRemove)
    {
        tankComp.allFriends.erase(r);
    }
    for(int i = 0; i < toAddID.size(); i++)
    {
        tankComp.allFriends.insert({toAddID[i],{toAddData[i].type, toAddData[i].visited}});
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


    if(ret)
    {
        tankComp.inCombat = true;
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


    if(ret)
    {
        tankComp.inCombat = false;
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

    if(ret)
    {
        tankComp.inCombat = false;
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

    if(ret)
    {
        for(auto& f: tankComp.allFriends)
        {
            if(f.second.type == "Swarm")
            {
                getTheScene()->getComponent<SwarmComponent>(f.first).shieldedByTank = false;
            }
            else
            {
                getTheScene()->getComponent<LichComponent>(f.first).shieldedByTank = false;
        
            }
        }
        tankComp.inCombat = true;
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
    if(ret)
    {
        for(auto& f: tankComp.allFriends)
        {
            if(f.second.type == "Swarm")
            {
                getTheScene()->getComponent<SwarmComponent>(f.first).shieldedByTank = false;
            }
            else
            {
                getTheScene()->getComponent<LichComponent>(f.first).shieldedByTank = false;

            }
        }
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
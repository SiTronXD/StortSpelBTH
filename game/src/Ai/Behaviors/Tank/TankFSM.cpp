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
            bool found = false;
            float dist = glm::length(tankTransform.position - trans.position);
            if(dist < tankComp.sightRadius)
            {
                for(auto f: tankComp.friendsInSight)
                {
                    if(f.id == entityid)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    TankFriend buddy{entityid, true};
                    tankComp.friendsInSight.emplace_back(buddy);
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
                    if(f.id == entityid)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    TankFriend buddy{entityid, false};
                    tankComp.friendsInSight.push_back(buddy);
                }
            }
        }
    };
    viewSwarm.each(swarmLamda);
    viewLich.each(lichLamda);

    //Clean up friends
    std::vector<int> groups;
    std::vector<int> groups_swarmID;
    for(std::vector<TankFriend>::iterator f = tankComp.friendsInSight.begin(); f != tankComp.friendsInSight.end(); f++)
    {
        if(f->swarm)
        {
            bool found = false;
            int groupID = scene->getComponent<SwarmComponent>(f->id).group->myId;
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
                groups_swarmID.push_back(f->id);
            }
            tankComp.friendsInSight.erase(f);
        }
    }
    for(auto g: groups_swarmID)
    {
        TankFriend buddy{g, true};
        tankComp.friendsInSight.push_back(buddy);
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
        if(theScene->hasComponents<SwarmComponent>(f.id))
        {
            if(theScene->getComponent<SwarmComponent>(f.id).group->inCombat)
            {
                return true;
            }
        }
        else if(theScene->hasComponents<LichComponent>(f.id))
        {
             if(theScene->getComponent<LichComponent>(f.id).inCombat)
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
    //updateFriendsInSight(entityID);//Do we need to update every time or is it enough in only one transition?
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
    TankComponent& tankComp = getTankComponent();
    if(!playerInSight(entityID) && !tankComp.inCombat && tankComp.friendsInSight.size() <= 0)
    {
        ret = true;
    }
    return ret;
}

bool TankFSM::toDead(Entity entityID)
{
    falseIfDead();
    bool ret = false;
    TankComponent& tankComp = getTankComponent();
    if(tankComp.life <= 0)
    {
        ret = true;
    }
    return ret;
}


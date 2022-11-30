#include "TankFSM.hpp"
#include "../../../Components/Combat.h"

bool TankFSM::falseIfDead(Entity entityID)
{
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if(tankComp.isDead())
    {return false;}
    else if(!getTheScene()->isActive(entityID))
    {return false;}
    else
    {return true;}
}

Scene* TankFSM::getTheScene()
{
    return FSM::sceneHandler->getScene();
}

float TankFSM::get_dt()
{
    return FSM::sceneHandler->getAIHandler()->getDeltaTime();
}

int TankFSM::getPlayerID(Entity entityID)
{
    // if network exist take player from there
    NetworkScene* s = dynamic_cast<NetworkScene*>(sceneHandler->getScene());
    if (s != nullptr)
        {
            return s->getNearestPlayer(entityID);
        }

    // else find player from script
    int playerID = -1;
    std::string playerString = "playerID";
    FSM::sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
    return playerID;
}

void TankFSM::resetTimers(Entity entityID)
{
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    tankComp.groundHumpTimer = tankComp.groundHumpTimerOrig;
    

	tankComp.alertTimer       = tankComp.alertTimerOrig;
	tankComp.huntTimer        = tankComp.huntTimerOrig;   
	tankComp.chargeTimer      = tankComp.chargeTimerOrig; 
	tankComp.runTimer	      = tankComp.runTimerOrig;    
	tankComp.groundHumpTimer  = tankComp.groundHumpTimerOrig;     
	tankComp.friendHealTimer  = tankComp.friendHealTimerOrig;     

}

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
        //Check if friend is dead, then remove it from all friends
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

        //Update friends in sight
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
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    int playerID = getPlayerID(entityID);
    Transform& playerTrans  = getTheScene()->getComponent<Transform>(playerID);
    Transform& tankTrans    = getTheScene()->getComponent<Transform>(entityID);
    float tank_player_dist = glm::length(playerTrans.position - tankTrans.position);
    if(tank_player_dist <= tankComp.sightRadius)
    {
        return true;
    }
    return false;
}

bool TankFSM::friendlysInFight(Entity entityID)
{
    bool ret = false;

    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
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
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    
        
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);

    updateFriendsInSight(entityID);
    if(playerInSight(entityID) || friendlysInFight(entityID))
    {
        ret = true;
    }

    if(ret)
    {
        tankComp.alertTempYpos = getTheScene()->getComponent<Transform>(entityID).position.y;
		tankComp.alertDone = false;
		tankComp.alertAtTop = false;
        resetTimers(entityID);
    }

    return ret;
}

bool TankFSM::alertToCombat(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    updateFriendsInSight(entityID);  
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);

    if(tankComp.alertDone && tankComp.friendsInSight.size() <= 0)
	{
		tankComp.alertDone = false;
		ret = true;
	}

    if(ret)
    {
        resetTimers(entityID);
        //tankComp.canBeHit = true;
    }

    return ret;
}

bool TankFSM::alertToShield(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    updateFriendsInSight(entityID);  
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if(tankComp.alertDone && tankComp.friendsInSight.size() > 0)
	{
		tankComp.alertDone = false;
		ret = true;
	}


    if(ret)
    {
        resetTimers(entityID);
    }

    return ret;
}

bool TankFSM::combatToIdel(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if(!playerInSight(entityID) && tankComp.huntTimer <= 0 && tankComp.friendsInSight.size() <= 0)
    {
        tankComp.huntTimer = tankComp.huntTimerOrig;
        ret = true;
    }
    else
    {
        tankComp.huntTimer -= get_dt();
    }


    if(ret)
    {
        tankComp.inCombat = false;
        tankComp.humps.clear();
	    tankComp.groundHumpTimer = tankComp.groundHumpTimerOrig;
        resetTimers(entityID);
    }
    return ret;
}

bool TankFSM::combatToShield(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if((playerInSight(entityID) && tankComp.friendsInSight.size() > 0) || friendlysInFight(entityID))
    {
        ret = true;
    }

    if(ret)
    {
        tankComp.inCombat = false;
        tankComp.humps.clear();
        resetTimers(entityID);
    }

    return ret;
}

bool TankFSM::shieldToCombat(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if(playerInSight(entityID) && tankComp.friendsInSight.size() <= 0)
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
        //tankComp.canBeHit = true;

        resetTimers(entityID);
    }
    return ret;
}

bool TankFSM::shieldToIdle(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    updateFriendsInSight(entityID);
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if(!playerInSight(entityID) && !friendlysInFight(entityID))
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
        tankComp.canBeHit = true;
        resetTimers(entityID);
    }
    return ret;
}

bool TankFSM::toDead(Entity entityID)
{
    bool ret = false;
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if(tankComp.life <= 0)
    {
        ret = true;
        resetTimers(entityID);
    }
    return ret;
}

bool TankFSM::revive(Entity entityID)
{
    bool ret = false;

	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	if(tankComp.life > 0)
	{
		ret = true;
	}

	return ret;
}

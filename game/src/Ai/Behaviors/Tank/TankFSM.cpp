#include "TankFSM.hpp"
#include "../../../Components/Combat.h"
#include "../../../Network/ServerGameMode.h"

Entity TankFSM::getPlayerID(Entity entityID){
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

void TankFSM::generalUpdate(Entity entityID)
{
    updateFriendsInSight(entityID);
    updateHumps(entityID);
}

void TankFSM::deactivateHump(Entity entityID, uint32_t what)
{
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	for(auto e: tankComp.humpEnteties)
	{
		if(e == what)
		{
			getTheScene()->setInactive(e);
            ServerGameMode* netScene = dynamic_cast<ServerGameMode*>(getTheScene());
            if(netScene != nullptr){
                netScene->addEvent({(int)GameEvent::INACTIVATE, (int)e});
            }
			break;
		}
	}
}

void TankFSM::updateHump(Entity entityID, uint32_t what)
{
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);

	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
	Transform& trans = getTheScene()->getComponent<Transform>(what);
	trans.scale.x = trans.scale.z = tankComp.humps[what];
	/*trans.position.x = tankTrans.position.x;
	trans.position.z = tankTrans.position.z;*/
    ServerGameMode* netScene = dynamic_cast<ServerGameMode*>(getTheScene());
    if(netScene != nullptr){
        netScene->addEvent({(int)GameEvent::UPDATE_HUMP, (int)what}, {trans.scale.x,trans.scale.y,trans.scale.z});
    }
}

void TankFSM::updateHumps(Entity entityID)
{
    int playerID = getPlayerID(entityID);
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
	Collider& playerCol = getTheScene()->getComponent<Collider>(playerID);	
	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
	std::vector<int> toRemove;

    for(auto& h: tankComp.humps)
	{
		h.second += tankComp.humpShockwaveSpeed * get_dt();

		updateHump(entityID, h.first);
        Transform& humpTrans = getTheScene()->getComponent<Transform>(h.first);

		float dist = glm::length(playerTrans.position - humpTrans.position);
		float minHitDist = dist - playerCol.radius;
		float maxHitDist = dist + playerCol.radius;

		if(h.second/2.0f >= tankComp.humpShockwaveShieldRadius)
		{
			toRemove.push_back(h.first);
		}
		else if(h.second/2.0f >= minHitDist && h.second/2.0f <= maxHitDist && (playerTrans.position.y < 1.0f))
		{
			//PlayerHit!
			glm::vec3 to = playerTrans.position;
			glm::normalize(to);
			getTheScene()->getComponent<HealthComp>(playerID).health -= (int)tankComp.humpHit;
            std::cout<<"Player humped!\n";
			//single player
			if (dynamic_cast<NetworkSceneHandler*>(FSM::sceneHandler) == nullptr) 
			{
				Script& playerScript = getTheScene()->getComponent<Script>(playerID);
				FSM::sceneHandler->getScriptHandler()->setScriptComponentValue(playerScript , 1.0f, "pushTimer");
                Rigidbody& playerRB = getTheScene()->getComponent<Rigidbody>(playerID);

                glm::vec3 dir = glm::normalize(to - tankTrans.position);
                playerRB.velocity = dir * tankComp.humpForce;
                playerRB.velocity.y += tankComp.humpYForce;

                toRemove.push_back(h.first);
			}
            else
            {
				//send pushPlayer
                glm::vec3 dir = glm::normalize(to - tankTrans.position);
                dir *= tankComp.humpForce;
                dir.y += tankComp.humpYForce;
				//trust that push timer never changes
                ((NetworkSceneHandler*)FSM::sceneHandler)
                    ->getScene()
                    ->addEvent({(int)GameEvent::PUSH_PLAYER, playerID}, 
						{
						dir.x,
						dir.y,
						dir.z
						});
			}
		}
	}
	for(auto r: toRemove)
	{
		deactivateHump(entityID, r);
		tankComp.humps.erase(r);
	}
}

Scene* TankFSM::getTheScene()
{
    return FSM::sceneHandler->getScene();
}

float TankFSM::get_dt()
{
    return FSM::sceneHandler->getAIHandler()->getDeltaTime();
}

void TankFSM::resetTimers(Entity entityID)
{
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    tankComp.groundHumpTimer = tankComp.groundHumpTimerOrig;
    

	tankComp.alertTimer       = tankComp.alertTimerOrig;
	tankComp.huntTimer        = tankComp.huntTimerOrig;   
	tankComp.chargeTimer      = tankComp.chargeTimerOrig; 
	tankComp.runTimer	      = tankComp.runTimerOrig;    
	tankComp.groundHumpTimer  = 0.0f;//tankComp.groundHumpTimerOrig;     
	tankComp.friendHealTimer  = tankComp.friendHealTimerOrig;     

}

void TankFSM::updateFriendsInSight(Entity entityID)
{
    Scene* scene = getTheScene();

    TankComponent& tankComp = scene->getComponent<TankComponent>(entityID);
    tankComp.friendsInSight.clear();
    Transform& tankTransform = scene->getComponent<Transform>(entityID);

    std::vector<int> toRemove;
    std::vector<TankFriend> toAddData;
    for(auto f: tankComp.allFriends)
    {
        //Check if friend is dead, then remove it from all friends
        if(f.second.type == "Swarm")
        {
            SwarmComponent& swarmComp = scene->getComponent<SwarmComponent>(f.first);
            if(swarmComp.life <= 0)
            {
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
        else if(f.second.type == "")
        {
            toRemove.push_back(f.first);
            continue;
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

void TankFSM::removeHumps(Entity entityID)
{
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    tankComp.humps.clear();
    for(auto h: tankComp.humpEnteties)
    {
        getTheScene()->setInactive(h);
        ServerGameMode* netScene = dynamic_cast<ServerGameMode*>(getTheScene());
        if(netScene)
        {
            netScene->addEvent({(int)GameEvent::INACTIVATE, (int)h});
        }
    }
    std::cout<<"Removing humps!\n";
}

bool TankFSM::idleToAler(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
       
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    
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
	    tankComp.groundHumpTimer = tankComp.groundHumpTimerOrig;
        resetTimers(entityID);
        //removeHumps(entityID);
    }
    return ret;
}

bool TankFSM::combatToShield(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if((playerInSight(entityID) && tankComp.friendsInSight.size() > 0) || friendlysInFight(entityID))
    {
        ret = true;
    }

    if(ret)
    {
        tankComp.inCombat = false;
        resetTimers(entityID);
    }

    return ret;
}

bool TankFSM::shieldToCombat(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
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
            else if(f.second.type == "Lich")
            {
                getTheScene()->getComponent<LichComponent>(f.first).shieldedByTank = false;
            }
        }
        tankComp.canBeHit = true;
        resetTimers(entityID);
        //removeHumps(entityID);
    }
    return ret;
}

bool TankFSM::toDead(Entity entityID)
{
    bool ret = false;
    generalUpdate(entityID);
    TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    if(tankComp.life <= 0)
    {
        ret = true;
        resetTimers(entityID);
        removeHumps(entityID);
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

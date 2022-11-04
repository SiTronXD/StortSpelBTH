#include "SwarmFSM.hpp"

float SwarmFSM::getEntityDist(Entity one, Entity two)
{
	Transform& trans1 = FSM::sceneHandler->getScene()->getComponent<Transform>(one);
	Transform& trans2 = FSM::sceneHandler->getScene()->getComponent<Transform>(two);
	return glm::length(trans1.position - trans2.position);
}

bool SwarmFSM::idle_alerted(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());
	if(enemySwarmComp.life <= 0 || groupHealth < enemySwarmComp.LOW_HEALTH || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}

	Entity playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);


    float swarmPlayerLen = getEntityDist(entityID, playerID);
	


	if (swarmPlayerLen <= enemySwarmComp.sightRadius || 
		enemySwarmComp.group->inCombat)
    {
		ret = true;
    }


	if(ret == true)
	{
		enemySwarmComp.alertTempYpos = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID).position.y;
		enemySwarmComp.alertDone = false;
		enemySwarmComp.alertAtTop = false;
	}

	return ret;
}

bool SwarmFSM::alerted_combat(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}

	if(enemySwarmComp.alertDone)
	{
		enemySwarmComp.alertDone = false;
		enemySwarmComp.group->inCombat = true;
		ret = true;
	}
	return ret;
}

bool SwarmFSM::idle_escape(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}

	Entity playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
    
	float swarmPlayerLen = getEntityDist(entityID, playerID);
	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());

	if (swarmPlayerLen <= enemySwarmComp.sightRadius && 
		groupHealth <= enemySwarmComp.LOW_HEALTH)
    {
		ret = true;
    }

	return ret;
}

bool SwarmFSM::combat_idle(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}

	

	Entity playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);

    bool noPlayerInSight = true;
	bool canSePlayer = false;
	float swarmPlayerLen = getEntityDist(entityID, playerID);
	if (swarmPlayerLen <= enemySwarmComp.sightRadius)
	{
		canSePlayer = true;
	}


	if(!canSePlayer && !enemySwarmComp.group->inCombat)
	{
		//I cant see player and my group is not incombat
		ret = true;
	}
	else if(canSePlayer)
	{
		//I can see player
		ret = false;
	}
	else if(enemySwarmComp.group->inCombat)
	{
		//Group may see player
		for(auto& p: enemySwarmComp.group->members)
		{
			Transform& enmyTrans = FSM::sceneHandler->getScene()->getComponent<Transform>(p);
			
			SwarmComponent& enmyswarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(p);
			if (glm::length(enmyTrans.position - playerTransform.position) > enmyswarmComp.sightRadius)
			{
				ret = true;
			}
			else
			{
				noPlayerInSight = false;
				ret = false;
				break;
			}
		}
	}
	//else if(enemySwarmComp.forcedToAttack && !canSePlayer)
	//{
	//	ret = false;
	//}

	if(ret == true)
	{
		enemySwarmComp.forcedToAttack = false;
		enemySwarmComp.group->inCombat = false;	
		//Reset Combat timer
		AiCombat& combat = FSM::sceneHandler->getScene()->getComponent<AiCombat>(entityID);
		combat.timer = combat.lightAttackTime;
		//Reset scale
		FSM::sceneHandler->getScene()->getComponent<Transform>(entityID).scale.y = 1.0f;
	}

	return ret;
}

bool SwarmFSM::combat_escape(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}


	Entity playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	

	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());
    
	if(groupHealth < enemySwarmComp.LOW_HEALTH &&
		!enemySwarmComp.forcedToAttack)
	{
		ret = true;
	}

	if(ret == true)
	{
		//Reset Combat timer
		AiCombat& combat = FSM::sceneHandler->getScene()->getComponent<AiCombat>(entityID);
		combat.timer = combat.lightAttackTime;

		//Reset scale
		FSM::sceneHandler->getScene()->getComponent<Transform>(entityID).scale.y = 1.0f;
	}

	return ret;
}

bool SwarmFSM::escape_idle(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}

	Entity playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Rigidbody& enemyRb = FSM::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);

    float velAbs = abs(glm::length(enemyRb.velocity));
	float swarmPlayerDist = getEntityDist(entityID, playerID);
	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());

	if (swarmPlayerDist > enemySwarmComp.sightRadius &&
		groupHealth < enemySwarmComp.LOW_HEALTH &&
		velAbs > 0.001f)
    {
		ret = true;
    }

	if(ret == true)
	{
		FSM::sceneHandler->getScene()->getComponent<Transform>(entityID).scale.x = 1.0f;
	}

	return ret;
}

bool SwarmFSM::escape_combat(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return ret;
	}

	Entity playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Rigidbody& enemyRb = FSM::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);

    float velAbs = abs(glm::length(enemyRb.velocity));
	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());

	if(groupHealth >= enemySwarmComp.LOW_HEALTH)
	{
		ret = true;
		enemySwarmComp.group->inCombat = true;
	}
	else if(velAbs <= 0.000001f)
	{
		ret = true;
		enemySwarmComp.forcedToAttack = true;
	}

	if(ret == true)
	{
		enemyTransform.scale.x = 1.0f;
	}

	return ret;
}

bool SwarmFSM::dead(Entity entityID)
{
	bool ret = false;

	SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(swarmComp.life <= 0)
	{
		ret = true;
	}

	if(ret == true)
	{
		swarmComp.alertDone = false;
		//Reset Combat timer
		AiCombat& combat = FSM::sceneHandler->getScene()->getComponent<AiCombat>(entityID);
		combat.timer = combat.lightAttackTime;
	}

	return ret;
}

bool SwarmFSM::revive(Entity entityID)
{
	bool ret = false;

	SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(swarmComp.life > 0)
	{
		ret = true;
	}

	return ret;
}

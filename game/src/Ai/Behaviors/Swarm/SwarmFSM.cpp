#include "SwarmFSM.hpp"


bool SwarmFSM::idle_combat(Entity entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    

	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());
	if (glm::length(enemyTransform.position - playerTransform.position) <= enemySwarmComp.sightRadius && 
		groupHealth > enemySwarmComp.LOW_HEALTH)
    {
		ret = true;
    }
	else if(enemySwarmComp.group->inCombat && 
		groupHealth > enemySwarmComp.LOW_HEALTH)
	{
		ret = true;
	}
	


	if(!sceneHandler->getScene()->isActive(entityID))
	{
		ret = false;
	}

	return ret;
}

bool SwarmFSM::idle_escape(Entity entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    

	if (glm::length(enemyTransform.position - playerTransform.position) <=
        enemySwarmComp.sightRadius && 
		enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene()) <= enemySwarmComp.LOW_HEALTH &&
		enemySwarmComp.life > 0)
    {
		ret = true;
    }

	if(!sceneHandler->getScene()->isActive(entityID))
	{
		ret = false;
	}

	return ret;
}

bool SwarmFSM::combat_idle(Entity entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);

	glm::vec3 groupMidPos = glm::vec3(0.0f, 0.0f, 0.0f);
    bool noPlayerInSight = true;
	if(!enemySwarmComp.group->inCombat)
	{
		ret = true;
	}
	else
	{
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

		if(ret)
		{
			enemySwarmComp.group->inCombat = false;			
		}
	}
	if(enemySwarmComp.forcedToAttack && !noPlayerInSight)
	{
		ret = false;
	}
	if(ret == true)
	{
		enemySwarmComp.forcedToAttack = false;
	}

	return ret;
}

bool SwarmFSM::combat_escape(Entity entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if(enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene()) < enemySwarmComp.LOW_HEALTH &&
		enemySwarmComp.life > 0 &&
		!enemySwarmComp.forcedToAttack)
	{
		ret = true;
	}

	return ret;
}

bool SwarmFSM::escape_idle(Entity entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (glm::length(enemyTransform.position - playerTransform.position) >
        enemySwarmComp.sightRadius)
    {
		ret = true;
    }

	return ret;
}

bool SwarmFSM::escape_combat(Entity entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& enemyRb = FSM::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
    float velAbs = abs(glm::length(enemyRb.velocity));
	if(enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene()) >= enemySwarmComp.LOW_HEALTH)
	{
		ret = true;
		enemySwarmComp.group->inCombat = true;
	}
	else if(velAbs <= 0.001f)
	{
		ret = true;
		enemySwarmComp.forcedToAttack = true;
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

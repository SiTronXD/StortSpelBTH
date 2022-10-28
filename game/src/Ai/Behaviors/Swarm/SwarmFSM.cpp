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
    
	if (glm::length(enemyTransform.position - playerTransform.position) <= enemySwarmComp.sightRadius && 
		enemySwarmComp.life > enemySwarmComp.LOW_HEALTH)
    {
		ret = true;
    }

	return ret;
}

bool SwarmFSM::idle_combat_FirendsInFight(Entity entityID)
{
	bool ret = false;

	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (enemySwarmComp.group->inCombat)
    {
		ret = true;
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
        enemySwarmComp.sightRadius && enemySwarmComp.life <= enemySwarmComp.LOW_HEALTH)
      {
		ret = true;
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
    
	if(!enemySwarmComp.group->inCombat)
	{
		ret = true;
	}
	else
	{
		bool noPlayerInSight = true;
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
	


	return ret;
}

bool SwarmFSM::combat_escape(Entity entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (enemySwarmComp.life <= enemySwarmComp.LOW_HEALTH)
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
    
	if (enemySwarmComp.group->members.size() > 1)
    {
		ret = true;
    }

	return ret;
}

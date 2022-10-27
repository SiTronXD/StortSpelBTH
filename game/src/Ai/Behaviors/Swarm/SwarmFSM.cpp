#include "SwarmFSM.hpp"



bool SwarmFSM::idle_combat(uint32_t entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	SwarmComponent& enemySwarmCompFSM = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (glm::length(enemyTransform.position - playerTransform.position) <= enemySwarmCompBT.sightRadius && 
		enemySwarmCompFSM.life > enemySwarmCompFSM.LOW_HEALTH)
    {
		ret = true;
    }

	return ret;
}

bool SwarmFSM::idle_escape(uint32_t entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	SwarmComponent& enemySwarmCompFSM = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (glm::length(enemyTransform.position - playerTransform.position) <=
        enemySwarmCompBT.sightRadius && enemySwarmCompFSM.life <= enemySwarmCompFSM.LOW_HEALTH)
      {
		ret = true;
      }

	return ret;
}

bool SwarmFSM::combat_idle(uint32_t entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (glm::length(enemyTransform.position - playerTransform.position) >
        enemySwarmCompBT.sightRadius)
    {
		ret = true;
    }


	return ret;
}

bool SwarmFSM::combat_escape(uint32_t entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	SwarmComponent& enemySwarmCompFSM = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (enemySwarmCompFSM.life <= enemySwarmCompFSM.LOW_HEALTH)
    {
		ret = true;
    }

	return ret;
}

bool SwarmFSM::escape_idle(uint32_t entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (glm::length(enemyTransform.position - playerTransform.position) >
        enemySwarmCompBT.sightRadius)
    {
		ret = true;
    }

	return ret;
}

bool SwarmFSM::escape_combat(uint32_t entityID)
{
	bool ret = false;

	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	SwarmComponent& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    
	if (enemySwarmCompBT.group->members.size() > 1)
    {
		ret = true;
    }

	return ret;
}

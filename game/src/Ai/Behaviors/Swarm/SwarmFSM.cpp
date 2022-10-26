#include "SwarmFSM.hpp"


//void SwarmComponentFSM::registerEntity(uint32_t entityId, Scene* scene) {}

bool SwarmFSM::idle_combat(uint32_t entityID)
{
	bool ret = false;
	int playerID;
	std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponentBT& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
	SwarmComponentFSM& enemySwarmCompFSM = FSM::sceneHandler->getScene()->getComponent<SwarmComponentFSM>(entityID);
    
	if (glm::length(enemyTransform.position - playerTransform.position) <=
        enemySwarmCompBT.sightRadius && enemySwarmCompFSM.life > enemySwarmCompFSM.LOW_HEALTH)
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
	SwarmComponentBT& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
	SwarmComponentFSM& enemySwarmCompFSM = FSM::sceneHandler->getScene()->getComponent<SwarmComponentFSM>(entityID);
    
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
	SwarmComponentBT& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
    
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
	SwarmComponentFSM& enemySwarmCompFSM = FSM::sceneHandler->getScene()->getComponent<SwarmComponentFSM>(entityID);
    
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
	SwarmComponentBT& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
    
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
	SwarmComponentBT& enemySwarmCompBT = FSM::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
    
	if (enemySwarmCompBT.group->members.size() > 1)
    {
		ret = true;
    }

	return ret;
}

#pragma once

#include <glm/glm.hpp>

#include "vengine.h"
//#include "ai/PathFinding.h"

// TODO: REmove
//// DUMMY HELP THINGS
uint32_t getPlayerID_DUMMY(SceneHandler*sceneHandler ,int playerID_in = -1);

struct SwarmGroup
{
	int max;
	bool inCombat = false;
	float huntTimer;
	float huntTimerOrig = 2.0f;
	glm::vec3 idleMidBos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 lastKnownPlayerPos = glm::vec3(0.0f, 0.0f, 0.0f);
	std::vector<uint32_t> members;
};


class SwarmBT : public BehaviorTree
{
   protected:
	virtual void start() = 0;
    void registerEntityComponents(uint32_t entityId) override;

  public:
	virtual void initEntityData(uint32_t entityID) = 0;
	virtual void update(uint32_t entityID) = 0;

   protected:
	static BTStatus hasFriends(uint32_t entityID);
	static BTStatus jumpInCircle(uint32_t entityID);
	static BTStatus lookingForGroup(uint32_t entityID);
	static BTStatus JoinGroup(uint32_t entityID);

	static BTStatus seesNewFriends(uint32_t entityID);
	static BTStatus escapeToFriends(uint32_t entityID);
	static BTStatus escapeFromPlayer(uint32_t entityID);

	static BTStatus informFriends(uint32_t entityID);
	static BTStatus jumpTowardsPlayer(uint32_t entityID);
	static BTStatus closeEnoughToPlayer(uint32_t entityID);
	static BTStatus attack(uint32_t entityID);




};

class Swarm_idle : public SwarmBT
{
   protected:
	void start();
  public:
	void initEntityData(uint32_t entityID){};
	void update(uint32_t entityID){};
};

class Swarm_combat : public SwarmBT
{
   protected:
	void start();

  public:
	void initEntityData(uint32_t entityID){};
	void update(uint32_t entityID){};
};


class Swarm_escape : public SwarmBT
{
   protected:
	void start();
  public:
	void initEntityData(uint32_t entityID){};
	void update(uint32_t entityID){};
};



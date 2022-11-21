#pragma once

#include <glm/glm.hpp>
#include <stack>
#include <algorithm>
#include <vector>
#include "vengine.h"

#include "../HelperFuncs.hpp"
//#include "ai/PathFinding.h"
Entity getPlayerID(SceneHandler*sceneHandler);

struct SwarmGroup
{
    private: 
    static int getNewId;
    public: 
    SwarmGroup() {myId = getNewId++;}
    int myId;  
	int max;
	bool inCombat = false;
	float huntTimer;
	float huntTimerOrig = 2.0f;
	glm::vec3 idleMidPos = glm::vec3(0.0f, 0.0f, 0.0f);
	float idleRadius = 0.0f;
	glm::vec3 lastKnownPlayerPos = glm::vec3(0.0f, 0.0f, 0.0f);
	std::vector<uint32_t> members;
	std::stack<uint32_t> aliveMembers;

	bool isInGroup(Entity entityID){if(std::find(members.begin(), members.end(), entityID) != members.end()){return true;}return false;}
};


class SwarmBT : public BehaviorTree
{
   protected:
	static int perkMeshes[3];
	static int abilityMeshes[2];
	virtual void start() = 0;
    void registerEntityComponents(Entity entityId) override;

  public:

   protected:
	static BTStatus hasFriends(Entity entityID);
	static BTStatus jumpInCircle(Entity entityID);
	static BTStatus lookingForGroup(Entity entityID);
	static BTStatus JoinGroup(Entity entityID);	

	static BTStatus seesNewFriends(Entity entityID);
	static BTStatus escapeToFriends(Entity entityID);
	static BTStatus escapeFromPlayer(Entity entityID);

	static BTStatus informFriends(Entity entityID);
	static BTStatus jumpTowardsPlayer(Entity entityID);
	static BTStatus closeEnoughToPlayer(Entity entityID);
	static BTStatus attack(Entity entityID);

	static BTStatus playDeathAnim(Entity entityID);
	static BTStatus die(Entity entityID);

    static BTStatus alerted(Entity entityID);

};

class Swarm_idle : public SwarmBT
{
   protected:
	void start();
  public:

};

class Swarm_alerted : public SwarmBT
{
   protected:
	void start();
  public:

};

class Swarm_combat : public SwarmBT
{
   protected:
	void start();

  public:

};

class Swarm_escape : public SwarmBT
{
   protected:
	void start();
  public:

};

class Swarm_dead: public SwarmBT
{
protected:
	void start();
public:
};

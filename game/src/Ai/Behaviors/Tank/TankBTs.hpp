#pragma once

#include <glm/glm.hpp>
#include <stack>
#include "vengine.h"

#include "../HelperFuncs.hpp"
#include "vengine/ai/PathFinding.h"
#include "../../../Components/AICombatTank.hpp"

class TankBT : public BehaviorTree
{
   protected:
	// static int perkMeshes[3];
	virtual void start() = 0;
    void registerEntityComponents(Entity entityId) override;

  public:

   protected:

	//Idle
	static BTStatus HasFreindsInSight(Entity entityID);
	static BTStatus AreFriendsAlive(Entity entityID);
	static BTStatus PickNewFreinds(Entity entityID);
	static BTStatus PickNewRandomTarget(Entity entityID);
	static BTStatus MoveAround(Entity entityID);

	//Combat
	static BTStatus playerInPersonalSpace(Entity entityID);
	static BTStatus GroundHump(Entity entityID);
	static BTStatus playerOutsidePersonalSpace(Entity entityID);
	static BTStatus ChargeAndRun(Entity entityID);

	//Shield
    static BTStatus getNearestGroupToPlayer(Entity entityID);
	static BTStatus groupInPersonalSpece(Entity entityID);
	static BTStatus moveTowardsGroup(Entity entityID);
	static BTStatus HoldShield(Entity entityID);

	//Alert
	static BTStatus playAlertAnim(Entity entityID);

	//Dead
	static BTStatus playDeathAnim(Entity entityID);
	static BTStatus die(Entity entityID);
};


class Tank_idle : public TankBT
{
   protected:
	void start();
  public:
};

class Tank_alert : public TankBT
{
   protected:
	void start();
  public:
};

class Tank_combat : public TankBT
{
   protected:
	void start();
  public:
};

class Tank_shield : public TankBT
{
   protected:
	void start();
  public:
};

class Tank_dead : public TankBT
{
   protected:
	void start();
  public:
};
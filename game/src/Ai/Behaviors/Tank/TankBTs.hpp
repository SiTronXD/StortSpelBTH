#pragma once

#include <glm/glm.hpp>
#include <stack>
#include "vengine.h"

#include "../HelperFuncs.hpp"
#include "vengine/ai/PathFinding.h"
#include "../../../Components/AICombatTank.hpp"
#include "../../../Components/Combat.h"

class TankBT : public BehaviorTree
{
   protected:
	// static int perkMeshes[3];
	virtual void start() = 0;
    void registerEntityComponents(Entity entityId) override;

  public:

   protected:

	//Idle
	static BTStatus HasFreindsTarget(Entity entityID);
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


	//Helper functions
	static void rotateTowardsTarget(Entity entityID, float precision);
	static void rotateTowards(Entity entityID, glm::vec3 target, float rotSpeed, float precision);
	static bool rotationDone(Entity entityID, glm::vec3 target, float rotSpeed, float precision);

	static int		getPlayerID();
	static float	get_dt();
	static Scene*	getTheScene();
	static void		updateCanBeHit(Entity entityID);
	static void		groundHumpShortcut(Entity entityID, float maxRad);
	static void		giveFriendsHealth(Entity entityID);
	static uint32_t	activateHump(Entity entityID);
	static void		deactivateHump(Entity entityID, uint32_t what);
	static void		updateHump(Entity entityID, uint32_t what);
	static int		numActiveHumps(Entity entityID);
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
#pragma once
#include "vengine.h"
#include "TankBTs.hpp"
#include "../../../Components/AICombatTank.hpp"
#include "../Swarm/SwarmFSM.hpp"
#include "../Lich/LichFSM.hpp"

struct TankFriend
{
	std::string type;
	bool visited = false;
};

struct TankFriendTarget
{
	int id = -1;
	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
};

struct TankComponent
{
	TankComponent() 
	{
		//this->hitCol = Collider::createSphere(4.0f);
	};

	void setFriends(Scene* scene, Entity entityID)
	{
		TankComponent& tankComp = scene->getComponent<TankComponent>(entityID);
		tankComp.allFriends.clear();
		auto viewSwarm = scene->getSceneReg().view<SwarmComponent, Transform>();
        auto viewLich = scene->getSceneReg().view<LichComponent, Transform>();
        auto swarmLamda = [&](const auto& entity, SwarmComponent& comp, Transform& trans) {
            int entityid = (int)entity;
            if(scene->isActive(entityid) && entityid != entityID)
            {
				bool found = false;
				for(auto g: comp.group->members)
				{
					if(tankComp.allFriends.find(g) != tankComp.allFriends.end())
					{
						found = true;
					}
				}
				if(!found)
				{
					tankComp.allFriends.insert({entityid, {"Swarm", false}});
				}
            }        
        };
        auto lichLamda = [&](const auto& entity, LichComponent& comp, Transform& trans) {
            int entityid = (int)entity;
            if(scene->isActive(entityid) && entityid != entityID)
            {
                tankComp.allFriends.insert({entityid, {"Lich", false}});
            }
        };
        viewSwarm.each(swarmLamda);
        viewLich.each(lichLamda);
	}

	//Ints
    int FULL_HEALTH					= 500;  
    int life						= FULL_HEALTH;
	int friendHealthRegen			= 15;

	//Floats
	float idleSpeed					= 10.0f;
	float shieldSpeed				= 5.0f;
	float cahargeSpeed				= 120.0f;
    float sightRadius				= 150.0f; // I'll can attack you
    float peronalSpaceRadius		= 50.0f; // This is my personal space, get away!
	float friendVisitRadius			= 15.0f; //When go this close to friends
	float combatRotSpeed			= 50.0f;
	float idleRotSpeed				= 40.0f;
	float shildRotSpeed				= 30.0f;
	float tempRotAngle				= 0.0f;//Dont touch!
	float shieldAngle				= 270.0f; //How many degrees the shiled goes around the tank
	float runDist					= 0.0f;
	float humpForce					= 75.0f;
	float humpYForce				= 150.0f;
	float humpShockwaveSpeed		= 30.0f;
	float humpShockwaveShieldRadius	= sightRadius;
	float humpShockwaveAttackRadius	= sightRadius;
	float deathAnimSpeed			= 3.0f;
	float origScaleY				= 1.0f;
	float alertScale				= 1.5f;
	float alertAnimSpeed			= 3.0f;
	float alertTempYpos				= 0.0f;

	//Bools
    bool isDead(){return life<=0;}
	bool inCombat					= false;
	bool rotateLeft					= true;
	bool canBeHit					= true;
	bool hasRunTarget				= false;
	bool alertAtTop					= false;
	bool alertDone					= false;
	bool canAttack					= false;

	//Timers
	float alertTimerOrig			= 1.0f;
	float alertTimer				= alertTimerOrig;
	float huntTimerOrig				= 0.5f;
	float huntTimer					= huntTimerOrig;
	float chargeTimerOrig			= 2.5f;
	float chargeTimer				= huntTimerOrig;
	float runTimerOrig				= 3.0f;
	float runTimer					= huntTimerOrig;
	float groundHumpTimerOrig		= 3.0f;
	float groundHumpTimer			= huntTimerOrig;
	float friendHealTimerOrig		= 1.0f;
	float friendHealTimer			= huntTimerOrig;
	std::vector<float> humps;	//Represents the shockwaves radius fromt the hump attack

	//Vecs
	glm::vec3 shieldTargetPos		= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 runTarget				= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 runOrigin				= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 runDir				= glm::vec3(0.0f, 0.0f, 0.0f);


	TankFriendTarget firendTarget;

	std::unordered_map<int, TankFriend> friendsInSight;
	std::unordered_map<int, TankFriend> allFriends;
};


class TankFSM : public FSM
{
private:
	static bool idleToAler(Entity entityID);
	static bool alertToCombat(Entity entityID);
	static bool alertToShield(Entity entityID);
	static bool combatToIdel(Entity entityID);
	static bool combatToShield(Entity entityID);
	static bool shieldToCombat(Entity entityID);
	static bool shieldToIdle(Entity entityID);

    static bool toDead(Entity entityID);
    static bool revive(Entity entityID);


	EntityEvent idle_to_alert{		"idle to alert",	idleToAler};
	EntityEvent alert_to_combat{	"alert to combat",	alertToCombat};
    EntityEvent alert_to_shield{	"alert to shield",	alertToShield};    
    EntityEvent combat_to_idle{		"combat to idle",	combatToIdel};
    EntityEvent combat_to_shield{	"combat to shield",	combatToShield};
    EntityEvent shield_to_combat{	"shield to combat",	shieldToCombat};
    EntityEvent shield_to_idle{		"shield to idle",	shieldToIdle};


    EntityEvent idle_to_dead{		"idle to dead",		toDead};
    EntityEvent alert_to_dead{		"alert to dead",	toDead};
    EntityEvent shield_to_dead{		"hunt to dead",		toDead};
    EntityEvent combat_to_dead{		"shield to dead",	toDead};

    EntityEvent dead_to_idle{		"dead to idle",	revive};




private:
	//Helper functions
	static void		updateFriendsInSight(Entity entityID);
	static bool		playerInSight(Entity entityID);
	static bool		friendlysInFight(Entity entityID);
	static void		resetTimers(Entity entityID);
	static int		getPlayerID();
	static float	get_dt();
	static Scene*	getTheScene();
	static bool		falseIfDead(Entity entityID);
protected:
	// Inherited via FSM
	virtual void registerEntityComponents(Entity entityId) override
	{
		addRequiredComponent<TankComponent>(entityId);
		addRequiredComponent<AiCombatTank>(entityId);
	}

	virtual void real_init() override
	{

		addBTs({
			{"idle",	new Tank_idle},
			{"alerted", new Tank_alert},
			{"combat",	new Tank_combat},
			{"shield",	new Tank_shield},
			{"dead",	new Tank_dead}
        });

		addEntityTransition("idle",		TankFSM::idle_to_alert,			"alerted");
		addEntityTransition("alerted",	TankFSM::alert_to_combat,		"combat");
		addEntityTransition("alerted",	TankFSM::alert_to_shield,		"shield");
		addEntityTransition("combat",	TankFSM::combat_to_idle,		"idle");
		addEntityTransition("combat",	TankFSM::combat_to_shield,		"shield");
		addEntityTransition("shield",	TankFSM::shield_to_idle,		"idle");
		addEntityTransition("shield",	TankFSM::shield_to_combat,		"combat");
		

        addEntityTransition("idle",     TankFSM::idle_to_dead,			"dead");
        addEntityTransition("alerted",  TankFSM::alert_to_dead,			"dead");
        addEntityTransition("combat",   TankFSM::combat_to_dead,		"dead");
        addEntityTransition("shield",   TankFSM::shield_to_dead,		"dead");

		addEntityTransition("dead", TankFSM::dead_to_idle, "idle");


		setInitialNode("idle");
	}
};
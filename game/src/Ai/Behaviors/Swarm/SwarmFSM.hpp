#pragma once
#include "vengine.h"
#include "SwarmBTs.hpp"
#include "../../../Components/HealthComp.h"
#include "../../../Components/AiCombatSwarm.h"


struct SwarmComponent
{
    inline static const uint32_t colliderRadius = 4;
    inline static const uint32_t GROUP_RAD_MULTIPLER = 4;

	//Ints
	int LOW_HEALTH				= 30;
	int FULL_HEALTH				= 100;
	int life					= FULL_HEALTH;
	//Floats
	float speed					= 17.0f; // TODO: Was 0, why?... (assume testing)
	float jumpForce				= 70.0f; // TODO: Was 0, why?... (assume testing)
	float idleSpeed				= 10.0f; // TODO: Was 0, why?... (assume testing)
	float jumpY					= 10.0f; // TODO: Was 0, why?... (assume testing)
	float deathAnimSpeed		= 1.0f;
	float alertAnimSpeed		= 2.0f;
	float chargeAnimSpeed		= 1.0f;
	float escapeAnimSpeed		= 2.0f;
	float alertScale			= 1.5f;
	float alertTempYpos			= 0.0f;
    float sightRadius			= 70.0f;
	float attackRange			= 40.0f;
    float alert_top;
	float idleRotSpeed			= 100.0f;
	float tempRotAngle			= 0.0f;//Dont touch!
	//Bools
    bool alert_go_up			= true;
	bool alertAtTop				= false;
	bool inCombat				= false;
	bool forcedToAttack			= false;
	bool alertDone				= false;
    bool inAttack				= false;
	bool touchedPlayer			= false;
	bool touchedFriend			= false;
	bool grounded				= true;
	bool shieldedByTank			= false;
	bool idleIgnoreCol			= false;
	bool attackGoRight			= false;
	bool rotateLeft				= false;
	//Timers					
	float groundTimer			= 0.0f;
	float groundTimerOrig		= 1.0f;
	float lonelyTime			= 3.0f;
	float lonelyTimer			= 0.0f;
	float ignoreColTimerOrig	= 1.0f;
	float ignoreColTimer		= ignoreColTimerOrig;


	glm::vec3 friendTouched = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 idleMoveTo = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 lonelyDir = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 dir = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 right = glm::vec3(0.0f, 0.0f, 0.0f);

	SwarmGroup* group;
	std::vector<SwarmGroup*> groupsInSight;


	SwarmComponent() 
	{
		attackGoRight = rand()%2;
	};

	float getGroupHealth(Scene* scene)
	{
		float avgHealth = 0.0f;
		int num = 0;
		for(auto p: group->members)
		{
			float health = (float)scene->getComponent<SwarmComponent>(p).life;
			if(health > 0.0f)
			{
				avgHealth += health;
				num++;
			}
		}
		if(num > 0)
		{
			avgHealth /=num;
		}
		return avgHealth;
	};
	int getNumAliveInGroup(Scene* scene)
	{
		int ret = 0;
		for(auto p: group->members)
		{
			if(scene->getComponent<SwarmComponent>(p).life > 0)
			{
				ret++;
			}
			
		}
		return ret;
	}
	void setGroupMidPos(Scene* scene)
	{
		this->group->idleMidPos = glm::vec3(0.0f, 0.0f, 0.0f);
		int num = 0;
		for(auto& m: this->group->members)
		{
			if(this->life > 0)
			{
				this->group->idleMidPos += scene->getComponent<Transform>(m).position;
				num++;
			}
		}
		this->group->idleMidPos/=num;
	}
	void setGroupRadius(Scene* scene)
	{
		float max = 0.0f;
		for(auto& m: this->group->members)
		{
			Transform& trans = scene->getComponent<Transform>(m);
			float dist = glm::length(trans.position - this->group->idleMidPos);
			if(dist > max)
			{
				max = dist;
			}
		}
		this->group->idleRadius = max * SwarmComponent::GROUP_RAD_MULTIPLER;
	}
};


class SwarmFSM : public FSM
{
private:
	static float getEntityDist(Entity one, Entity two);
    static int   getPlayerID(Entity entityID);
    
private:
	static bool idle_alerted(Entity entityID);
	static bool alerted_combat(Entity entityID);
	static bool idle_escape(Entity entityID);

	static bool combat_idle(Entity entityID);
	static bool combat_escape(Entity entityID);

	static bool escape_idle(Entity entityID);
	static bool escape_combat(Entity entityID);

	static bool dead(Entity entityID);
	static bool revive(Entity entityID);


	EntityEvent idle_to_alerted{"idle to alert", idle_alerted};
	EntityEvent alerted_to_combat{"alert to combat", alerted_combat};
	EntityEvent idle_to_escape{"idle to escape", idle_escape};
	EntityEvent combat_to_idle{"combat to idle", combat_idle};
	EntityEvent combat_to_escape{"combat to escape", combat_escape};
	EntityEvent escape_to_idle{"escape to idle", escape_idle};
	EntityEvent escape_to_combat{"escape to combat", escape_combat};
	EntityEvent to_dead{"to Dead",dead};
	EntityEvent to_living{"to Living" ,revive};

public:
protected:
	// Inherited via FSM
	virtual void registerEntityComponents(Entity entityId) override
	{
		addRequiredComponent<SwarmComponent>(entityId);
		addRequiredComponent<AiCombatSwarm>(entityId);
	}

	virtual void real_init() override
	{

		addBTs({
			{"idle", new Swarm_idle},
			{"alerted", new Swarm_alerted},
		    {"combat", new Swarm_combat},
		    {"escape", new Swarm_escape},
			{"dead", new Swarm_dead}
        });

		//TODO: Cehck transitions (Only one should be possible).
		addEntityTransition("idle", SwarmFSM::idle_to_escape, "escape");
		addEntityTransition("idle", SwarmFSM::idle_to_alerted, "alerted");
		addEntityTransition("alerted", SwarmFSM::alerted_to_combat, "combat");

		addEntityTransition("combat", SwarmFSM::combat_to_escape, "escape");
		addEntityTransition("combat", SwarmFSM::combat_to_idle, "idle");

		addEntityTransition("escape", SwarmFSM::escape_to_combat, "combat");
		addEntityTransition("escape", SwarmFSM::escape_to_idle, "idle");

		addEntityTransition("idle", SwarmFSM::to_dead, "dead");
		addEntityTransition("alerted", SwarmFSM::to_dead, "dead");
		addEntityTransition("combat", SwarmFSM::to_dead, "dead");
		addEntityTransition("escape", SwarmFSM::to_dead, "dead");

		addEntityTransition("dead", SwarmFSM::to_living, "idle");

	


		setInitialNode("idle");



	}
	//Helper functions
	static void updateSwarmGrounded(Entity entityID);
};
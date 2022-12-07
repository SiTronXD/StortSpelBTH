#pragma once
#include "vengine.h"
#include "SwarmBTs.hpp"
#include "../../../Components/HealthComp.h"
#include "../../../Components/AiElite.hpp"

struct SwarmComponent
{
	inline static bool s_initialized = false;
	inline static int s_takeDmg = -1;
	inline static int s_move = -1;
	inline static int s_attack = -1;

    inline static const uint32_t colliderRadius = 4;
    inline static const uint32_t GROUP_RAD_MULTIPLER = 4;

	//Ints
	int LOW_HEALTH				= 30;
	int FULL_HEALTH				= 100;
	int life					= FULL_HEALTH;
	//Floats
	float speed					= 17.0f;
	float jumpForce				= 70.0f;
	float idleSpeed				= 10.0f;
	float jumpY					= 10.0f;
	float deathAnimSpeed		= 1.0f;
	float alertAnimSpeed		= 2.0f;
	float chargeAnimSpeed		= 1.0f;
	float escapeAnimSpeed		= 2.0f;
	float alertScale			= 1.5f;
	float origScaleY			= 1.0f;
	float alertTempYpos			= 0.0f;
    float sightRadius			= 70.0f;
	float attackRange			= 40.0f;
    float alert_top;
	float idleRotSpeed			= 100.0f;
	float tempRotAngle			= 0.0f;//Dont touch!
	float lightHit = 15.f;
	float lightAttackTime = 2.0f;
	float timer = 0.f;
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
	bool isElite				= false;
	//Timers					
	float groundTimer			= 0.0f;
	float groundTimerOrig		= 1.0f;
	float lonelyTime			= 3.0f;
	float lonelyTimer			= 0.0f;
	float ignoreColTimerOrig	= 1.0f;
	float ignoreColTimer		= ignoreColTimerOrig;

	glm::vec3 origScale			= glm::vec3(0.0f, 0.0f, 0.0f); 
	glm::vec3 friendTouched		= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 idleMoveTo		= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 lonelyDir			= glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 dir				= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 forward			= glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 right				= glm::vec3(0.0f, 0.0f, 0.0f);

	SwarmGroup* group;
	std::vector<SwarmGroup*> groupsInSight;
	AiEliteComponent eliteStats;
	SwarmComponent() 
	{
		attackGoRight = rand()%2;
	};

	void applyEliteStats(AiEliteComponent& eliteComp, Scene* scene, Entity entityID)
    {
		this->eliteStats = eliteComp;
		this->isElite					= true;

        this->lightHit		            *= eliteComp.dmgMultiplier;
        this->LOW_HEALTH                *= eliteComp.healthMultiplier;           
        this->FULL_HEALTH               *= eliteComp.healthMultiplier;

        this->sightRadius               *= eliteComp.radiusMultiplier;
		this->attackRange				*= eliteComp.radiusMultiplier;

		scene->getComponent<Collider>(entityID).radius *= eliteComp.sizeMultiplier;
		Transform& trans = scene->getComponent<Transform>(entityID);
		trans.scale = this->origScale * eliteComp.sizeMultiplier;
		this->origScale = trans.scale;
        
		this->speed						*= eliteComp.speedMultiplier;
		this->idleRotSpeed				*= eliteComp.speedMultiplier;
        this->speed                     *= eliteComp.speedMultiplier;
    }
    void removeEliteStats(Scene* scene, Entity entityID)
    {
      	this->isElite					= false;

        this->lightHit		            /= this->eliteStats.dmgMultiplier;
        this->LOW_HEALTH                /= this->eliteStats.healthMultiplier;           
        this->FULL_HEALTH               /= this->eliteStats.healthMultiplier;
        this->sightRadius               /= this->eliteStats.radiusMultiplier;
		this->attackRange				/= this->eliteStats.radiusMultiplier;
										
		scene->getComponent<Collider>(entityID).radius /= this->eliteStats.sizeMultiplier;
		scene->getComponent<Transform>(entityID).scale /= this->eliteStats.sizeMultiplier;
		Transform& trans = scene->getComponent<Transform>(entityID);
		trans.scale = this->origScale / this->eliteStats.sizeMultiplier;
		this->origScale = trans.scale;
        								
		this->speed						/= this->eliteStats.speedMultiplier;
		this->idleRotSpeed				/= this->eliteStats.speedMultiplier;
        this->speed                     /= this->eliteStats.speedMultiplier;
    }

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
	}

	virtual void real_init() override
	{

		addBTs({
			{"idle", new(__FILE__, __LINE__) Swarm_idle},
			{"alerted", new(__FILE__, __LINE__) Swarm_alerted},
		    {"combat", new(__FILE__, __LINE__) Swarm_combat},
		    {"escape", new(__FILE__, __LINE__) Swarm_escape},
			{"dead", new(__FILE__, __LINE__) Swarm_dead}
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
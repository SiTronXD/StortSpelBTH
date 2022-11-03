#pragma once
#include "vengine.h"
#include "SwarmBTs.hpp"
#include "../../../Components/AiCombat.h"


struct SwarmComponent
{
	int LOW_HEALTH = 30;
	int FULL_HEALTH = 100;
	int life = FULL_HEALTH;
	float speed = 17.0f;
	float jumpForce = 70.0f;
	float jumpY = 10.0f;

	float deathAnimSpeed = 1.0f;
	float alertAnimSpeed = 2.0f;
	float alertScale	 = 1.5f;
	float alertTempYpos	= 0.0f;
	bool alertAtTop = false;

    float sightRadius	= 70;
	float attackRange	= 40;
	bool inCombat		= false;
	bool forcedToAttack = false;
    bool inAttack		= false;
	bool alertDone		= false;
	SwarmGroup* group;
	std::vector<SwarmGroup*> groupsInSight;

    bool alert_go_up = true;
    float alert_top;

	SwarmComponent() {};

	float getGroupHealth(Scene* scene)
	{
		float ret = 0.0f;
		for(auto p: group->members)
		{
			ret += scene->getComponent<SwarmComponent>(p).life;
		}
		if(group->members.size() > 0)
		{
			ret /= group->members.size();
		}
		return ret;
	};
};


class SwarmFSM : public FSM
{
private:
	static float getEntityDist(int one, int two);
private:
	static bool idle_alerted(Entity entityID);
	static bool alerted_combat(Entity entityID);
	static bool idle_escape(Entity entityID);

	static bool combat_idle(Entity entityID);
	static bool combat_escape(Entity entityID);

	static bool escape_idle(Entity entityID);
	static bool escape_combat(Entity entityID);

    // static bool notisPlayer(Entity entityID);
	// static bool playerDissapeared(Entity entityID);

	static bool dead(Entity entityID);
	static bool revive(Entity entityID);


	EntityEvent idle_to_alerted{"idle to alert", idle_alerted};
	EntityEvent alerted_to_combat{"alert to combat", alerted_combat};
	EntityEvent idle_to_escape{"idle to escape", idle_escape};
	EntityEvent combat_to_idle{"combat to idle", combat_idle};
	EntityEvent combat_to_escape{"combat to escape", combat_escape};
	EntityEvent escape_to_idle{"escape to idle", escape_idle};
	EntityEvent escape_to_combat{"escape to combat", escape_combat};

    // EntityEvent notisPlayer_event{notisPlayer};
	// EntityEvent playerDissapeared_event{playerDissapeared};

	EntityEvent to_dead{"to Dead",dead};
	EntityEvent to_living{"to Living" ,revive};

public:
protected:
	// Inherited via FSM
	virtual void registerEntityComponents(Entity entityId) override
	{
		addRequiredComponent<SwarmComponent>(entityId);
		addRequiredComponent<AiCombat>(entityId);
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
};
#pragma once
#include "vengine.h"
#include "SwarmBTs.hpp"
#include "../../../Components/AiCombat.h"

struct SwarmComponent
{
	int LOW_HEALTH = 30;
	int FULL_HEALTH = 100;
	int life = FULL_HEALTH ;
	float speed = 10;

    float sightRadius	= 10;
	float attackRange	= sightRadius/2;
	bool inCombat		= false;
	SwarmGroup* group;
	std::vector<SwarmGroup*> groupsInSight;	

	SwarmComponent(){};
};


class SwarmFSM : public FSM
{
private:
	static bool idle_combat(Entity entityID);
	static bool idle_combat_FirendsInFight(Entity entityID);
	static bool idle_escape(Entity entityID);
	
	static bool combat_idle(Entity entityID);
	static bool combat_escape(Entity entityID);

	static bool escape_idle(Entity entityID);
	static bool escape_combat(Entity entityID);


	EntityEvent idle_to_combat{idle_combat};
	EntityEvent idle_to_combat_firends_in_fight{idle_combat_FirendsInFight};
	EntityEvent idle_to_escape{idle_escape};
	EntityEvent combat_to_idle{combat_idle};
	EntityEvent combat_to_escape{combat_escape};
	EntityEvent escape_to_idle{escape_idle};
	EntityEvent escape_to_combat{escape_combat};

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
		    {"combat", new Swarm_combat},
		    {"escape", new Swarm_escape}
        });

		//TODO: Cehck transitions (Only one should be possible).
		addEntityTransition("idle", SwarmFSM::idle_to_escape , "escape");
		addEntityTransition("idle", SwarmFSM::idle_to_combat , "combat");
		addEntityTransition("idle", SwarmFSM::idle_to_combat_firends_in_fight , "combat");

		addEntityTransition("combat", SwarmFSM::combat_to_escape, "escape");
		addEntityTransition("combat", SwarmFSM::combat_to_idle, "idle");

		addEntityTransition("escape", SwarmFSM::escape_to_combat , "combat");
		addEntityTransition("escape", SwarmFSM::escape_to_idle, "idle");
	

		setInitialNode("idle");
	}
};
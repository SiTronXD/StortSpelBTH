#pragma once
#include "vengine.h"
#include "SwarmBTs.hpp"


struct SwarmComponentFSM : public FSM_component
{
	 int LOW_HEALTH = 30;
	 int FULL_HEALTH = 100;
	int life = FULL_HEALTH ;
	int attack  = 1;
	float speed = 10;

	// Inherited via FSM_component
	virtual void registerEntity(uint32_t entityId, SceneHandler* sceneHandler) override {
		sceneHandler->getScene()->setComponent(entityId, SwarmComponentFSM());
	}
};


class SwarmFSM : public FSM
{
private:
	static bool idle_combat(uint32_t entityID);
	static bool idle_escape(uint32_t entityID);
	
	static bool combat_idle(uint32_t entityID);
	static bool combat_escape(uint32_t entityID);

	static bool escape_idle(uint32_t entityID);
	static bool escape_combat(uint32_t entityID);


	EntityEvent idle_to_combat{idle_combat};
	EntityEvent idle_to_escape{idle_escape};
	EntityEvent combat_to_idle{combat_idle};
	EntityEvent combat_to_escape{combat_escape};
	EntityEvent escape_to_idle{escape_idle};
	EntityEvent escape_to_combat{escape_combat};

public:
protected:
	// Inherited via FSM
	virtual void real_init() override
	{
		addRequiredComponent(new SwarmComponentFSM);		

		addBTs({
			{"idle", new Swarm_idle},
		    {"combat", new Swarm_combat},
		    {"escape", new Swarm_escape}
        });

		//// Movement temporary shit
		//addEntityTransition("walking", energyMed, "jogging");
		addEntityTransition("idle", SwarmFSM::idle_to_combat , "combat");
		addEntityTransition("idle", SwarmFSM::idle_to_escape , "escape");

		addEntityTransition("combat", SwarmFSM::combat_to_idle, "idle");
		addEntityTransition("combat", SwarmFSM::combat_to_escape, "escape");

		addEntityTransition("escape", SwarmFSM::escape_to_combat , "combat");
		addEntityTransition("escape", SwarmFSM::escape_to_idle, "idle");
	

		setInitialNode("idle");
	}
};
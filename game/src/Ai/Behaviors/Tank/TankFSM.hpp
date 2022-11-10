#pragma once
#include "vengine.h"
#include "TankBTs.hpp"
#include "../../../Components/AICombatTank.hpp"
#include "../Swarm/SwarmFSM.hpp"

struct TankComponent
{
	TankComponent() {};

    int FULL_HEALTH = 300;  
    int life = FULL_HEALTH;
    bool isDead(){return life<=0;}

    float sightRadius           = 100; // I'll can attack you
    float peronalSpaceRadius    = 90 ; // This is my personal space, get away!

	Entity firendTarget;


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


public:
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


		setInitialNode("idle");
	}
};
#pragma once
#include "vengine.h"
#include "LichBTs.hpp"

enum ATTACK_STRATEGY
{
    NONE    = 0,  
    LIGHT   = 1,
    FIRE    = 2,
    ICE     = 3,
    _LAST
};

struct LichAttack
{
    ATTACK_STRATEGY type;
    int damage;
    float cooldownTimer;
    float cooldownTimerOrig;
    float castTimeTimer;
    float castTimeTimerOrig;
    
    void setStats(ATTACK_STRATEGY type)
    {
        this->type = type;
        switch (type)
        {
        case LIGHT:
            this->damage = 10;
            this->cooldownTimer = this->cooldownTimerOrig = 0.5f;
            this->castTimeTimer = this->castTimeTimerOrig = 0.0f;
            break;
        case FIRE:
            this->damage = 65;
            this->cooldownTimer = this->cooldownTimerOrig = 7.0f;
            this->castTimeTimer = this->castTimeTimerOrig = 2.0f;
            break;
        case ICE:
            this->damage = 25;
            this->cooldownTimer = this->cooldownTimerOrig = 3.0f;
            this->castTimeTimer = this->castTimeTimerOrig = 1.0f;
            break;
        }
    };
};

struct LichComponent
{
	LichComponent() 
    {
        lightning.setStats(ATTACK_STRATEGY::LIGHT);  
        fire.setStats(ATTACK_STRATEGY::FIRE);
        ice.setStats(ATTACK_STRATEGY::ICE);
    };

    //Ints
    int LOW_HEALTH              = 30;            
    int FULL_HEALTH             = 300;  
    int ESCAPE_HEALTH           = FULL_HEALTH / 4; 
    int life                    = FULL_HEALTH;    
    int numBones                = 0;

    //Floats
    float tempRotAngle			= 0.0f;//Dont touch!
    float creepRotSpeed         = 60.0f;
        //Alert
    float origScaleY			= 1.0f;
	float alertScale			= 1.5f;
	float alertAnimSpeed		= 3.0f;
	float alertTempYpos			= 0.0f;
        //Radius
    float sightRadius           = 150; // I'll just look at you
    float peronalSpaceRadius    = 90 ; // To close! I will initiate hunt!
    float attackRadius          = 70 ; // I'm actually able to shoot at you!
    float nonoRadius            = 40 ; // Too close, I will back away from you! (while shooting) 
        //Stats
    float mana                  = 100;
    float speed                 = 20 ; // Too close, I will back away from you! (while shooting) 

    ATTACK_STRATEGY strat       = ATTACK_STRATEGY::NONE;

    //Bools
    bool rotateLeft				= true;
    bool inCombat               = false;
    bool shieldedByTank         = false;
    bool alertAtTop				= false;
	bool alertDone				= false;

    bool isDead(){return life<=0;}

    //Combat stuff
    LichAttack lightning;
    LichAttack fire;
    LichAttack ice;
};


class LichFSM : public FSM
{
private:
	static bool idleToCreep(Entity entityID);
	static bool creepToAlerted(Entity entityID);
    static bool alertToHunt(Entity entityID);    
    static bool huntToIdle(Entity entityID);
    static bool huntToCombat(Entity entityID);
    static bool escapeToCombat(Entity entityID);
    static bool escapeToIdle(Entity entityID);
    static bool combatToIdle(Entity entityID);
    static bool combatToHunt(Entity entityID);

    static bool toDead(Entity entityID);
    static bool revive(Entity entityID);

	EntityEvent idle_to_creep{   "idle to creep",      idleToCreep};
	EntityEvent creep_to_alerted{"creep to alerted",   creepToAlerted};
    EntityEvent alert_to_hunt{   "alert to hunt",      alertToHunt};    
    EntityEvent hunt_to_idle{    "hunt to idle",       huntToIdle};
    EntityEvent hunt_to_combat{  "hunt to combat",     huntToCombat};
    EntityEvent combat_to_idle{  "combat to dead",     combatToIdle};
    EntityEvent combat_to_hunt{  "combat to hunt",     combatToHunt};
    EntityEvent escape_to_combat{"escape to combat",   escapeToCombat};
    EntityEvent escape_to_idle{  "escape to idle",     escapeToIdle};


    EntityEvent idle_to_dead{    "idle to dead",    toDead};
    EntityEvent alert_to_dead{   "alert to dead",   toDead};
    EntityEvent hunt_to_dead{    "hunt to dead",    toDead};
    EntityEvent combat_to_dead{  "combat to dead",  toDead};
    EntityEvent escape_to_dead{  "escape to dead",  toDead};
    EntityEvent creep_to_dead{   "creep to dead",   toDead};

    EntityEvent dead_to_idle{   "dead to idle",   revive};


public:
protected:
	// Inherited via FSM
	virtual void registerEntityComponents(Entity entityId) override
	{
		addRequiredComponent<LichComponent>(entityId);
	}

	virtual void real_init() override
	{

		addBTs({
			{"idle", new Lich_idle},
			{"creep", new Lich_creep},
			{"alerted", new Lich_alerted},
			{"hunt", new Lich_hunt},
			{"combat", new Lich_combat},
			{"escape", new Lich_escape},
			{"dead", new Lich_dead}
        });

		addEntityTransition("idle",     LichFSM::idle_to_creep,        "creep");
		addEntityTransition("creep",    LichFSM::creep_to_alerted,     "alerted");
		addEntityTransition("alerted",  LichFSM::alert_to_hunt,        "hunt");
		addEntityTransition("hunt",     LichFSM::hunt_to_idle,         "idle");
		addEntityTransition("hunt",     LichFSM::hunt_to_combat,       "combat");
		addEntityTransition("combat",   LichFSM::combat_to_idle,       "idle");
		addEntityTransition("combat",   LichFSM::combat_to_hunt,       "hunt");
		addEntityTransition("escape",   LichFSM::escape_to_combat,     "combat");
		addEntityTransition("escape",   LichFSM::escape_to_idle,       "idle");

        addEntityTransition("idle",     LichFSM::idle_to_dead,         "dead");
        addEntityTransition("alerted",  LichFSM::alert_to_dead,        "dead");
        addEntityTransition("hunt",     LichFSM::hunt_to_dead,         "dead");
        addEntityTransition("combat",   LichFSM::combat_to_dead,       "dead");
        addEntityTransition("escape",   LichFSM::escape_to_dead,       "dead");
        addEntityTransition("creep",    LichFSM::creep_to_dead,        "dead");

        addEntityTransition("dead",    LichFSM::dead_to_idle,        "idle");


		setInitialNode("idle");
	}


    //Helper functions
    static int		getPlayerID();
	static float	get_dt();
	static Scene*	getTheScene();
	static bool		falseIfDead(Entity entityID);
};
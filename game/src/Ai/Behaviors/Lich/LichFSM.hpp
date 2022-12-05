#pragma once
#include "vengine.h"
#include "LichBTs.hpp"
#include <string>
#include "../../../Components/AiElite.hpp"


struct LichAttack
{
    ATTACK_STRATEGY type;
    float manaCost;
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
        case ATTACK_STRATEGY::LIGHT:
            this->damage = 10;
            this->manaCost = 5.0f;
            this->cooldownTimer = this->cooldownTimerOrig = 2.0f;
            this->castTimeTimer = this->castTimeTimerOrig = 0.5f;
            break;
        case ATTACK_STRATEGY::FIRE:
            this->damage = 65;
            this->manaCost = 30.0f;
            this->cooldownTimer = this->cooldownTimerOrig = 14.0f;
            this->castTimeTimer = this->castTimeTimerOrig = 4.0f;
            break;
        case ATTACK_STRATEGY::ICE:
            this->damage = 25;
            this->manaCost = 10.0f;
            this->cooldownTimer = this->cooldownTimerOrig = 7.0f;
            this->castTimeTimer = this->castTimeTimerOrig = 2.0f;
            break;
        }
    }


};

struct Orb {
    
    inline static const int LIFE_TIME = 10; 
    int timeAtCast = 0;
    LichAttack* orbPower; 

    inline void setInactive(Entity entityID, SceneHandler* sceneHandler)
    {        
        Rigidbody& rb = sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
        rb.velocity = glm::vec3(0.f,0.f,0.f);
        sceneHandler->getScene()->setInactive(static_cast<int>(entityID));
    }
    inline void onCollision(Entity entityID, SceneHandler* sceneHandler)
    {
        //TODO: Some effect? 
        this->setInactive(entityID, sceneHandler);

    }
};

class OrbSystem : public System 
{
private:
    SceneHandler* sceneHandler; 
public: 
    OrbSystem(SceneHandler* sceneHandler) : sceneHandler(sceneHandler)
    {}
    
    bool update(entt::registry& reg, float deltaTime)
    {
        reg.view<Orb>(entt::exclude<Inactive>).each(
            [&](const auto& entity, Orb& orb){
                if(orb.timeAtCast + Orb::LIFE_TIME < Time::getTimeSinceStart() )
                {
                    orb.setInactive(static_cast<int>(entity),sceneHandler);
                }
            }
        );
        return false; 
    }
};

struct LichComponent
{
    inline static const uint32_t colliderRadius = 4;
    inline static const uint32_t colliderHeight = 12;

    inline static const uint32_t graveHeight = 8;
    inline static const uint32_t graveWidth = 6;
    inline static const uint32_t graveDepth = 4;

    inline static const uint32_t alterHeight = 8;
    inline static const uint32_t alterWidth = 3;
    inline static const uint32_t alterDepth = 3;

    inline static const uint32_t orbRadius  = 3;
    inline static const uint32_t orbSpawnDistFrom = 5;

    inline static const uint32_t NR_ICE_ORBS   = 20;
    inline static const uint32_t NR_FIRE_ORBS  = 20;
    inline static const uint32_t NR_LIGHT_ORBS = 20;

    inline static const float spellForce = 175;

	LichComponent() 
    {
        LichAttack lightning, fire, ice;
        lightning.setStats(ATTACK_STRATEGY::LIGHT);  
        fire.setStats(ATTACK_STRATEGY::FIRE);
        ice.setStats(ATTACK_STRATEGY::ICE);
        attacks.insert({"lightning", lightning});
        attacks.insert({"ice", ice});
        attacks.insert({"fire", fire});
    };

    //Ints
    int LOW_HEALTH              = 30;            
    int FULL_HEALTH             = 300;  
    int ESCAPE_HEALTH           = FULL_HEALTH / 4; 
    int BACK_TO_FIGHT_HEALTH    = FULL_HEALTH / 2; 
    int numBones                = 0;
    int life                    = FULL_HEALTH;    


    //Floats
    float life_float            = 0.0f;//Dont touch!
    float tempRotAngle			= 0.0f;//Dont touch!
    float creepRotSpeed         = 60.0f;
    float huntRotSpeed          = 200.0f;
    float idleTurnSpeed         = 300.0f;
    float plunderDuration       = 4.f; 
    float DropOffDuration       = 4.f; 
    float timeSincePlunderBegin = 0.f; 
    float timeSinceAlterWaitBegin = 0.f; 
        //Alert
    float origScaleY			= 1.0f;
	float alertScale			= 1.5f;
	float alertAnimSpeed		= 3.0f;
	float alertTempYpos			= 0.0f;
        //Radius
    float sightRadius           = 105.0f; // I'll just look at you
    float peronalSpaceRadius    = 100.0f; // To close! I will initiate hunt!
    float attackRadius          = 80.0f; // I'm actually able to shoot at you!
    float nonoRadius            = 40.0f; // Too close, I will back away from you! (while shooting) 

    float closeToGrave          = 20.f + LichComponent::graveWidth;
    float closeToAlter          = 20.f + LichComponent::alterWidth;
        //Stats
    float maxMana               = 100.0f;
    float mana                  = maxMana;
    float manaRegenSpeed        = 5.0f;
    float healthRegenSpeed      = 2.0f;
    float deathAnimSpeed        = 3.0f;
    float huntSpeed             = 60.0f;
    float speed                 = 20.0f ; // Too close, I will back away from you! (while shooting) 

    //ATTACK_STRATEGY strat       = ATTACK_STRATEGY::NONE;

    //Bools
    bool rotateLeft				= true;
    bool inCombat               = false;
    bool shieldedByTank         = false;
    bool alertAtTop				= false;
	bool alertDone				= false;
    bool regeningMana           = false;
    bool chargingAttack         = true;
    bool tempAttack             = false;//For testing strategy picker
    bool attackGoRight          = false;

    bool carryingBones = false;

    // Orbs
    std::array<Entity, LichComponent::NR_FIRE_ORBS>  fireOrbs;
    std::array<Entity, LichComponent::NR_ICE_ORBS>   iceOrbs;
    std::array<Entity, LichComponent::NR_LIGHT_ORBS> lightOrbs;    

    // Movement Locations
    Entity alterID;
    Entity graveID;

    void applyEliteStats(AiEliteComponent& eliteComp)
    {
        for(auto a: this->attacks)
        {
            a.second.damage             *= eliteComp.dmgMultiplier;
        }
        this->LOW_HEALTH                *= eliteComp.healthMultiplier;           
        this->FULL_HEALTH               *= eliteComp.healthMultiplier;
        this->ESCAPE_HEALTH             *= eliteComp.healthMultiplier;
        this->BACK_TO_FIGHT_HEALTH      *= eliteComp.healthMultiplier;

        this->sightRadius               *= eliteComp.radiusMultiplier;
        this->peronalSpaceRadius        *= eliteComp.radiusMultiplier;
        this->attackRadius              *= eliteComp.radiusMultiplier;
        this->nonoRadius                *= eliteComp.radiusMultiplier;

        this->origScaleY                *= eliteComp.sizeMultiplier;
        
        this->healthRegenSpeed          *= eliteComp.speedMultiplier;
        this->manaRegenSpeed            *= eliteComp.speedMultiplier;
        this->creepRotSpeed             *= eliteComp.speedMultiplier;
        this->huntRotSpeed              *= eliteComp.speedMultiplier;
        this->huntSpeed                 *= eliteComp.speedMultiplier;
        this->speed                     *= eliteComp.speedMultiplier;
    }
    void removeEliteStats(AiEliteComponent& eliteComp)
    {
        for(auto a: this->attacks)
        {
            a.second.damage             /= eliteComp.dmgMultiplier;
        }
        this->LOW_HEALTH                /= eliteComp.radiusMultiplier;           
        this->FULL_HEALTH               /= eliteComp.radiusMultiplier;
        this->ESCAPE_HEALTH             /= eliteComp.radiusMultiplier;
        this->BACK_TO_FIGHT_HEALTH      /= eliteComp.radiusMultiplier;

        this->sightRadius               /= eliteComp.radiusMultiplier;
        this->peronalSpaceRadius        /= eliteComp.radiusMultiplier;
        this->attackRadius              /= eliteComp.radiusMultiplier;
        this->nonoRadius                /= eliteComp.radiusMultiplier;

        this->origScaleY                /= eliteComp.sizeMultiplier;

        this->healthRegenSpeed          /= eliteComp.speedMultiplier;
        this->manaRegenSpeed            /= eliteComp.speedMultiplier;
        this->creepRotSpeed             /= eliteComp.speedMultiplier;
        this->huntRotSpeed              /= eliteComp.speedMultiplier;
        this->huntSpeed                 /= eliteComp.speedMultiplier;
        this->speed                     /= eliteComp.speedMultiplier;
    }

    bool isDead(){return life<=0;}

    //Combat stuff
    LichAttack* curAttack       = nullptr;
    std::unordered_map<std::string, LichAttack> attacks;
   /* LichAttack lightning;
    LichAttack fire;
    LichAttack ice;*/

    std::string lastAttack      = "";
};


class LichFSM : public FSM
{
private:
    static Entity getPlayerID(Entity entityID);
private:
	static bool idleToCreep(Entity entityID);
	static bool creepToAlerted(Entity entityID);
    static bool creepToIdle(Entity entityID);
    static bool alertToHunt(Entity entityID);
    static bool huntToIdle(Entity entityID);
    static bool huntToCombat(Entity entityID);
    static bool escapeToCombat(Entity entityID);
    static bool escapeToIdle(Entity entityID);
    static bool combatToEscape(Entity entityID);
    static bool combatToIdle(Entity entityID);
    static bool combatToHunt(Entity entityID);

    static bool toDead(Entity entityID);
    static bool revive(Entity entityID);

	EntityEvent idle_to_creep{   "idle to creep",      idleToCreep};
	EntityEvent creep_to_alerted{"creep to alerted",   creepToAlerted};
	EntityEvent creep_to_idle{   "creep to idle",      creepToIdle};
    EntityEvent alert_to_hunt{   "alert to hunt",      alertToHunt};  
    EntityEvent hunt_to_idle{    "hunt to idle",       huntToIdle};
    EntityEvent hunt_to_combat{  "hunt to combat",     huntToCombat};
    EntityEvent combat_to_idle{  "combat to dead",     combatToIdle};
    EntityEvent combat_to_hunt{  "combat to hunt",     combatToHunt};
    EntityEvent combat_to_escape{"combat to escape",   combatToEscape};
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
		addEntityTransition("creep",    LichFSM::creep_to_idle,        "idle");
		addEntityTransition("alerted",  LichFSM::alert_to_hunt,        "hunt");
		addEntityTransition("hunt",     LichFSM::hunt_to_idle,         "idle");
		addEntityTransition("hunt",     LichFSM::hunt_to_combat,       "combat");
		addEntityTransition("combat",   LichFSM::combat_to_idle,       "idle");
		addEntityTransition("combat",   LichFSM::combat_to_hunt,       "hunt");
		addEntityTransition("combat",   LichFSM::combat_to_escape,     "escape");
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
	static float	get_dt();
	static Scene*	getTheScene();
	static bool		falseIfDead(Entity entityID);
    static void updateAttackColldowns(Entity entityID);
};
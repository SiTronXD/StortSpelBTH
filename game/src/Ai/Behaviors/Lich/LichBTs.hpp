#pragma once

#include <glm/glm.hpp>
#include <stack>
#include "vengine.h"

#include "../../../Components/Combat.h"
#include "../HelperFuncs.hpp"

enum class ATTACK_STRATEGY
{
    NONE    = 0,  
    LIGHT   = 1,
    FIRE    = 2,
    ICE     = 3,
    _LAST

};

enum class LichAnim
{
    Walking,
    Attacking
};

struct ExtraBlendAnimationArgs
{
    bool use = false;
    std::string slotName; 
    float transitionTime;
    float nextAniTimeScale;
};

class LichBT : public BehaviorTree
{
    friend struct Orb;
   protected:
	// static int perkMeshes[3];
	virtual void start() = 0;
    void registerEntityComponents(Entity entityId) override;

  public:
	  virtual ~LichBT() {};

   protected:

    static BTStatus plunder(Entity entityID);
    static BTStatus goToGrave(Entity entityID);
    static BTStatus goToAlter(Entity entityID);
    static BTStatus dropOffBones(Entity entityID);
    static BTStatus carryingBones(Entity entityID);
    
    static BTStatus closeToGrave(Entity entityID);
    static BTStatus closeToAlter(Entity entityID);

	static BTStatus creepyLook(Entity entityID);

	static BTStatus huntingPlayer(Entity entityID);    

	static BTStatus playerInNoNoZone(Entity entityID);    
	static BTStatus moveAwayFromPlayer(Entity entityID);    
	static BTStatus notEnoughMana(Entity entityID);    
    static BTStatus hasStrategy(Entity entityID);
	static BTStatus regenerateMana(Entity entityID);    
	static BTStatus pickBestStrategy(Entity entityID);    
	static BTStatus pickRandomStrategy(Entity entityID);    
    static BTStatus attack(Entity entityID);

	static BTStatus selfHeal(Entity entityID);    
	static BTStatus playerNotVisible(Entity entityID);    
	static BTStatus runAwayFromPlayer(Entity entityID);      

	static BTStatus playDeathAnim(Entity entityID);
	static BTStatus die(Entity entityID);
    static BTStatus alerted(Entity entityID);


	//Helper funcs
    static ExtraBlendAnimationArgs junkArg;
    static void     setAnimation(Entity entityID, LichAnim anim, BTStatus(*callee)(Entity) );
    static void     activateAnim(Entity entityID, LichAnim anim, const std::string& blendTo, ExtraBlendAnimationArgs& extra = LichBT::junkArg);

	static int		getPlayerID(Entity entityID);
	static float	get_dt();
	static Scene*	getTheScene();
	static void		rotateTowards(Entity entityID, glm::vec3 target, float rotSpeed, float precision = 5.0f);
	static void		drawRaySimple(Ray& ray, float dist, glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f));
	static bool		rayChecking(Entity entityID, glm::vec3& moveDir);

	static void		givePointsForPlayerHealth	(Entity entityID, float& l_points, float& i_points, float& f_points);
	static void		givePointsForOwnHealth		(Entity entityID, float& l_points, float& i_points, float& f_points);
	static void		givePointsForDistance	    (Entity entityID, float& l_points, float& i_points, float& f_points);
	static void		setStrategyBasedOnPoints	(Entity entityID, float& l_points, float& i_points, float& f_points);

    static Entity getFreeOrb(Entity entityID, ATTACK_STRATEGY type);

	static bool canUseAttack				(Entity entityID, std::string attack);
};

class Lich_idle : public LichBT
{
   protected:
	void start();
  public:
};

class Lich_alerted : public LichBT
{
   protected:
	void start();
  public:

};

class Lich_creep: public LichBT
{
protected:
	void start();
public:	
};

class Lich_hunt: public LichBT
{
protected:
	void start();
public:	
};


class Lich_combat : public LichBT
{
   protected:
	void start();

  public:	
};

class Lich_escape : public LichBT
{
   protected:
	void start();
  public:	
};

class Lich_dead: public LichBT
{
protected:
	void start();
public:	
};


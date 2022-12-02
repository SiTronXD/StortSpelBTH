#include "LichBTs.hpp"
#include "LichFSM.hpp"
//#include "../../../Components/Combat.h"
//#include "../../../Components/Perks.h" //TODO: Adam, add the perk stuff to Lich... ? 
#include <limits>
#include "../../../Network/ServerGameMode.h"


Scene* LichBT::getTheScene()
{
    return BehaviorTree::sceneHandler->getScene();
}

float LichBT::get_dt()
{
    return BehaviorTree::sceneHandler->getAIHandler()->getDeltaTime();
}

int LichBT::getPlayerID(Entity entityID)
{
	int playerID = -1;
    // if network exist take player from there
    NetworkScene* s = dynamic_cast<NetworkScene*>(sceneHandler->getScene());
    if (s != nullptr)
    {   
        float nearset = 99999999.0f;
        Transform& trans = s->getComponent<Transform>(entityID);
        for(auto p: *s->getPlayers())
        {
            Transform& pTrans = s->getComponent<Transform>(p);
            HealthComp& pHealth = s->getComponent<HealthComp>(p);
            float dist = glm::length(trans.position - pTrans.position);
            if(dist < nearset && pHealth.health > 0.0f)
            {
                nearset = dist;
                playerID = p;
            }
        }
        //return s->getNearestPlayer(entityID);
    }
    // else find player from script
    else
    {
        std::string playerString = "playerID";
        BehaviorTree::sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
    }
  
    return playerID;
}


void LichBT::rotateTowards(Entity entityID, glm::vec3 target, float rotSpeed, float precision)
{
	LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
	Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
	//Rotate towards target start
	lichTrans.updateMatrix();
	glm::vec2 targetPos			= glm::vec2(target.x, target.z);
	glm::vec2 lichPos			= glm::vec2(lichTrans.position.x, lichTrans.position.z);
	glm::vec2 curRot			= -safeNormalize(glm::vec2(lichTrans.forward().x, lichTrans.forward().z));
	glm::vec2 lich_to_friend	= safeNormalize(targetPos - lichPos);

	float angle_between			= glm::degrees(glm::acos(glm::dot(lich_to_friend, curRot)));
	lichComp.tempRotAngle       = angle_between;

	if(lichComp.rotateLeft && angle_between >= precision)
	{
		lichTrans.rotation.y += rotSpeed * get_dt();
	}
	else if(angle_between >= precision)
	{
		lichTrans.rotation.y -= rotSpeed * get_dt();
	}

	//Check if we rotated in correct direction
	lichTrans.updateMatrix();
	targetPos			= glm::vec2(target.x, target.z);
	lichPos				= glm::vec2(lichTrans.position.x, lichTrans.position.z);
	curRot				= -safeNormalize(glm::vec2(lichTrans.forward().x, lichTrans.forward().z));
	lich_to_friend		= safeNormalize(targetPos - lichPos);
	angle_between		= glm::degrees(glm::acos(glm::dot(lich_to_friend, curRot)));
	//If angle got bigger, then change direction
	if(lichComp.tempRotAngle < angle_between)
	{
		if(lichComp.rotateLeft)
		{
			lichComp.rotateLeft = false;
		}
		else
		{
			lichComp.rotateLeft = true;
		}
	}
	//Rotate towards target end
}
void LichBT::drawRaySimple(Ray& ray, float dist, glm::vec3 color)
{
	//Draw ray
	BehaviorTree::sceneHandler->getDebugRenderer()->renderLine(
	ray.pos,
	ray.pos + ray.dir * dist,
	glm::vec3(1.0f, 0.0f, 0.0f));
}
bool LichBT::rayChecking(Entity entityID, glm::vec3& moveDir)
{
	bool ret = true;
	bool somethingInTheWay = false;
	bool canGoForward=true;	
	bool canGoRight=true;
	bool canGoLeft=true;

	int player_id = getPlayerID(entityID);
    if(player_id == -1){return ret;}
	Collider& entityCollider = getTheScene()->getComponent<Collider>(entityID);
	Collider& playerCollider = getTheScene()->getComponent<Collider>(player_id);
	Transform& entityTransform = getTheScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = getTheScene()->getComponent<Transform>(player_id);
	LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    
	entityTransform.updateMatrix();
	glm::vec3 from = playerTransform.position;
	from = from + playerTransform.up() * 3.0f;
	glm::vec3 to = entityTransform.position;
	float maxDist = glm::length(to - from);
	glm::vec3 dir = safeNormalize(from - to);
	Ray rayToPlayer{from, -dir};    
	Ray rayRight{to, entityTransform.right()};    
	Ray rayLeft{to, -entityTransform.right()};    
	float left_right_maxDist = entityCollider.radius + 3.0f;
    RayPayload rp = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayToPlayer, maxDist);
	//drawRaySimple(rayToPlayer, maxDist);
	if(rp.hit)
	{
		if(getTheScene()->hasComponents<Collider>(rp.entity)){
            if(!getTheScene()->getComponent<Collider>(rp.entity).isTrigger &&
                rp.entity != entityID)
            {
                ret = false;
                somethingInTheWay = true;
                entityTransform.updateMatrix();

                RayPayload r_right= BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayRight, left_right_maxDist);
                RayPayload r_left = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayLeft, left_right_maxDist);
                RayPayload r_forward = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayToPlayer, left_right_maxDist);
                //drawRaySimple(rayToPlayer, left_right_maxDist);
                //drawRaySimple(rayRight, left_right_maxDist);
                //drawRaySimple(rayLeft, left_right_maxDist);

                if(r_forward.hit && !getTheScene()->getComponent<Collider>(r_forward.entity).isTrigger)
                {
                    canGoForward = false;
                }
                if(r_right.hit && !getTheScene()->getComponent<Collider>(r_right.entity).isTrigger)
                {
                    canGoRight = false;
                    lichComp.attackGoRight = false;
                }
                if(r_left.hit && !getTheScene()->getComponent<Collider>(r_left.entity).isTrigger)
                {
                    canGoLeft = false;
                    lichComp.attackGoRight = true;
                }

            }
        }
	}

	if(somethingInTheWay)
	{
		ret = false;
		dir = glm::vec3(0.0f, 0.0f, 0.0f);
		entityTransform.updateMatrix();

		if(canGoForward)
		{
			dir += -entityTransform.forward();
		}

		if(canGoRight && lichComp.attackGoRight)
		{
			dir += entityTransform.right();
		}
		else if(canGoLeft && !lichComp.attackGoRight)
		{
			dir -= entityTransform.right();
		}
	}

	if(dir == glm::vec3(0.0f, 0.0f, 0.0f))
	{
		dir = -entityTransform.forward();
	}
	rotateTowards(entityID, playerTransform.position, lichComp.creepRotSpeed, 5.0f);
	safeNormalize(dir);
	dir.y = 0;
	moveDir = dir;


	return ret;
}

//Giving points for attack strategy
void LichBT::givePointsForPlayerHealth	(Entity entityID, float& l_points, float& i_points, float& f_points)
{
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return;}
    int playerHealth = 0;
    HealthComp& playerHealthComp = getTheScene()->getComponent<HealthComp>(playerID);
    playerHealth = playerHealthComp.health;

    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);

    //Points based on player health
    static const float pointsForPlayerHealth = 1.5f;
    if(playerHealth <= lichComp.attacks["lightning"].damage){l_points+=pointsForPlayerHealth;}//Player low health
    else if(playerHealth > lichComp.attacks["lightning"].damage && playerHealth <= lichComp.attacks["ice"].damage){i_points+=pointsForPlayerHealth;}//Player medium health
    else if(playerHealth > lichComp.attacks["ice"].damage){f_points+=pointsForPlayerHealth;}//Player high health
}
void LichBT::givePointsForOwnHealth		(Entity entityID, float& l_points, float& i_points, float& f_points)
{
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);

    //Points based on own health
    //Player damage 50-150
    static const float pointsForLichHealth = 1.2f;
    if(lichComp.life <= 50){l_points+=pointsForLichHealth;}//Lich low health
    else if(lichComp.life > 50 && lichComp.life <= 150) {i_points+=pointsForLichHealth;}//Lich medium health
    else if(lichComp.life > 150){f_points+=pointsForLichHealth;}//Lich high health
}
void LichBT::givePointsForDistance	    (Entity entityID, float& l_points, float& i_points, float& f_points)
{
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return;}
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    float dist = glm::length(playerTrans.position - lichTrans.position);

     //points baes on distance from player
    static const float pointsForDistance = 1.0f;
    if(dist <= lichComp.nonoRadius){l_points+=pointsForDistance;}//Close to player
    else if(dist > lichComp.nonoRadius && dist <= lichComp.peronalSpaceRadius) {i_points+=pointsForDistance;}//Medium distance to player
    else if(dist > lichComp.peronalSpaceRadius){f_points+=pointsForDistance;}//Far away from player
}
void LichBT::setStrategyBasedOnPoints	(Entity entityID, float& l_points, float& i_points, float& f_points)
{
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(l_points > i_points && l_points > f_points && lichComp.attacks["lightning"].manaCost <= lichComp.mana)
    {
        if(lichComp.attacks["lightning"].cooldownTimer <= 0.0f)
        {
            //Lightning attack!
            lichComp.curAttack = &lichComp.attacks["lightning"];
            lichComp.lastAttack = "lightning";
        }
    }
    else  if(i_points > l_points && i_points > f_points && lichComp.attacks["ice"].manaCost <= lichComp.mana)
    {
        if(lichComp.attacks["ice"].cooldownTimer <= 0.0f)
        {
            //ice attack!
             lichComp.curAttack = &lichComp.attacks["ice"];
             lichComp.lastAttack = "ice";
        }
    }
    else  if(f_points > l_points && f_points > i_points && lichComp.attacks["fire"].manaCost <= lichComp.mana)
    {
        if(lichComp.attacks["fire"].cooldownTimer <= 0.0f)
        {
            //fire attack!
             lichComp.curAttack = &lichComp.attacks["fire"];
             lichComp.lastAttack = "fire";
        }
    }
}

Entity LichBT::getFreeOrb(Entity entityID, ATTACK_STRATEGY type)
{
    int projectileID = -1;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.curAttack->type == ATTACK_STRATEGY::FIRE){

        for(size_t i = 0; i < LichComponent::NR_FIRE_ORBS;i++)
        {
            if(!getTheScene()->isActive(lichComp.fireOrbs[i]))
            {projectileID = lichComp.fireOrbs[i]; getTheScene()->setActive(lichComp.fireOrbs[i]); break;}
        }
    }
    else if(lichComp.curAttack->type == ATTACK_STRATEGY::ICE)
    {

        for(size_t i = 0; i < LichComponent::NR_ICE_ORBS;i++)
        {
            if(!getTheScene()->isActive(lichComp.iceOrbs[i]))
            {projectileID = lichComp.iceOrbs[i]; getTheScene()->setActive(lichComp.iceOrbs[i]); break;}
        }
    }
    else if(lichComp.curAttack->type == ATTACK_STRATEGY::LIGHT)
    {
        for(size_t i = 0; i < LichComponent::NR_LIGHT_ORBS;i++)
        {
            if(!getTheScene()->isActive(lichComp.lightOrbs[i]))
            {projectileID = lichComp.lightOrbs[i]; getTheScene()->setActive(lichComp.lightOrbs[i]); break;}

        }
    }
    return projectileID;
}

bool LichBT::canUseAttack(Entity entityID, std::string attack)
{
    bool ret = false;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    float cost = lichComp.attacks[attack].manaCost;
    float timer = lichComp.attacks[attack].cooldownTimer;
    if(lichComp.mana >= cost && timer <= 0.0f)
    {
        ret = true;
    }
    return ret;
}

void LichBT::registerEntityComponents(Entity entityId)
{
    this->addRequiredComponent<LichComponent>(entityId);
}

BTStatus LichBT::plunder(Entity entityID)
{
    BTStatus ret = BTStatus::Running;

    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    
    if(lichComp.timeSincePlunderBegin + lichComp.plunderDuration < Time::getTimeSinceStart())
    {
        lichComp.carryingBones = true; 
        lichComp.numBones = 1; 
        ret = BTStatus::Success;
    }

    return ret;
}

BTStatus LichBT::goToGrave(Entity entityID)
{

    BTStatus ret = BTStatus::Running;
    Transform& lichTrans    = getTheScene()->getComponent<Transform>(entityID);
    Rigidbody& lichRb       = getTheScene()->getComponent<Rigidbody>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    Transform& graveTrans   = getTheScene()->getComponent<Transform>(lichComp.graveID);

    glm::vec3 moveDir		= pathFindingManager.getDirTo(lichTrans.position, graveTrans.position);
    avoidStuff(entityID, BehaviorTree::sceneHandler, lichComp.attackGoRight, graveTrans.position, moveDir, glm::vec3(0.0f, -3.0f, 0.0f));
    lichRb.velocity = moveDir * lichComp.speed;
    rotateTowards(entityID, graveTrans.position, lichComp.idleTurnSpeed);

    float distToGrave = glm::length(graveTrans.position - lichTrans.position);

    if(distToGrave < lichComp.closeToGrave)
    {
        ret = BTStatus::Success;
        lichComp.timeSincePlunderBegin = Time::getTimeSinceStart();
    }

    return ret;
}

BTStatus LichBT::goToAlter(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    Transform& lichTrans    = getTheScene()->getComponent<Transform>(entityID);
    Rigidbody& lichRb       = getTheScene()->getComponent<Rigidbody>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    Transform& alterTrans   = getTheScene()->getComponent<Transform>(lichComp.alterID);

    glm::vec3 moveDir		= pathFindingManager.getDirTo(lichTrans.position, alterTrans.position);
    avoidStuff(entityID, BehaviorTree::sceneHandler, lichComp.attackGoRight, alterTrans.position, moveDir, glm::vec3(0.0f, -3.0f, 0.0f));
	moveDir = safeNormalize(moveDir);
    lichRb.velocity = moveDir * lichComp.speed;
    rotateTowards(entityID, alterTrans.position, lichComp.idleTurnSpeed);


    return ret;
}

BTStatus LichBT::dropOffBones(Entity entityID)
{
    BTStatus ret = BTStatus::Success;    
    //TODO: Visualise the boes dropping
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    
    if(lichComp.timeSinceAlterWaitBegin + lichComp.DropOffDuration < Time::getTimeSinceStart())
    {
        lichComp.carryingBones = false; 
        lichComp.numBones = 0; 
        ret = BTStatus::Success;
    }
    return ret;
}

BTStatus LichBT::carryingBones(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.numBones > 0)
    {
        ret = BTStatus::Success;
    }
    return ret;
}

BTStatus LichBT::closeToGrave(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    Transform& lichTrans    = getTheScene()->getComponent<Transform>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    Transform& graveTrans   = getTheScene()->getComponent<Transform>(lichComp.graveID);
    float distToGrave = glm::length(graveTrans.position - lichTrans.position);

    if(distToGrave < lichComp.closeToGrave)
    {
        ret = BTStatus::Success;        
    }
    else
    {
        lichComp.timeSincePlunderBegin = Time::getTimeSinceStart();
    }
    return ret;
}

BTStatus LichBT::closeToAlter(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    Transform& lichTrans    = getTheScene()->getComponent<Transform>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    Transform& alterTrans   = getTheScene()->getComponent<Transform>(lichComp.alterID);
    float distToAlter = glm::length(alterTrans.position - lichTrans.position);

    if(distToAlter < lichComp.closeToAlter)
    {
        ret = BTStatus::Success;        
    }
    else
    {
        lichComp.timeSinceAlterWaitBegin = Time::getTimeSinceStart();
    }
    return ret;
}

BTStatus LichBT::creepyLook(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    rotateTowards(entityID, playerTrans.position, lichComp.creepRotSpeed);
    return ret;
}

BTStatus LichBT::huntingPlayer(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    Rigidbody& lichRb = getTheScene()->getComponent<Rigidbody>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    glm::vec3 moveDir		= pathFindingManager.getDirTo(lichTrans.position, playerTrans.position);

    rayChecking(entityID, moveDir);

	moveDir = safeNormalize(moveDir);
    avoidStuff(entityID, BehaviorTree::sceneHandler, lichComp.attackGoRight, playerTrans.position, moveDir, glm::vec3(0.0f, -3.0f, 0.0f));
    lichRb.velocity = moveDir * lichComp.huntSpeed;
    rotateTowards(entityID, playerTrans.position, lichComp.huntRotSpeed);

    float dist = glm::length(playerTrans.position - lichTrans.position);
    if(dist <= lichComp.sightRadius)
    {
        ret = BTStatus::Success;
    }

    return ret;
}

BTStatus LichBT::playerInNoNoZone(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    float dist = glm::length(playerTrans.position -  lichTrans.position);
    if(dist < lichComp.nonoRadius)
    {
        ret = BTStatus::Success;
    }
    return ret;
}

BTStatus LichBT::moveAwayFromPlayer(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    Rigidbody& lichRb = getTheScene()->getComponent<Rigidbody>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    glm::vec3 moveDir		= pathFindingManager.getDirTo(lichTrans.position, playerTrans.position);
    //avoidStuff(entityID, BehaviorTree::sceneHandler, lichComp.attackGoRight, playerTrans.position, moveDir, glm::vec3(0.0f, -3.0f, 0.0f)); //TODO: Check if this improves or not
	moveDir = -safeNormalize(moveDir);
    lichRb.velocity = moveDir * lichComp.huntSpeed;

    rotateTowards(entityID, playerTrans.position, lichComp.huntRotSpeed);


    return ret;
}

BTStatus LichBT::notEnoughMana(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if (lichComp.mana < lichComp.attacks["lightning"].manaCost && !lichComp.regeningMana)
    {
        ret = BTStatus::Success;
        lichComp.regeningMana = true;
    }
    else if(lichComp.regeningMana)
    {
        ret = BTStatus::Success;
    }

    return ret;
}

BTStatus LichBT::hasStrategy(Entity entityID)
{
    BTStatus ret = BTStatus::Success;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.curAttack == nullptr)
    {
        ret = BTStatus::Failure;
    }

    return ret;
}

BTStatus LichBT::regenerateMana(Entity entityID)
{
    BTStatus ret =  BTStatus::Success;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.mana <= lichComp.maxMana)
    {
        lichComp.mana+=lichComp.manaRegenSpeed*get_dt();
    }
    else
    {
        lichComp.mana = lichComp.mana;
        lichComp.regeningMana = false;
        ret = BTStatus::Failure;
    }


    return ret;
}

BTStatus LichBT::pickBestStrategy(Entity entityID)
{
    BTStatus ret = BTStatus::Success;

    float lightningPoints       = 0.0f;   
    float icePoints             = 0.0f;
    float firePoints            = 0.0f;

    int playerID                = getPlayerID(entityID);
    if(playerID == -1){return ret;}
    int playerHealth            = 0;

    //singleplayer
    HealthComp& playerHealthComp        = getTheScene()->getComponent<HealthComp>(playerID);
    playerHealth                = playerHealthComp.health;
    

    Transform& playerTrans      = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans        = getTheScene()->getComponent<Transform>(entityID);
    LichComponent& lichComp     = getTheScene()->getComponent<LichComponent>(entityID);

    

    givePointsForPlayerHealth   (entityID, lightningPoints, icePoints, firePoints);
    givePointsForOwnHealth      (entityID, lightningPoints, icePoints, firePoints);
    givePointsForDistance       (entityID, lightningPoints, icePoints, firePoints);
   


    setStrategyBasedOnPoints    (entityID, lightningPoints, icePoints, firePoints);
   

    if( lichComp.curAttack == nullptr)
    {
        pickRandomStrategy(entityID);
    }
    
    return ret;
}

BTStatus LichBT::pickRandomStrategy(Entity entityID)
{
    BTStatus ret = BTStatus::Success;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    std::vector<ATTACK_STRATEGY>validChoises;


    if(canUseAttack(entityID,"lightning"))
    {
        validChoises.push_back(ATTACK_STRATEGY::LIGHT);
    }
    if (canUseAttack(entityID, "ice"))
    {
        validChoises.push_back(ATTACK_STRATEGY::ICE);
    }
    if(canUseAttack(entityID, "fire"))
    {
        validChoises.push_back(ATTACK_STRATEGY::FIRE);
    }

    if(validChoises.size() > 0)
    {
        ATTACK_STRATEGY randStrat =  validChoises[rand()%validChoises.size()];
        switch (randStrat)
        {
        case ATTACK_STRATEGY::LIGHT:
            lichComp.curAttack = &lichComp.attacks["lightning"];
            lichComp.lastAttack = "lightning";
            break;
        case ATTACK_STRATEGY::ICE:
            lichComp.curAttack = &lichComp.attacks["ice"];
            lichComp.lastAttack = "ice";
            break;
        case ATTACK_STRATEGY::FIRE:
            lichComp.curAttack = &lichComp.attacks["fire"];
            lichComp.lastAttack = "fire";
            break;
        }
    }
    else
    {
        lichComp.curAttack = nullptr;
    }


    return ret;
}

BTStatus LichBT::attack(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    rotateTowards(entityID, playerTrans.position, lichComp.huntRotSpeed);

    if(lichComp.curAttack == nullptr){return ret;}

    //Tick down cast time for current strategy
    if(lichComp.curAttack->castTimeTimer > 0.0f)
    {
        lichComp.curAttack->castTimeTimer -= get_dt();
    }
    else
    {
        //Remove mana
        lichComp.mana -= lichComp.curAttack->manaCost;
       
        //Reset cast times
        lichComp.curAttack->castTimeTimer = lichComp.curAttack->castTimeTimerOrig;
        lichComp.curAttack->cooldownTimer = lichComp.curAttack->cooldownTimerOrig;

        // Cast Projectile stuff
        Transform&  lichTrans = getTheScene()->getComponent<Transform>(entityID);
        lichTrans.updateMatrix();

        //Pick Projectile 
        Entity projectileID = LichBT::getFreeOrb(entityID, lichComp.curAttack->type);
        
        if(projectileID != -1){

            Orb& orb = getTheScene()->getComponent<Orb>(projectileID);

            ServerGameMode* netScene = dynamic_cast<ServerGameMode*>(getTheScene());
            if (netScene == nullptr)
            {            
            
                //Shoot projectile!
                Transform&  orbTrans = getTheScene()->getComponent<Transform>(projectileID);
                Rigidbody&  orbRB    = getTheScene()->getComponent<Rigidbody>(projectileID);
                
                orbTrans.position = lichTrans.position + (-lichTrans.forward() * (float)(LichComponent::colliderRadius + LichComponent::orbRadius + LichComponent::orbSpawnDistFrom));
                auto spellVector = safeNormalize(playerTrans.position - lichTrans.position) * LichComponent::spellForce;
                spellVector.y = 0;  //TODO: What if player is on top of something... Will not aim att player
                orbRB.velocity = spellVector;
                orb.orbPower = lichComp.curAttack;                
                            
            }
            else
            {
                glm::vec3 initialOrbPos = lichTrans.position + (-lichTrans.forward() * (float)(LichComponent::colliderRadius + LichComponent::orbRadius + LichComponent::orbSpawnDistFrom));
                glm::vec3 spellVector = safeNormalize(playerTrans.position - lichTrans.position) * LichComponent::spellForce;
                spellVector.y = 0;  //TODO: What if player is on top of something... Will not aim att player
                
                netScene->addEvent({(int)GameEvent::THROW_ORB, (int)projectileID },{initialOrbPos.x,initialOrbPos.y,initialOrbPos.z, spellVector.x, spellVector.y,spellVector.z});
            }

            orb.timeAtCast = Time::getTimeSinceStart();
        }

        //Remove current strat
        Log::write("Current attack: "+std::to_string((int)lichComp.curAttack->type), BT_FILTER);
        lichComp.curAttack = nullptr;

        ret = BTStatus::Success;
       
    }
    return ret;
}

BTStatus LichBT::selfHeal(Entity entityID)
{
    BTStatus ret =  BTStatus::Running;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.life < lichComp.FULL_HEALTH)
    {
        lichComp.life_float += get_dt() * lichComp.healthRegenSpeed;
        if(lichComp.life_float > 1.0f)
        {
            lichComp.life++;
            lichComp.life_float = 0.0f;
        }
    }
    else
    {
         lichComp.life = lichComp.FULL_HEALTH;
         ret =  BTStatus::Success;
    }
    return ret;

}

BTStatus LichBT::playerNotVisible(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    LichComponent lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    float dist = glm::length(playerTrans.position -  lichTrans.position);
   /* if(dist > lichComp.sightRadius)
    {
        ret = BTStatus::Success;
    }*/
    if(dist > lichComp.attackRadius)
    {
        ret = BTStatus::Success;
    }

    return ret;
}

BTStatus LichBT::runAwayFromPlayer(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    Rigidbody& lichRb = getTheScene()->getComponent<Rigidbody>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    glm::vec3 moveDir		= pathFindingManager.getDirTo(lichTrans.position, playerTrans.position);
    //avoidStuff(entityID, BehaviorTree::sceneHandler, lichComp.attackGoRight, playerTrans.position, moveDir, glm::vec3(0.0f, -3.0f, 0.0f)); //TODO: Check if this improves or not
	moveDir = -safeNormalize(moveDir);
    lichRb.velocity = moveDir * lichComp.huntSpeed;
    //rotateTowards(entityID, playerTrans.position, lichComp.huntRotSpeed);

    
    glm::vec3 player_to_lich = safeNormalize(lichTrans.position - playerTrans.position);
    glm::vec3 lookAtPos = lichTrans.position + player_to_lich * 2.0f;
    rotateTowards(entityID, lookAtPos, lichComp.huntRotSpeed);

    return ret;
}

BTStatus LichBT::playDeathAnim(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
	LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
	Transform& lichTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
	if(lichTrans.scale.y <= 0.0f)
	{
		ret = BTStatus::Success;
	}
	else
	{
		lichTrans.rotation.y +=  1000*lichComp.deathAnimSpeed*get_dt();
		lichTrans.scale.y -= lichComp.deathAnimSpeed*get_dt();
	}

	return ret;
}

BTStatus LichBT::die(Entity entityID)
{
   BTStatus ret = BTStatus::Failure;

	int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
   //TODO : this was changed from combat to networkCombat see if it works
	HealthComp& playerHealth = sceneHandler->getScene()->getComponent<HealthComp>(playerID);
	if (playerHealth.health <= (playerHealth.maxHealth - 10))
	{
        playerHealth.health += 10;
	}

	getTheScene()->setInactive(entityID);
    ServerGameMode* serverScene = dynamic_cast<ServerGameMode*>(sceneHandler->getScene());
    if (serverScene != nullptr) 
    {
        serverScene->addEvent({(int)GameEvent::INACTIVATE, entityID});
    }

	return ret;
}

BTStatus LichBT::alerted(Entity entityID)
{
    BTStatus ret = BTStatus::Running;

	LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
	int playerID = getPlayerID(entityID);
    if(playerID == -1){return ret;}
	Transform& playerTransform = getTheScene()->getComponent<Transform>(playerID);
	Transform& lichTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
	Collider& lichCol = sceneHandler->getScene()->getComponent<Collider>(entityID);
	float toMove = (lichCol.radius*2) * (1.0f - lichComp.origScaleY + lichComp.alertScale);
	
	lichTrans.rotation.y = lookAtY(lichTrans, playerTransform);
	lichTrans.updateMatrix();

	if(!lichComp.alertAtTop)
	{
		if(lichTrans.scale.y >= lichComp.origScaleY + lichComp.alertScale &&
		lichTrans.position.y >= (lichComp.alertTempYpos + toMove))
		{
			lichComp.alertAtTop = true;
		}
		else
		{
			if (lichTrans.scale.y < lichComp.origScaleY + lichComp.alertScale)
			{
				lichTrans.scale.y += lichComp.alertAnimSpeed * get_dt();
			}
			if (lichTrans.position.y < (lichComp.alertTempYpos + toMove))
			{
				lichTrans.position.y += lichComp.alertAnimSpeed * get_dt();
			}
		}
	}
	else
	{
		if(lichTrans.scale.y <= lichComp.origScaleY)
		{
			lichTrans.scale.y = lichComp.origScaleY;
			lichTrans.position.y = lichComp.alertTempYpos;
			lichComp.alertAtTop = false;
			lichComp.alertDone = true;
			ret = BTStatus::Success;
		}
		else
		{
			if(lichTrans.scale.y > 1.0)
			{
				lichTrans.scale.y -= lichComp.alertAnimSpeed * get_dt();
			}
		}
	}

	return ret;
}

void Lich_idle::start()
{
    Selector* pickDestination = c.c.selector();

    Sequence* checkIfBonesExists = c.c.sequence();
    Selector* plunderInProgress = c.c.selector();

    Condition* hasBones = c.l.condition("HasBones", LichBT::carryingBones);
    Selector* returningWithBones = c.c.selector();
    Sequence* plunderIfCloseToGrave = c.c.sequence();
    Task* goToGrave     = c.l.task("Go to grave", LichBT::goToGrave);

    Sequence* leaveBonesIfCloseToAlter = c.c.sequence();
    Task* goToAlter     = c.l.task("Go to Alter", LichBT::goToAlter);
    Condition* isCloseToGrave = c.l.condition("close to Grave", LichBT::closeToGrave);
    Task* plunderGrave  = c.l.task("Plunder grave", LichBT::plunder);

    Condition* isCloseToAlter = c.l.condition("close to Alter", LichBT::closeToAlter);
    Task* dropOffBones  = c.l.task("Drop off bones at alter", LichBT::dropOffBones);

    pickDestination->addCompositors({checkIfBonesExists, plunderInProgress});
    
    checkIfBonesExists->addFallback({{hasBones},returningWithBones});
    plunderInProgress->addCompositor(plunderIfCloseToGrave);
    plunderInProgress->addLeaf(goToGrave);

    returningWithBones->addCompositor(leaveBonesIfCloseToAlter);
    returningWithBones->addLeaf(goToAlter);

    plunderIfCloseToGrave->addLeafs({isCloseToGrave, plunderGrave});

    leaveBonesIfCloseToAlter->addLeafs({isCloseToAlter, dropOffBones});




    this->setRoot(pickDestination);
}

void Lich_alerted::start()
{
    Sequence*   actAlerted          = c.c.sequence();

    Decorator*  alwaysTrue          = c.d.forceSuccess();    
    Task*       alertedAnimation    = c.l.task("Alerted Animation", LichBT::alerted);

    Sequence*   handlePickedUpBones = c.c.sequence();
    Condition*  isCarryingBones     = c.l.condition("carrying Bone", LichBT::carryingBones);
    Task*       dropOffBones        = c.l.task("Drop off bones at alter", LichBT::dropOffBones);

    actAlerted->addDecorator(alwaysTrue);
    actAlerted->addLeaf(alertedAnimation);

    alwaysTrue->addCompositor(handlePickedUpBones);
    handlePickedUpBones->addLeafs({isCarryingBones,dropOffBones});    
    
    this->setRoot(actAlerted);
}

void Lich_creep::start()
{
    Task*   creepyLook  = c.l.task("Creepy look at player", LichBT::creepyLook);
    this->setRoot(creepyLook);
}

void Lich_hunt::start()
{
    Task*   huntDownPlayer  = c.l.task("Hunting player", LichBT::huntingPlayer);
    this->setRoot(huntDownPlayer);
}

void Lich_combat::start()
{
    Sequence*   root          = c.c.sequence();

    ForceSuccess*  alwaysTrue           = c.d.forceSuccess();    
    Selector*   attackIfManaExists      = c.c.selector();


    Sequence*   avoidPlayerInNoNoZone   = c.c.sequence();
    Sequence*   regenerateManaIfNeeded  = c.c.sequence();
    Sequence*   attackSeq               = c.c.sequence();


    Condition*  playerInNoNoZone        = c.l.condition("player in NoNo zone",  LichBT::playerInNoNoZone);
    Task*       moveAwayFromPlayer      = c.l.task("moving away from player",   LichBT::moveAwayFromPlayer);
    Condition*  noManaToAttack          = c.l.condition("must generte mana",    LichBT::notEnoughMana);
    Task*       regenerateMana          = c.l.task("generating mana",           LichBT::regenerateMana);
    Selector*   shouldPickNewStrat      = c.c.selector();
    Task*       doTheAttack             = c.l.task("Do the attack",         LichBT::attack);
        

    Condition*  hasStrat                = c.l.condition("must generte mana",    LichBT::hasStrategy);
    Random*     pickChoice              = c.d.random();

    Task*       smartStrategy           = c.l.task("picked strategy smart",       LichBT::pickBestStrategy);
    Task*       randomStrategy          = c.l.task("picket strategy random",      LichBT::pickRandomStrategy);


    root->addDecorator(alwaysTrue);
    root->addCompositor(attackIfManaExists);

    alwaysTrue->addCompositor(avoidPlayerInNoNoZone);
    attackIfManaExists->addCompositor(regenerateManaIfNeeded);

    attackIfManaExists->addCompositor(attackSeq);


    avoidPlayerInNoNoZone->addLeafs({playerInNoNoZone,moveAwayFromPlayer});
    regenerateManaIfNeeded->addLeafs({noManaToAttack,regenerateMana});
    attackSeq->addCompositor(shouldPickNewStrat);
    attackSeq->addLeaf(doTheAttack);

    shouldPickNewStrat->addLeaf(hasStrat);
    shouldPickNewStrat->addDecorator(pickChoice);

    pickChoice->addLeafs({smartStrategy, randomStrategy});

    this->setRoot(root);
}

void Lich_escape::start()
{
    Sequence* escapeToHeal = c.c.sequence();

    Selector*  checkIfShouldRun     = c.c.selector();
    Task*      selfHeal             = c.l.task("Healing self",              LichBT::selfHeal);
    Condition* playerNotVisible     = c.l.condition("player not Visible",   LichBT::playerNotVisible);    
    Task*      runAwayFromPlayer    = c.l.task("running from player",       LichBT::runAwayFromPlayer);

    escapeToHeal->addCompositor(checkIfShouldRun);
    escapeToHeal->addLeaf(selfHeal);

    checkIfShouldRun->addLeafs({playerNotVisible,runAwayFromPlayer});

    this->setRoot(escapeToHeal);
}

void Lich_dead::start()
{
    Sequence*   animateThenDie  = c.c.sequence();
    Task*   die  = c.l.task("dies", LichBT::die);
    Task*   deathAnim  = c.l.task("dies", LichBT::playDeathAnim);

    animateThenDie->addLeafs({deathAnim, die});

    this->setRoot(animateThenDie);
}

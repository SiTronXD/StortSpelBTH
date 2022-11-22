#include "LichBTs.hpp"
#include "LichFSM.hpp"
//#include "../../../Components/Combat.h"
//#include "../../../Components/Perks.h" //TODO: Adam, add the perk stuff to Lich... ? 
#include <limits>


Scene* LichBT::getTheScene()
{
    return BehaviorTree::sceneHandler->getScene();
}

float LichBT::get_dt()
{
    return BehaviorTree::sceneHandler->getAIHandler()->getDeltaTime();
}

int LichBT::getPlayerID()
{
    int playerID = -1;
    std::string playerId_str = "playerID";
    BehaviorTree::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str);
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
	glm::vec2 curRot			= -glm::normalize(glm::vec2(lichTrans.forward().x, lichTrans.forward().z));
	glm::vec2 lich_to_friend	= glm::normalize(targetPos - lichPos);

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
	curRot				= -glm::normalize(glm::vec2(lichTrans.forward().x, lichTrans.forward().z));
	lich_to_friend		= glm::normalize(targetPos - lichPos);
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

void LichBT::registerEntityComponents(Entity entityId)
{
    this->addRequiredComponent<LichComponent>(entityId);
}

BTStatus LichBT::plunder(Entity entityID)
{
    return BTStatus::Failure;
}

BTStatus LichBT::goToGrave(Entity entityID)
{
    return BTStatus::Failure;
}

BTStatus LichBT::goToAlter(Entity entityID)
{
    return BTStatus::Failure;
}

BTStatus LichBT::dropOffBones(Entity entityID)
{
    BTStatus ret = BTStatus::Success;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    lichComp.numBones = 0;
    //TODO: Visualise the boes dropping
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

BTStatus LichBT::creepyLook(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    int playerID = getPlayerID();
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    rotateTowards(entityID, playerTrans.position, lichComp.creepRotSpeed);
    return ret;
}

BTStatus LichBT::huntingPlayer(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    
    int playerID = getPlayerID();
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    Rigidbody& lichRb = getTheScene()->getComponent<Rigidbody>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    glm::vec3 moveDir		= pathFindingManager.getDirTo(lichTrans.position, playerTrans.position);
	moveDir = glm::normalize(moveDir);
    lichRb.velocity = moveDir * lichComp.huntSpeed;
    rotateTowards(entityID, playerTrans.position, lichComp.huntRotSpeed);

    return ret;
}

BTStatus LichBT::playerInNoNoZone(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    int playerID = getPlayerID();
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
    return BTStatus::Success;
}

BTStatus LichBT::hasMana(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.mana > 0)
    {
        ret = BTStatus::Success;
    }

    return BTStatus::Failure;
}

BTStatus LichBT::hasStrategy(Entity entityID)
{
    BTStatus ret = BTStatus::Success;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.strat == ATTACK_STRATEGY::NONE)
    {
        ret = BTStatus::Failure;
    }

    return ret;
}

BTStatus LichBT::regenerateMana(Entity entityID)
{
    return BTStatus::Success;
}

BTStatus LichBT::pickBestStrategy(Entity entityID)
{
    BTStatus ret = BTStatus::Success;

    float lightningPoints = 0.0f; 
    float icePoints = 0.0f;
    float firePoints = 0.0f;

    int playerID = getPlayerID();
    int playerHealth = 0;

    Combat& playerCombat = getTheScene()->getComponent<Combat>(playerID);
    playerHealth = playerCombat.health;
    

    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    float dist = glm::length(playerTrans.position - lichTrans.position);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);

    //Points based on player health
    static const float pointsForPlayerHealth = 1.5f;
    if(playerHealth <= lichComp.lightning.damage){lightningPoints+=pointsForPlayerHealth;}//Player low health
    else if(playerHealth > lichComp.lightning.damage && playerHealth <= lichComp.ice.damage){icePoints+=pointsForPlayerHealth;}//Player medium health
    else if(playerHealth > lichComp.ice.damage){firePoints+=pointsForPlayerHealth;}//Player high health

    //Points based on own health
    //Player damage 50-150
    static const float pointsForLichHealth = 1.5f;
    if(lichComp.life <= 50){lightningPoints+=pointsForLichHealth;}//Lich low health
    else if(lichComp.life > 50 && lichComp.life <= 150) {icePoints+=pointsForLichHealth;}//Lich medium health
    else if(lichComp.life > 150){firePoints+=pointsForLichHealth;}//Lich high health

    //points baes on distance from player
    static const float pointsForDistance = 1.0f;
    if(dist <= lichComp.nonoRadius){lightningPoints+=pointsForDistance;}//Close to player
    else if(dist > lichComp.nonoRadius && dist <= lichComp.peronalSpaceRadius) {icePoints+=pointsForDistance;}//Medium distance to player
    else if(dist > lichComp.peronalSpaceRadius){firePoints+=pointsForDistance;}//Far away from player



    if(lightningPoints > icePoints && lightningPoints > firePoints && lichComp.lightning.manaCost <= lichComp.mana)
    {
        //Lightning attack!
        lichComp.strat = ATTACK_STRATEGY::LIGHT;
    }
    else  if(icePoints > lightningPoints && icePoints > firePoints && lichComp.ice.manaCost <= lichComp.mana)
    {
        //ice attack!
        lichComp.strat = ATTACK_STRATEGY::ICE;
    }
    else  if(firePoints > lightningPoints && firePoints > icePoints && lichComp.fire.manaCost <= lichComp.mana)
    {
        //fire attack!
        lichComp.strat = ATTACK_STRATEGY::FIRE;
    }
    else
    {
        pickRandomStrategy(entityID);
    }
    
    return ret;
}

BTStatus LichBT::pickRandomStrategy(Entity entityID)
{
    BTStatus ret = BTStatus::Success;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if (lichComp.mana >= lichComp.lightning.manaCost && lichComp.mana < lichComp.ice.manaCost)
    {
         lichComp.strat =  ATTACK_STRATEGY::LIGHT;
    }
    else if(lichComp.mana >= lichComp.lightning.manaCost && lichComp.mana < lichComp.fire.manaCost)
    {
        //Select ligh or ice
        lichComp.strat = ATTACK_STRATEGY(rand()%(ATTACK_STRATEGY::_LAST-2)+1);
    }
    else if(lichComp.mana >= lichComp.lightning.manaCost && lichComp.mana >= lichComp.fire.manaCost)
    {
        //Select ligh or ice or fire
        lichComp.strat = ATTACK_STRATEGY(rand()%(ATTACK_STRATEGY::_LAST-1)+1);
    }
    else
    {
        lichComp.strat = ATTACK_STRATEGY::NONE;
    }
    return ret;
}

BTStatus LichBT::attack(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.tempAttack)
    {
        lichComp.tempAttack = false;
        switch (lichComp.strat)
        {
        case ATTACK_STRATEGY::LIGHT:
            lichComp.mana -= lichComp.lightning.manaCost;
            break;
        case ATTACK_STRATEGY::ICE:
            lichComp.mana -= lichComp.ice.manaCost;
            break;
        case ATTACK_STRATEGY::FIRE:
            lichComp.mana -= lichComp.fire.manaCost;
            break;
        }
        lichComp.strat = ATTACK_STRATEGY::NONE;
        ret = BTStatus::Success;
    }
    return BTStatus();
}

BTStatus LichBT::selfHeal(Entity entityID)
{
    return BTStatus::Failure;
}

BTStatus LichBT::playerNotVisible(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
    int playerID = getPlayerID();
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    LichComponent lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    float dist = glm::length(playerTrans.position -  lichTrans.position);
    if(dist > lichComp.sightRadius)
    {
        ret = BTStatus::Success;
    }

    return ret;
}

BTStatus LichBT::runAwayFromPlayer(Entity entityID)
{
    return BTStatus::Failure;
}

BTStatus LichBT::playDeathAnim(Entity entityID)
{
    return BTStatus::Failure;
}

BTStatus LichBT::die(Entity entityID)
{
   BTStatus ret = BTStatus::Failure;

	int playerID = getPlayerID();
	Combat& playerCombat = sceneHandler->getScene()->getComponent<Combat>(playerID);
	if (playerCombat.health <= (playerCombat.maxHealth - 10))
	{
		playerCombat.health += 10;
	}

	getTheScene()->setInactive(entityID);

	return ret;
}

BTStatus LichBT::alerted(Entity entityID)
{
    BTStatus ret = BTStatus::Running;

	LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
	int playerID = getPlayerID();
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
    Sequence* keepOnPlunder = c.c.sequence();

    Task* goToGrave     = c.l.task("Go to grave", LichBT::goToGrave);
    Task* plunderGrave  = c.l.task("Plunder grave", LichBT::plunder);
    Task* goToAlter     = c.l.task("Go to Alter", LichBT::goToAlter);
    Task* dropOffBones  = c.l.task("Drop off bones at alter", LichBT::dropOffBones);

    keepOnPlunder->addLeafs({goToGrave,plunderGrave,goToAlter,dropOffBones});

    this->setRoot(keepOnPlunder);
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
    Condition*  noManaToAttack          = c.l.condition("must generte mana",    LichBT::hasMana);
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

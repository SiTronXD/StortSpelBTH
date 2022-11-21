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
    return BTStatus::Failure;
}

BTStatus LichBT::carryingBones(Entity entityID)
{
    return BTStatus::Failure;
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
    return BTStatus::Failure;
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
    return BTStatus::Failure;
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

BTStatus LichBT::regenerateMana(Entity entityID)
{
    return BTStatus::Failure;
}

BTStatus LichBT::pickBestStrategy(Entity entityID)
{
    return BTStatus::Failure;
}

BTStatus LichBT::pickRandomStrategy(Entity entityID)
{
    return BTStatus::Failure;
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

BTStatus LichBT::attack(Entity entityID)
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
	Transform& tankTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
	Collider& tankCol = sceneHandler->getScene()->getComponent<Collider>(entityID);
	float toMove = (tankCol.radius*2) * (1.0f - lichComp.origScaleY + lichComp.alertScale);
	
	tankTrans.rotation.y = lookAtY(tankTrans, playerTransform);
	tankTrans.updateMatrix();

	if(!lichComp.alertAtTop)
	{
		if(tankTrans.scale.y >= lichComp.origScaleY + lichComp.alertScale &&
		tankTrans.position.y >= (lichComp.alertTempYpos + toMove))
		{
			lichComp.alertAtTop = true;
		}
		else
		{
			if (tankTrans.scale.y < lichComp.origScaleY + lichComp.alertScale)
			{
				tankTrans.scale.y += lichComp.alertAnimSpeed * get_dt();
			}
			if (tankTrans.position.y < (lichComp.alertTempYpos + toMove))
			{
				tankTrans.position.y += lichComp.alertAnimSpeed * get_dt();
			}
		}
	}
	else
	{
		if(tankTrans.scale.y <= lichComp.origScaleY)
		{
			tankTrans.scale.y = lichComp.origScaleY;
			tankTrans.position.y = lichComp.alertTempYpos;
			lichComp.alertAtTop = false;
			lichComp.alertDone = true;
			ret = BTStatus::Success;
		}
		else
		{
			if(tankTrans.scale.y > 1.0)
			{
				tankTrans.scale.y -= lichComp.alertAnimSpeed * get_dt();
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

    Selector*   avoidPlayerInNoNoZone   = c.c.selector();
    Selector*   regenerateManaIfNeeded  = c.c.selector();
    Random*     pickChoice              = c.d.random();
        

    Condition*  playerInNoNoZone      = c.l.condition("player in NoNo zone", LichBT::playerInNoNoZone);
    Task*       moveAwayFromPlayer    = c.l.task("moving away from player",  LichBT::moveAwayFromPlayer);
    Condition*  noManaToAttack        = c.l.condition("must generte mana",   LichBT::hasMana);
    Task*       regenerateMana        = c.l.task("generating mana",          LichBT::regenerateMana);
    Task*       smartStrategy         = c.l.task("picked strategy smart",    LichBT::pickBestStrategy);
    Task*       randomStrategy        = c.l.task("picket strategy random",   LichBT::pickRandomStrategy);


    root->addDecorator(alwaysTrue);
    root->addCompositor(attackIfManaExists);

    alwaysTrue->addCompositor(avoidPlayerInNoNoZone);
    attackIfManaExists->addCompositor(regenerateManaIfNeeded);
    attackIfManaExists->addDecorator(pickChoice);

    avoidPlayerInNoNoZone->addLeafs({playerInNoNoZone,moveAwayFromPlayer});
    regenerateManaIfNeeded->addLeafs({noManaToAttack,regenerateMana});
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

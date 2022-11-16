#include "TankBTs.hpp"
#include "TankFSM.hpp"

#define getTankComponent() BehaviorTree::sceneHandler->getScene()->getComponent<TankComponent>(entityID)
#define getPlayerID(playerID) std::string playerId_str = "playerID";BehaviorTree::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str)
#define getPlayerTrans(playerID) BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(playerID) 
#define getPlayerCombat(playerID) BehaviorTree::sceneHandler->getScene()->getComponent<Combat>(playerID) 
#define getTankTrans() BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID) 
#define getTheScene() BehaviorTree::sceneHandler->getScene()

void TankBT::rotateTowardsTarget(Entity entityID, float precision)
{
	TankComponent& tankComp = getTankComponent();
	Transform& tankTrans = getTankTrans();
	if(tankComp.firendTarget.id == entityID)
	{
		return;
	}
	//Rotate towards target start
	tankTrans.updateMatrix();
	glm::vec2 targetPos			= glm::vec2(tankComp.firendTarget.pos.x, tankComp.firendTarget.pos.z);
	glm::vec2 tankPos			= glm::vec2(tankTrans.position.x, tankTrans.position.z);
	glm::vec2 curRot			= -glm::normalize(glm::vec2(tankTrans.forward().x, tankTrans.forward().z));
	glm::vec2 tank_to_friend	= glm::normalize(targetPos - tankPos);

	float angle_between			= glm::degrees(glm::acos(glm::dot(tank_to_friend, curRot)));
	tankComp.tempRotAngle = angle_between;

	if(tankComp.rotateLeft && angle_between >= precision)
	{
		tankTrans.rotation.y += tankComp.idleRotSpeed * Time::getDT();
	}
	else if(angle_between >= precision)
	{
		tankTrans.rotation.y -= tankComp.idleRotSpeed * Time::getDT();
	}

	//Check if we rotated in correct direction
	tankTrans.updateMatrix();
	targetPos			= glm::vec2(tankComp.firendTarget.pos.x, tankComp.firendTarget.pos.z);
	tankPos				= glm::vec2(tankTrans.position.x, tankTrans.position.z);
	curRot				= -glm::normalize(glm::vec2(tankTrans.forward().x, tankTrans.forward().z));
	tank_to_friend		= glm::normalize(targetPos - tankPos);
	angle_between		= glm::degrees(glm::acos(glm::dot(tank_to_friend, curRot)));
	//If angle got bigger, then change direction
	if(tankComp.tempRotAngle < angle_between)
	{
		if(tankComp.rotateLeft)
		{
			tankComp.rotateLeft = false;
		}
		else
		{
			tankComp.rotateLeft = true;
		}
	}
	//Rotate towards target end
}
void TankBT::rotateTowards(Entity entityID, glm::vec3 target, float rotSpeed, float precision)
{
	TankComponent& tankComp = getTankComponent();
	Transform& tankTrans = getTankTrans();
	//Rotate towards target start
	tankTrans.updateMatrix();
	glm::vec2 targetPos			= glm::vec2(target.x, target.z);
	glm::vec2 tankPos			= glm::vec2(tankTrans.position.x, tankTrans.position.z);
	glm::vec2 curRot			= -glm::normalize(glm::vec2(tankTrans.forward().x, tankTrans.forward().z));
	glm::vec2 tank_to_friend	= glm::normalize(targetPos - tankPos);

	float angle_between			= glm::degrees(glm::acos(glm::dot(tank_to_friend, curRot)));
	tankComp.tempRotAngle = angle_between;

	if(tankComp.rotateLeft && angle_between >= precision)
	{
		tankTrans.rotation.y += rotSpeed * Time::getDT();
	}
	else if(angle_between >= precision)
	{
		tankTrans.rotation.y -= rotSpeed * Time::getDT();
	}

	//Check if we rotated in correct direction
	tankTrans.updateMatrix();
	targetPos			= glm::vec2(target.x, target.z);
	tankPos				= glm::vec2(tankTrans.position.x, tankTrans.position.z);
	curRot				= -glm::normalize(glm::vec2(tankTrans.forward().x, tankTrans.forward().z));
	tank_to_friend		= glm::normalize(targetPos - tankPos);
	angle_between		= glm::degrees(glm::acos(glm::dot(tank_to_friend, curRot)));
	//If angle got bigger, then change direction
	if(tankComp.tempRotAngle < angle_between)
	{
		if(tankComp.rotateLeft)
		{
			tankComp.rotateLeft = false;
		}
		else
		{
			tankComp.rotateLeft = true;
		}
	}
	//Rotate towards target end
}

void TankBT::registerEntityComponents(Entity entityId)
{
	addRequiredComponent<TankComponent>(entityId);
	addRequiredComponent<AiCombatTank>(entityId);
}


BTStatus TankBT::HasFreindsTarget(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTankComponent();
	if(tankComp.firendTarget.id != -1)
	{
		ret = BTStatus::Success;
	}
	return ret;
}

BTStatus TankBT::AreFriendsAlive(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTankComponent();
	if( tankComp.allFriends.size() > 0)
	{
		ret = BTStatus::Success;
	}
	/*for(auto f: tankComp.allFriends)
	{
		int health = 0;
		if(f.second == "Swarm")
		{
			SwarmGroup* swarmGroup = getTheScene()->getComponent<SwarmComponent>(f.first).group;
			for(auto g: swarmGroup->members)
			{
				health = getTheScene()->getComponent<SwarmComponent>(g).life;
				if(health > 0)
				{
					break;
				}
			}
			
		}
		else if(f.second == "Lich")
		{
			health = getTheScene()->getComponent<LichComponent>(f.first).life;
		}
		if(health > 0)
		{
			ret = BTStatus::Success;
			break;
		}
		
	}*/
	return ret;
}

BTStatus TankBT::PickNewFreinds(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;

	float minDist = 10000.0f;
	int minDistID = -1;
	std::string minDistType = "";
	Transform& tankTrans = getTankTrans();
	//Get all unvisited freinds
	TankComponent& tankComp = getTankComponent();
	for(auto& f: tankComp.allFriends)
	{
		//Pick nearest unvisited friends
		if(f.second.visited){continue;}
		Transform& friendTrans = getTheScene()->getComponent<Transform>(f.first);
		float curDist = glm::length(tankTrans.position - friendTrans.position);
		if(curDist < minDist)
		{
			minDist = curDist;
			minDistID = f.first;
			minDistType = f.second.type;
		}
	}

	if(minDistID != -1)
	{
		ret = BTStatus::Success;
	}
	else
	{
		for(auto& f: tankComp.allFriends)
		{
			f.second.visited = false;
		}
		return ret;
	}

	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
	if(minDistType == "Swarm")
	{
		pos = getTheScene()->getComponent<SwarmComponent>(minDistID).group->idleMidPos;
	}
	else
	{
		pos = getTheScene()->getComponent<Transform>(minDistID).position;
	}
	tankComp.firendTarget = {minDistID, pos};

	return ret;
}

BTStatus TankBT::PickNewRandomTarget(Entity entityID)
{
	BTStatus ret = BTStatus::Success;
	TankComponent& tankComp	= getTankComponent();
	Transform& tankTrans	= getTankTrans();
	tankComp.firendTarget	= {entityID, tankTrans.position};
	return ret;
}

BTStatus TankBT::MoveAround(Entity entityID)
{
	BTStatus ret = BTStatus::Running;

	TankComponent& tankComp = getTankComponent();
	if(tankComp.firendTarget.id == entityID)
	{
		tankComp.firendTarget.id = -1;
		return ret;
	}
	Transform& tankTrans	= getTankTrans();
	glm::vec3 moveDir		= pathFindingManager.getDirTo(tankTrans.position, tankComp.firendTarget.pos);
	moveDir = glm::normalize(moveDir);

	Rigidbody& tankRb		= getTheScene()->getComponent<Rigidbody>(entityID);
	Collider& tankCol		= getTheScene()->getComponent<Collider>(entityID);
	Collider& friendCol		= getTheScene()->getComponent<Collider>(tankComp.firendTarget.id);

	rotateTowardsTarget(entityID, 5.0f);
	

	float dist = glm::length(tankTrans.position - tankComp.firendTarget.pos);
	float distToCheck = 0.0f;
	if(tankComp.allFriends[tankComp.firendTarget.id].type == "Swarm")
	{
		distToCheck = getTheScene()->getComponent<SwarmComponent>(tankComp.firendTarget.id).group->idleRadius + (tankCol.radius*2) + (friendCol.radius*2) + 2.0f;
	}
	else
	{
		distToCheck = tankComp.friendVisitRadius;
	}
	
	if(dist <= distToCheck)
	{
		tankComp.allFriends[tankComp.firendTarget.id].visited = true;
		tankComp.firendTarget.id = -1;
		
		ret = BTStatus::Success;
	}
	else
	{
		tankRb.velocity = moveDir * tankComp.idleSpeed;
	}

	return ret;
}

BTStatus TankBT::playerInPersonalSpace(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;

	TankComponent& tankComp = getTankComponent();
    int playerID			= -1;
    getPlayerID(playerID);
    Transform& playerTrans  = getPlayerTrans(playerID);
    Transform& tankTrans    = getTankTrans();
    float tank_player_dist	= glm::length(playerTrans.position - tankTrans.position);
    if(tank_player_dist <= tankComp.peronalSpaceRadius)
    {
        ret = BTStatus::Success;
    }

	return ret;
}

BTStatus TankBT::GroundHump(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::playerOutsidePersonalSpace(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTankComponent();
    int playerID			= -1;
    getPlayerID(playerID);
    Transform& playerTrans  = getPlayerTrans(playerID);
    Transform& tankTrans    = getTankTrans();
    float tank_player_dist	= glm::length(playerTrans.position - tankTrans.position);
    if(tank_player_dist > tankComp.peronalSpaceRadius)
    {
        ret = BTStatus::Success;
    }
	return ret;
}

BTStatus TankBT::ChargeAndRun(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::getNearestGroupToPlayer(Entity entityID)
{
	BTStatus ret = BTStatus::Success;

	glm::vec3 average		= glm::vec3(0.0f, 0.0f, 0.0f);
	int num					= 0;
	TankComponent& tankComp = getTankComponent();
	for(auto f: tankComp.friendsInSight)
	{
		if(f.second.type == "Swarm")
		{
			SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(f.first);
			for(auto g: swarmComp.group->members)
			{
				if(getTheScene()->getComponent<SwarmComponent>(g).group->inCombat)
				{
					average += getTheScene()->getComponent<Transform>(g).position;
					num++;
				}
			}
		}
		else if(f.second.type == "Lich")
		{
			LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(f.first);
			if(lichComp.inCombat)
			{
				average += getTheScene()->getComponent<Transform>(f.first).position;
				num++;
			}
		}
	}
	if(num == 0)
	{
		average = getTankTrans().position;
	}
	else
	{
		average /= num;
	}
	tankComp.shieldTargetPos = average;


	return ret;
}

BTStatus TankBT::groupInPersonalSpece(Entity entityID)
{
	BTStatus ret			= BTStatus::Failure;
	TankComponent& tankComp = getTankComponent();
	Transform& tankTrans	= getTankTrans();
	float len				= glm::length(tankComp.shieldTargetPos - tankTrans.position);
	if(len <= tankComp.peronalSpaceRadius)
	{
		ret = BTStatus::Success;
	}

	return ret;
}

BTStatus TankBT::moveTowardsGroup(Entity entityID)
{
	BTStatus ret			= BTStatus::Success;
	TankComponent& tankComp = getTankComponent();
	Transform& tankTrans	= getTankTrans();
	Rigidbody& tankRb		= getTheScene()->getComponent<Rigidbody>(entityID);
	glm::vec3 moveDir		= pathFindingManager.getDirTo(tankTrans.position, tankComp.shieldTargetPos);
	moveDir					= glm::normalize(moveDir);
	tankRb.velocity			= moveDir * tankComp.shieldSpeed;
	return ret;
}

BTStatus TankBT::HoldShield(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTankComponent();
	if(tankComp.friendHealTimer <= 0)
	{
		tankComp.friendHealTimer = tankComp.friendHealTimerOrig;
		for(auto& f: tankComp.friendsInSight)
		{
		    if(f.second.type == "Swarm")
		    {
				SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(f.first);
		        swarmComp.shieldedByTank = true;
				
				int toAdd = tankComp.friendHealthRegen;
				if((swarmComp.life + toAdd) > swarmComp.FULL_HEALTH)
				{
					swarmComp.life = swarmComp.FULL_HEALTH;
				}
				else
				{
					swarmComp.life += (int)toAdd;
				}
		    }
		    else if(f.second.type == "Lich")
		    {
				LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(f.first);
		        lichComp.shieldedByTank = true;
				float toAdd = tankComp.friendHealthRegen;
				if((lichComp.life + toAdd) > lichComp.FULL_HEALTH)
				{
					lichComp.life = lichComp.FULL_HEALTH;
				}
				else
				{
					lichComp.life += toAdd;
				}

		    }
		}

	}
	else
	{
		tankComp.friendHealTimer -= Time::getDT();
	}
	

	int playerID = -1;
	getPlayerID(playerID);
	Transform& playerTrans = getPlayerTrans(playerID);
	rotateTowards(entityID, playerTrans.position, tankComp.shildRotSpeed, 5.0f);

	return ret;
}

BTStatus TankBT::playAlertAnim(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::playDeathAnim(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::die(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}


void Tank_idle::start()
{
	// new row
    Sequence*	root					= c.c.sequence();
    // new row
	Selector*   selectMovePos			= c.c.selector();
	Task*		move					= c.l.task("Moving", TankBT::MoveAround);
	// new row
	Selector*   havePosOrGetNew			= c.c.selector();
	Task*		pickRandomPosition		= c.l.task("Pick random position", TankBT::PickNewRandomTarget);
	// new row
	Condition*	hasFriendTarget			= c.l.condition("Has friend target", TankBT::HasFreindsTarget);
	Sequence*	getNewTargetIfFriends	= c.c.sequence();
	// new row
	Condition*	anyFriendsAlive			= c.l.condition("Any firends alive?", TankBT::AreFriendsAlive);
	Task*		pickUnvisitedFriend		= c.l.task("Pick unvisited friend", TankBT::PickNewFreinds);

    // Relations 
	root->addCompositor				(selectMovePos);
	root->addLeaf					(move);
	// new row
	selectMovePos->addCompositor	(havePosOrGetNew);
	selectMovePos->addLeaf			(pickRandomPosition);
	// new row
	havePosOrGetNew->addLeaf		(hasFriendTarget);
	havePosOrGetNew->addCompositor	(getNewTargetIfFriends);
	// new row
	getNewTargetIfFriends->addLeafs	({anyFriendsAlive, pickUnvisitedFriend});

	//Set root
    this->setRoot(root);
}

void Tank_alert::start()
{
	Task* theAlert = c.l.task("Alert!", TankBT::playAlertAnim);
	this->setRoot(theAlert);
}

void Tank_combat::start()
{
	// new row
    Selector*	root					= c.c.selector();
    // new row
	Sequence*   personalSequence		= c.c.sequence();
	Sequence*   stinkySequence			= c.c.sequence();
	// new row
	Condition*  playerIsInPersonalSpace	= c.l.condition("Player in personal space", TankBT::playerInPersonalSpace);
	Task*		DoTheGroundHump			= c.l.task("Tank is humping the ground", TankBT::GroundHump);
	Condition*	playerIsInSight			= c.l.condition("Player is in sight!", TankBT::playerOutsidePersonalSpace);
	Task*		doTheChargeAndRun		= c.l.task("Charge and rum!", TankBT::ChargeAndRun);

    // Relations 
	root->addCompositors				({personalSequence, stinkySequence});
	// new row
	personalSequence->addLeafs			({playerIsInPersonalSpace, DoTheGroundHump});
	// new row
	stinkySequence->addLeafs			({playerIsInSight, doTheChargeAndRun});

	//Set root
    this->setRoot(root);
}

void Tank_shield::start()
{
	// new row
    Sequence*	root					= c.c.sequence();	
    // new row
	Sequence*   groupSeq				= c.c.sequence();
	Task*		holdShield				= c.l.task("Hodl shield", TankBT::HoldShield);
	// new row
	Task*		getGroupNearestPlayer	= c.l.task("Get group nearset player", TankBT::getNearestGroupToPlayer);
	Selector*	groupMoveSelecotr		= c.c.selector();
	// new row
	Condition*  groupInPersonalSpace	= c.l.condition("Group in personal space", TankBT::groupInPersonalSpece);
	Task*		moveTowardsGroup		= c.l.task("Moving towards group", TankBT::moveTowardsGroup);

    // Relations 
	root->addCompositor					(groupSeq);
	root->addLeaf						(holdShield);
	// new row
	groupSeq->addLeaf					(getGroupNearestPlayer);
	groupSeq->addCompositor				(groupMoveSelecotr);
	// new row
	groupMoveSelecotr->addLeafs			({groupInPersonalSpace, moveTowardsGroup});

	//Set root
    this->setRoot(root);
}

void Tank_dead::start()
{
	Sequence*	root					= c.c.sequence();
	// new row
	Task*		deathAnim				= c.l.task("playing death anim", TankBT::playDeathAnim);
	Task*		death					= c.l.task("Dead, oof!", TankBT::die);

	root->addLeafs({deathAnim, death});

	this->setRoot(root);
}

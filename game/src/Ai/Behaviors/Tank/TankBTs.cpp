#include "TankBTs.hpp"
#include "TankFSM.hpp"

#define getTankComponent() BehaviorTree::sceneHandler->getScene()->getComponent<TankComponent>(entityID)
#define getPlayerID(playerID) std::string playerId_str = "playerID";BehaviorTree::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str)
#define getPlayerTrans(playerID) BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(playerID) 
#define getPlayerCombat(playerID) BehaviorTree::sceneHandler->getScene()->getComponent<Combat>(playerID) 
#define getTankTrans() BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID) 
#define getTheScene() BehaviorTree::sceneHandler->getScene()

void TankBT::registerEntityComponents(Entity entityId)
{
	addRequiredComponent<TankComponent>(entityId);
	addRequiredComponent<AiCombatTank>(entityId);
}


BTStatus TankBT::HasFreindsTarget(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTankComponent();
	if(tankComp.firendTarget != -1)
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
	return ret;
}

BTStatus TankBT::PickNewRandomTarget(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::MoveAround(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::playerInPersonalSpace(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
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
	return ret;
}

BTStatus TankBT::ChargeAndRun(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::getNearestGroupToPlayer(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::groupInPersonalSpece(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::moveTowardsGroup(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	return ret;
}

BTStatus TankBT::HoldShield(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
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

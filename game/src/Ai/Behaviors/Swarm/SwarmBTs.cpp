#include "SwarmBTs.hpp"
#include "SwarmFSM.hpp"
//#include "../components/Script.hpp"

//TODO: Remove
uint32_t getPlayerID_DUMMY(SceneHandler* sceneHandler,int playerID_in) 
{
	static bool notSet = true;
	static int playerID = 0; 
	if (notSet && playerID_in != -1)
	{
		playerID = playerID_in;
		notSet = false; 
	}


    std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);

	return playerID; 
}


float lookAtY(Transform from, Transform to)
{
    float posX = from.position.x - to.position.x;
    float posZ = from.position.z - to.position.z;
    float angle = atan2(posX, posZ);
    angle = glm::degrees(angle);
  
    return angle; 
}
//SwarmGroup* SwarmComponentBT::group = new SwarmGroup;

BTStatus SwarmBT::hasFriends(uint32_t entityID)
{

	BTStatus ret = BTStatus::Success;

	SwarmGroup* groupPtr =
	    sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID).group;
		if (groupPtr->members.size() <= 1)
	{
		return BTStatus::Failure;
	}
	return ret;
}
BTStatus SwarmBT::jumpInCircle(uint32_t entityID)
{

	BTStatus ret = BTStatus::Running;
	//TODO: Make blob jump in circle!

	if (hasFriends(entityID) == BTStatus::Failure)
	{
		return BTStatus::Failure;

	}
	return ret;
}
BTStatus SwarmBT::lookingForGroup(uint32_t entityID)
{

	BTStatus ret = BTStatus::Running;
	//TODO: Make blob jump around and look for friends!

	if (hasFriends(entityID) == BTStatus::Failure)
	{
		return BTStatus::Failure;
	}
	return ret;
}
BTStatus SwarmBT::JoinGroup(uint32_t entityID)
{

	BTStatus ret = BTStatus::Running;
	//TODO: Make blob jump in circle!

	SwarmComponentBT& thisSwarmComp =
	    BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
	Transform& thisTransform =
	    BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	sceneHandler->getScene()->getSceneReg().view<SwarmComponentBT, Transform>().each(
	    [&](const auto& entity, SwarmComponentBT& swComp, Transform& trans)
	    {
		    if (static_cast<int>(entity) != entityID)
		    {
			    //SwarmComponentFSM& curSwarmComp = BehaviorTree::scene->getComponent<SwarmComponentFSM>(static_cast<int>(entity));
			    if ((thisTransform.position - trans.position).length() <=
			        thisSwarmComp.sightRadius)
			    {
				    //TODO: Join group

				    return BTStatus::Success;
			    }
		    }
	    }
	);
	return ret;
}

BTStatus SwarmBT::seesNewFriends(uint32_t entityID)
{
	BTStatus ret = BTStatus::Failure;

	SwarmComponentBT& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);

	//TODO perf: Define size of vecor from start to avoid push_back
	thisSwarmComp.groupsInSight.clear();

	sceneHandler->getScene()->getSceneReg().view<SwarmComponentBT, Transform>().each(
	    [&](const auto& entity, SwarmComponentBT& swComp, Transform& trans)
	    {
		    if (static_cast<int>(entity) != entityID)
		    {
				//TODO: use cone(frustum) to detect friends
			    if ((thisTransform.position - trans.position).length() <= thisSwarmComp.sightRadius)
			    {
					//TODO: store all visable friends
				    thisSwarmComp.groupsInSight.push_back(swComp.group);
				    return BTStatus::Success;
			    }
		    }
	    }
	);

	return ret;
}
BTStatus SwarmBT::escapeToFriends(uint32_t entityID)
{
	BTStatus ret = BTStatus::Running;
	
	SwarmComponentBT& thisSwarmComp =
	    BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);

	SwarmGroup* nearestGroup = nullptr;
	for (auto& f : thisSwarmComp.groupsInSight)
	{
		nearestGroup = f;
		break;
		//TODO: find the nearest friend group
	}

	//if nearest group is nullptr, then it is empty.
	if (nearestGroup == nullptr)
	{
		return BTStatus::Failure;
	}

	Transform& thisTransform =
	    BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);

	//TODO: find nearset blob instead.
	glm::vec3 swarmMidPoint;
	for (auto& blob : nearestGroup->members)
	{
		swarmMidPoint += sceneHandler->getScene()->getComponent<Transform>(blob).position;
	}
	swarmMidPoint /= nearestGroup->members.size();
	
	 if ((thisTransform.position - swarmMidPoint).length() <=
	    thisSwarmComp.sightRadius)
	{
		//TODO: Join group

		return BTStatus::Success;
	}

	//Move towards player

	return ret;
}
BTStatus SwarmBT::escapeFromPlayer(uint32_t entityID)
{
	BTStatus ret = BTStatus::Running;
	//TODO: change to real player ID
	int player = getPlayerID_DUMMY(sceneHandler);
	

	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponentBT& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
	Transform playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(player);
	SwarmComponentFSM& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentFSM>(entityID);

	if (glm::length((thisTransform.position - playerTransform.position)) > thisSwarmComp.sightRadius)
	{
		return BTStatus::Success;
	}

	 thisTransform.rotation.y = lookAtY(playerTransform, thisTransform);
	 thisTransform.updateMatrix();

	 glm::vec3 dir = -glm::normalize(playerTransform.position- thisTransform.position);
	 dir.y = 0;
	 thisTransform.position += dir * Time::getDT() * swarmComp.speed;

	//TODO BTStatus: failure not returned.
	//TODO : Check if cornered, return failure

	return ret;
}

BTStatus SwarmBT::informFriends(uint32_t entityID)
{
	BTStatus ret = BTStatus::Success;

	SwarmComponentBT& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
	for (auto& f : thisSwarmComp.group->members)
	{
		SwarmComponentBT& curSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentBT>(f);
		curSwarmComp.inCombat = true;
	}

	//TODO BTStatus: failure not returned.

	return ret;
}
BTStatus SwarmBT::jumpTowardsPlayer(uint32_t entityID)
{
	BTStatus ret = BTStatus::Running;

	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID_DUMMY(sceneHandler));
	SwarmComponentFSM& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentFSM>(entityID);

     thisTransform.rotation.y = lookAtY(thisTransform, playerTransform);
	 thisTransform.updateMatrix();

	 glm::vec3 dir = glm::normalize(playerTransform.position- thisTransform.position);
	 dir.y = 0;
     thisTransform.position += dir * Time::getDT() * swarmComp.speed;

    

	//TODO: Something may be wrong here??!?
	if (closeEnoughToPlayer(entityID) == BTStatus::Success)
	{
		return BTStatus::Success;
	}

	return ret;
}
BTStatus SwarmBT::closeEnoughToPlayer(uint32_t entityID)
{
	BTStatus ret = BTStatus::Failure;

	SwarmComponentBT& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentBT>(entityID);
	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID_DUMMY(sceneHandler));

	float dist = glm::length(thisTransform.position - playerTransform.position);
	if (dist <= thisSwarmComp.attackRange)
	{
		return BTStatus::Success;
	}

	return ret;
}



BTStatus SwarmBT::attack(uint32_t entityID)
{
	BTStatus ret = BTStatus::Running;

	//SwarmComponentBT& thisSwarmComp = BehaviorTree::scene->getComponent<SwarmComponentBT>(entityID);
	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID_DUMMY(sceneHandler));
	SwarmComponentFSM& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponentFSM>(entityID);

     thisTransform.rotation.y = lookAtY(thisTransform, playerTransform);
	 thisTransform.updateMatrix();

	 glm::vec3 dir = glm::normalize(playerTransform.position- thisTransform.position);
	 dir.y = 0;
     thisTransform.position += dir * Time::getDT() * swarmComp.speed;

	return ret;
}



void Swarm_idle::start() {

    this->addRequiredComponent(new SwarmComponentBT);

	Selector* root = c.c.selector();
    
    // Compositors
	Sequence*   socializing     = c.c.sequence();
	Sequence*   groupExploring  = c.c.sequence();

    // Leafs
	Condition*  has_friends     = c.l.condition("Has firends", SwarmBT::hasFriends);
	Task*       jump_in_circle  = c.l.task("Jump in circle", SwarmBT::jumpInCircle);
	Task*       lfg             = c.l.task("Looking for group", SwarmBT::lookingForGroup);
	Task*       join_group      = c.l.task("Join group", SwarmBT::JoinGroup);
	
    // Relations 
	root->addCompositors({socializing, groupExploring});
	socializing->addLeafs({has_friends, jump_in_circle});
	groupExploring->addLeafs({lfg, join_group});

    this->setRoot(root);
}

void Swarm_combat::start() 
{
	 this->addRequiredComponent(new SwarmComponentBT);

	Parallel* root = create.compositor.parallel();

	Sequence* decide_to_snitch = c.c.sequence();
	Selector* initiate_attack = c.c.selector();

	Condition* has_firends = c.l.condition("Has freinds", SwarmBT::hasFriends);
	Task* inform_friends = c.l.task("Inform freinds", SwarmBT::informFriends);

	Sequence* attack_if_close_enough = c.c.sequence();
	Task* attack = c.l.task("Attack", SwarmBT::attack);

	Decorator* not_close_enough_to_player = c.d.invert();
	Condition* close_enough_to_player = c.l.condition("Close enough to player", SwarmBT::closeEnoughToPlayer);
	Task* jump_towards_player = c.l.task("Jump towards player", SwarmBT::jumpTowardsPlayer);


	
	root->addCompositors({decide_to_snitch, initiate_attack});
	decide_to_snitch->addLeafs({has_firends, inform_friends});
	initiate_attack->addCompositor(attack_if_close_enough);
	initiate_attack->addLeaf(attack);
	attack_if_close_enough->addDecorator(not_close_enough_to_player);
	attack_if_close_enough->addLeaf(jump_towards_player);
	not_close_enough_to_player->addLeaf(close_enough_to_player);

	this->setRoot(root);
}

void Swarm_escape::start() 
{
	this->addRequiredComponent(new SwarmComponentBT);

	Selector* root = c.c.selector();

	Sequence* escape_to_friends_if_possible = c.c.sequence();
	Task* escape_from_player = c.l.task("Escape from plater", SwarmBT::escapeFromPlayer);

	Condition* sees_friends = c.l.condition("Sees firends", SwarmBT::seesNewFriends);
	Task* escape_to_friends = c.l.task("Escape to friends", SwarmBT::escapeToFriends);

	// Relations
	root->addCompositors({escape_to_friends_if_possible});
	root->addLeaf(escape_from_player);


	escape_to_friends_if_possible->addLeafs({sees_friends, escape_to_friends});

	this->setRoot(root);
}

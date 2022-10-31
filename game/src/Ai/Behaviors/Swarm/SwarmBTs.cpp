#include "SwarmBTs.hpp"
#include "SwarmFSM.hpp"
#include "../../../Components/Combat.h"
#include "../../../Components/AiCombat.h"

int SwarmGroup::getNewId = 0;

Entity getPlayerID(SceneHandler* sceneHandler) 
{
    int playerID = 0;

    std::string playerString = "playerID";
    sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);

	return playerID; 
}


float lookAtY(const Transform& from, const Transform& to)
{
    float posX = from.position.x - to.position.x;
    float posZ = from.position.z - to.position.z;
    float angle = atan2(posX, posZ);
    angle = glm::degrees(angle);
  
    return angle; 
}
void removeFromGroup(SwarmComponent& comp, Entity entityID)
{
	for(int i = 0; i < comp.group->members.size(); i++)
	{
		if(comp.group->members[i] == entityID)
		{
			comp.group->members.erase(comp.group->members.begin()+i);
		}
	}
}

void SwarmBT::registerEntityComponents(Entity entityId)
{
  this->addRequiredComponent<SwarmComponent>(entityId);
}

BTStatus SwarmBT::hasFriends(Entity entityID)
{

	BTStatus ret = BTStatus::Success;

	SwarmGroup* groupPtr =
	    BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID).group;
	if (groupPtr->members.size() <= 1)
	{
		return BTStatus::Failure;
	}
	return ret;
}
BTStatus SwarmBT::jumpInCircle(Entity entityID)
{

	BTStatus ret = BTStatus::Running;
	//TODO: Make blob jump in circle!

	//Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	//SwarmComponent& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	//glm::vec3 dir = glm::normalize(thisSwarmComp.group->idleMidBos - thisTransform.position);
	//thisTransform.position += Time::getDT() * dir * thisSwarmComp.speed;



	if (hasFriends(entityID) == BTStatus::Failure)
	{
		return BTStatus::Failure;

	}
	return ret;
}
BTStatus SwarmBT::lookingForGroup(Entity entityID)
{

	BTStatus ret = BTStatus::Running;
	//TODO: Make blob jump around and look for friends!

	if (hasFriends(entityID) == BTStatus::Failure)
	{
		return BTStatus::Failure;
	}
	return ret;
}
BTStatus SwarmBT::JoinGroup(Entity entityID)
{

	BTStatus ret = BTStatus::Running;
	//TODO: Make blob jump in circle!

	SwarmComponent& thisSwarmComp =
	    BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);

	Transform& thisTransform =
	    BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);

    BehaviorTree::sceneHandler->getScene()->getSceneReg().view<SwarmComponent, Transform>(entt::exclude<Inactive>).each(
	    [&](const auto& entity, SwarmComponent& swComp, Transform& trans)
	    {
		    if (static_cast<int>(entity) != entityID)
		    {
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

BTStatus SwarmBT::seesNewFriends(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;

	SwarmComponent& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Transform& thisTransform      = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);

	//TODO perf: Define size of vecor from start to avoid push_back
	thisSwarmComp.groupsInSight.clear();

	sceneHandler->getScene()->getSceneReg().view<SwarmComponent, Transform>(entt::exclude<Inactive>).each(
	    [&](const auto& entity, SwarmComponent& swComp, Transform& trans)
	    {
		    if (static_cast<int>(entity) != entityID)
		    {
				//TODO: use cone(frustum) to detect friends
			    if ((thisTransform.position - trans.position).length() <= thisSwarmComp.sightRadius)
			    {
					//TODO: store all visable friends
				    thisSwarmComp.groupsInSight.push_back(swComp.group);				    
			    }
		    }
	    }
	);

	return ret;
}
BTStatus SwarmBT::escapeToFriends(Entity entityID)
{
	BTStatus ret = BTStatus::Running;
	
	SwarmComponent& thisSwarmComp =
	    BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);

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
		swarmMidPoint += BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(blob).position;
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
BTStatus SwarmBT::escapeFromPlayer(Entity entityID)
{
	BTStatus ret = BTStatus::Running;
	//TODO: change to real player ID
	int player = getPlayerID(sceneHandler);
	

	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Transform playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(player);
	SwarmComponent& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& rigidbody = BehaviorTree::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);

	if (glm::length((thisTransform.position - playerTransform.position)) > thisSwarmComp.sightRadius)
	{
		return BTStatus::Success;
	}

	 thisTransform.rotation.y = lookAtY(playerTransform, thisTransform);
	 thisTransform.updateMatrix();

	 glm::vec3 dir = -glm::normalize(playerTransform.position- thisTransform.position);
	 dir.y = 0;
	 rigidbody.velocity = dir * swarmComp.speed;

	//TODO BTStatus: failure not returned.
	//TODO : Check if cornered, return failure

	return ret;
}

BTStatus SwarmBT::informFriends(Entity entityID)
{
	BTStatus ret = BTStatus::Success;

	SwarmComponent& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	for (auto& f : thisSwarmComp.group->members)
	{
		SwarmComponent& curSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(f);
		curSwarmComp.group->inCombat = true;
	}

	//TODO BTStatus: failure not returned.

	return ret;
}
BTStatus SwarmBT::jumpTowardsPlayer(Entity entityID)
{
	BTStatus ret = BTStatus::Running;

	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID(sceneHandler));
	SwarmComponent& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& rigidbody = BehaviorTree::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
    thisTransform.rotation.y = lookAtY(thisTransform, playerTransform);
	thisTransform.updateMatrix();

	 glm::vec3 dir = glm::normalize(playerTransform.position- thisTransform.position);
	 dir.y = 0;
	 rigidbody.velocity = dir * swarmComp.speed;


    

	//TODO: Something may be wrong here??!?
	if (closeEnoughToPlayer(entityID) == BTStatus::Success)
	{
		return BTStatus::Success;
	}

	return ret;
}
BTStatus SwarmBT::closeEnoughToPlayer(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;

	SwarmComponent& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID(sceneHandler));

	float dist = glm::length(thisTransform.position - playerTransform.position);
	if (dist <= thisSwarmComp.attackRange)
	{
		return BTStatus::Success;
	}

	return ret;
}
BTStatus SwarmBT::attack(Entity entityID)
{
	BTStatus ret = BTStatus::Running;

	//SwarmComponent& thisSwarmComp = BehaviorTree::scene->getComponent<SwarmComponent>(entityID);
	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID(sceneHandler));
	SwarmComponent& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	AiCombat& combat = BehaviorTree::sceneHandler->getScene()->getComponent<AiCombat>(entityID);

     thisTransform.rotation.y = lookAtY(thisTransform, playerTransform);
	 thisTransform.updateMatrix();

	 if(combat.timer > 0.0f){
		combat.timer -= Time::getDT();
	 }
	 else
	 {
		Combat& playerCombat = BehaviorTree::sceneHandler->getScene()->getComponent<Combat>(getPlayerID(sceneHandler));
		playerCombat.health -= combat.lightHit;
		combat.timer = combat.lightAttackTime;
		std::cout<<"ATTACK!!!!\n";
		ret = BTStatus::Success;
	 
	 }

	return ret;
}

BTStatus SwarmBT::playDeathAnim(Entity entityID)
{
	BTStatus ret = BTStatus::Running;
	SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Transform& swarmTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
	if(swarmTrans.scale.y <= 0.0f)
	{
		ret = BTStatus::Success;
	}
	else
	{
		swarmTrans.rotation.y +=  1000*swarmComp.deathAnimSpeed*Time::getDT();
		swarmTrans.scale.y -= swarmComp.deathAnimSpeed*Time::getDT();
	}

	return ret;
}
BTStatus SwarmBT::die(Entity entityID)
{
	BTStatus ret = BTStatus::Success;

	//TODO: Sometgin goes wrong when we remove from group.
	//SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);

	//removeFromGroup(swarmComp, entityID);
	sceneHandler->getScene()->setInactive(entityID);

	return ret;
}

BTStatus SwarmBT::alerted(Entity entityID)
{
	BTStatus ret = BTStatus::Running;
	SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Transform& swarmTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);

	// if(swarmTrans.scale.y <= 1.0f)
	// {
	// 	ret = BTStatus::Success;
	// }
	// else
    static float speed = 10;        
    if(swarmComp.alert_go_up && swarmTrans.scale.y <= 1.5f)
	{        
		swarmTrans.scale.y += speed*Time::getDT();        
	}
    else if(swarmTrans.scale.y >= 1.f)
    {
        swarmComp.alert_go_up = false; 
        swarmTrans.scale.y -= speed*Time::getDT();
    }
    else 
    {
        swarmComp.alert_go_up = true; 
        BTStatus ret = BTStatus::Success;
    }

	return ret;
}

void Swarm_idle::start() {


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

	Parallel* root = create.compositor.parallel();

	Sequence* decide_to_snitch = c.c.sequence();
	Selector* initiate_attack = c.c.selector();
    Task* alertedOnPlayerDiscover = c.l.task("Alerted", SwarmBT::alerted);        

	Condition* has_firends = c.l.condition("Has freinds", SwarmBT::hasFriends);
	Task* inform_friends = c.l.task("Inform freinds", SwarmBT::informFriends);
    	

	Sequence* attack_if_close_enough = c.c.sequence();
	Task* attack = c.l.task("Attack", SwarmBT::attack);

	Decorator* not_close_enough_to_player = c.d.invert();
	Condition* close_enough_to_player = c.l.condition("Close enough to player", SwarmBT::closeEnoughToPlayer);
	Task* jump_towards_player = c.l.task("Jump towards player", SwarmBT::jumpTowardsPlayer);


	
    root->addLeaf(alertedOnPlayerDiscover);
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

void Swarm_dead::start()
{
	Sequence* root = c.c.sequence();

	Task* playDeathAnimTask = c.l.task("Play death animation", SwarmBT::playDeathAnim);
	Task* dieTask = c.l.task("die", SwarmBT::die);

	root->addLeafs({playDeathAnimTask, dieTask});

	this->setRoot(root);

}


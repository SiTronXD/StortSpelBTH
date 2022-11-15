#include "SwarmBTs.hpp"
#include "SwarmFSM.hpp"
#include "../../../Components/Combat.h"
#include "../../../Components/AiCombat.h"
#include "../../../Components/Perks.h"
#include "../../../Components/Abilities.h"
#include <limits>

int SwarmGroup::getNewId = 0;
int SwarmBT::perkMeshes[] = { 0, 0, 0 };
int SwarmBT::abilityMeshes[] = { 0, 0 };

Entity getPlayerID(SceneHandler* sceneHandler) 
{
  int playerID = -1;

  std::string playerString = "playerID";
  NetworkScene* s = dynamic_cast<NetworkScene*>(sceneHandler->getScene());
  if (s != nullptr)
    {
      //now it only goes to host make so it goes to any one
      return s->getPlayer(0);
    }
  sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
  return playerID;
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
  std::cout << "has friends" << std::endl;
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
  std::cout << "jumpInCircle" << std::endl;
	BTStatus ret = BTStatus::Running;
	if (hasFriends(entityID) == BTStatus::Failure)
	{
		return BTStatus::Failure;
	}


	Transform& swarmTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	SwarmComponent& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& swarmRB = BehaviorTree::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
	Collider& swarmCol = BehaviorTree::sceneHandler->getScene()->getComponent<Collider>(entityID);

	if( swarmComp.group->aliveMembers.size() != 1)
	{
		float len = glm::length(swarmComp.idleMoveTo - swarmTransform.position);
		glm::vec3 dir = glm::normalize(swarmComp.idleMoveTo - swarmTransform.position);
 		swarmRB.velocity = dir * swarmComp.idleSpeed;

		if(swarmComp.touchedFriend)
		{
			//Set move to
			swarmComp.idleMoveTo = swarmComp.group->idleMidBos;
			glm::vec3 dir = glm::normalize(glm::vec3(rand() * (rand() % 2 == 0 ? - 1 : 1), 0.0f, rand() * (rand() % 2 == 0 ? - 1 : 1)));
			swarmComp.idleMoveTo = swarmComp.group->idleMidBos + dir * swarmComp.group->idleRadius;
		}
		else if(swarmCol.radius*2 > len)
		{
			//Set move to
			swarmComp.idleMoveTo = swarmComp.group->idleMidBos;
			glm::vec3 dir = glm::normalize(glm::vec3(rand() * (rand() % 2 == 0 ? - 1 : 1), 0.0f, rand() * (rand() % 2 == 0 ? - 1 : 1)));
			swarmComp.idleMoveTo = swarmComp.group->idleMidBos + dir * swarmComp.group->idleRadius;
		}
	}
	else
	{
		// If 
		if(swarmComp.lonelyTimer > swarmComp.lonelyTime)
		{	
			swarmComp.lonelyTimer = 0.f;
			swarmComp.lonelyDir = glm::normalize(glm::vec3(rand() * (rand() % 2 == 0 ? - 1 : 1), 0.0f, rand() * (rand() % 2 == 0 ? - 1 : 1)));	
			float a = 4.1f;
			swarmComp.idleMoveTo = swarmTransform.position + swarmComp.lonelyDir * std::numeric_limits<float>().max(); 
		}
		else
		{
			swarmComp.lonelyTimer += Time::getDT();
		}
		swarmRB.velocity = swarmComp.lonelyDir * swarmComp.idleSpeed;

	}

	swarmTransform.rotation.y = lookAtY(swarmTransform.position, swarmComp.idleMoveTo);
	swarmTransform.updateMatrix();

	return ret;
}
BTStatus SwarmBT::lookingForGroup(Entity entityID)
{
  std::cout << "lookingForGroup" << std::endl;
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
  std::cout << "joinGroup" << std::endl;
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
  std::cout << "seeNewFriend" << std::endl;
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
  std::cout << "EscapeToFriends" << std::endl;
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
  std::cout << "EscapeFromPlayer" << std::endl;
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
	 float tempYvel =  rigidbody.velocity.y;
	 rigidbody.velocity = dir * swarmComp.speed;
	 rigidbody.velocity.y = tempYvel;

	 if(rand()%2==0)
	 {
		 thisTransform.rotation.y += 100 * thisSwarmComp.escapeAnimSpeed * Time::getDT();
	 }
	 else
	 {
		 thisTransform.rotation.y -= 100 * thisSwarmComp.escapeAnimSpeed * Time::getDT();
	 }

	//TODO BTStatus: failure not returned.
	//TODO : Check if cornered, return failure

	return ret;
}

BTStatus SwarmBT::informFriends(Entity entityID)
{
  std::cout << "informFriends" << std::endl;
	BTStatus ret = BTStatus::Success;

	SwarmComponent& thisSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(thisSwarmComp.getGroupHealth(BehaviorTree::sceneHandler->getScene()) > thisSwarmComp.LOW_HEALTH)
	{
		for (auto& f : thisSwarmComp.group->members)
		{
			SwarmComponent& curSwarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(f);
			curSwarmComp.group->inCombat = true;
		}
	}
	else
	{
		ret = BTStatus::Failure;
	}

	return ret;
}
BTStatus SwarmBT::jumpTowardsPlayer(Entity entityID)
{
  std::cout << "jump to player" << std::endl;
	BTStatus ret = BTStatus::Running;

	Collider& entityCollider = BehaviorTree::sceneHandler->getScene()->getComponent<Collider>(entityID);
	Collider& playerCollider = BehaviorTree::sceneHandler->getScene()->getComponent<Collider>(getPlayerID(sceneHandler));
	Transform& entityTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID(sceneHandler));
	SwarmComponent& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& rigidbody = BehaviorTree::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
    

    entityTransform.rotation.y = lookAtY(entityTransform, playerTransform);
	entityTransform.updateMatrix();    

	glm::vec3 newPlayerPos = playerTransform.position;
	newPlayerPos.y+=3.5f;
	glm::vec3 playerDown = -playerTransform.up();
	float playerLen = playerCollider.height;
	float swarmLen = entityCollider.radius;
	float newPosOffset = playerLen - swarmLen;
	newPlayerPos = newPlayerPos + playerDown * newPosOffset;
	//Temp code end------------------------------------------
    
    glm::vec3 dirEntityToPlayer = glm::normalize(newPlayerPos - entityTransform.position);
    glm::vec3 entityTargetPos = entityTransform.position;
    float distEntityTargetPosToPlayer = glm::length(newPlayerPos - entityTargetPos);
    glm::vec3 dirEntityTargetPosToPlayer = glm::normalize(newPlayerPos - entityTargetPos);
	int safetyBreak = 0;
    bool canSeePlayer = false; 
	//TODO: saftybeak is bad stuff, fix this shit
    while(!canSeePlayer) 
    {
        distEntityTargetPosToPlayer = glm::length(newPlayerPos - entityTargetPos) ;
        dirEntityTargetPosToPlayer = glm::normalize(newPlayerPos - entityTargetPos);
        Ray rayToPlayer{entityTargetPos, dirEntityTargetPosToPlayer};    
        RayPayload rp = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayToPlayer, distEntityTargetPosToPlayer+10.f);

        if (rp.hit)
        {        
            int playerId = getPlayerID(sceneHandler);

            if(playerId != rp.entity)
            {
                glm::vec3 leftOfPlayer = glm::cross(dirEntityTargetPosToPlayer, glm::vec3(0.f,1.f,0.f));
                dirEntityToPlayer = glm::normalize(leftOfPlayer + dirEntityTargetPosToPlayer); 
                entityTargetPos += dirEntityToPlayer;

            }

            canSeePlayer = rp.entity == playerId;
            if (canSeePlayer)
              {
                std::cout << "stop" << std::endl;  
			}
        } 
		else
		{
            glm::vec3 leftOfPlayer = glm::cross(
                dirEntityTargetPosToPlayer, glm::vec3(0.f, 1.f, 0.f)
            );
            dirEntityToPlayer =
                glm::normalize(leftOfPlayer + dirEntityTargetPosToPlayer);
            entityTargetPos += dirEntityToPlayer;
			if(++safetyBreak>25)
			{
				Log::warning("Swarm ray check running too many times,this is bad");
				break;
			}
		}

    }

    dirEntityToPlayer.y = 0;
    float tempYvel =  rigidbody.velocity.y;
    rigidbody.velocity = dirEntityToPlayer * swarmComp.speed;
    rigidbody.velocity.y = tempYvel;


    

	//TODO: Something may be wrong here??!?
	if (closeEnoughToPlayer(entityID) == BTStatus::Success)
	{
		return BTStatus::Success;
	}

	return ret;
}
BTStatus SwarmBT::closeEnoughToPlayer(Entity entityID)
{
  std::cout << "closeEnough" << std::endl;
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
	std::cout << "ATTACK!" << std::endl;
	BTStatus ret = BTStatus::Running;

	Transform& thisTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID(sceneHandler));
	SwarmComponent& swarmComp = BehaviorTree::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	AiCombat& combat = BehaviorTree::sceneHandler->getScene()->getComponent<AiCombat>(entityID);
	Rigidbody& rigidbody = BehaviorTree::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
	Collider& sawrmCollider = BehaviorTree::sceneHandler->getScene()->getComponent<Collider>(entityID);

	glm::vec3 dir = playerTransform.position - thisTransform.position;
	dir.y += swarmComp.jumpY;
	dir = glm::normalize(dir);

    thisTransform.rotation.y = lookAtY(thisTransform, playerTransform);
	thisTransform.updateMatrix();

	static float initialFriction = rigidbody.friction;


    static Ray downRay{
        thisTransform.position,
        glm::vec3(0.0f,-1.0f,0.0f)
    };
        
    float heightOfSwarmBlob = sawrmCollider.radius + 2.0f;//TODO: get height of swarmblob
    RayPayload rp = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(downRay,heightOfSwarmBlob);   
    if(rp.hit && swarmComp.groundTimer <= 0.0f)
    {
        swarmComp.grounded = true;	
		swarmComp.groundTimer = swarmComp.groundTimerOrig;
    }
     


	if(!swarmComp.grounded && swarmComp.groundTimer > 0.0f)
	{
		swarmComp.groundTimer -= Time::getDT();
	}


	if(swarmComp.grounded && combat.timer > 0.0f)
	{
		combat.timer -= Time::getDT();
		if(thisTransform.scale.y > 0.5f)
		{
			thisTransform.scale.y -= swarmComp.chargeAnimSpeed * Time::getDT();
		}
	}
	else if(!swarmComp.inAttack && !swarmComp.touchedPlayer && swarmComp.grounded)
	{
		//JUMP!
		swarmComp.grounded = false;
		thisTransform.scale.y = 1.0f;
		rigidbody.velocity = dir * swarmComp.jumpForce;
		swarmComp.inAttack = true; 
		rigidbody.friction = 0.0f;
	
		std::cout<<"ATTACK!!!!\n";
		ret = BTStatus::Success;
	}
	else if (swarmComp.grounded)
    {
		swarmComp.inAttack = false; 
		swarmComp.touchedPlayer = false; 
		rigidbody.friction = initialFriction;
		combat.timer = combat.lightAttackTime;
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

	Combat& playerCombat = sceneHandler->getScene()->getComponent<Combat>(getPlayerID(sceneHandler));
	if (playerCombat.health <= (playerCombat.maxHealth - 10))
	{
		playerCombat.health += 10;
	}

	int spawnLoot = rand() % 10;
	if (spawnLoot < 2)
	{
		// Spawn Perk
		PerkType perkType = (PerkType)(rand() % 3);
		Perks perk{ .multiplier = 0.2f, .perkType = perkType };
		Entity perkEnt = sceneHandler->getScene()->createEntity();
		sceneHandler->getScene()->setComponent<MeshComponent>(perkEnt, SwarmBT::perkMeshes[perkType]);
		Transform& perkTrans = sceneHandler->getScene()->getComponent<Transform>(perkEnt);
		Transform& swarmTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
		perkTrans.position = swarmTrans.position;
		perkTrans.scale = glm::vec3(2.f, 2.f, 2.f);
		sceneHandler->getScene()->setComponent<Collider>(perkEnt, Collider::createSphere(2.f, glm::vec3(0.f, 0.f, 0.f), true));
		sceneHandler->getScene()->setComponent<Rigidbody>(perkEnt);
		Rigidbody& perkRb = sceneHandler->getScene()->getComponent<Rigidbody>(perkEnt);
		glm::vec3 spawnDir = glm::vec3((rand() % 201) * 0.01f - 1, 1, (rand() % 200) * 0.01f - 1);
		perkRb.gravityMult = 6.f;
		perkRb.velocity = glm::normalize(spawnDir) * 20.f;
		sceneHandler->getScene()->setComponent<Perks>(perkEnt, perk);
		sceneHandler->getScene()->setComponent<PointLight>(perkEnt, glm::vec3(5.f, 7.f, 9.f));
	}
	else if (spawnLoot == 2)
	{
		AbilityType abilityType = (AbilityType)(rand() % 2);
		Abilities ability{ .abilityType = abilityType };
		Entity abilityEnt = sceneHandler->getScene()->createEntity();
		sceneHandler->getScene()->setComponent<MeshComponent>(abilityEnt, SwarmBT::abilityMeshes[abilityType]);
		Transform& abilityTrans = sceneHandler->getScene()->getComponent<Transform>(abilityEnt);
		Transform& swarmTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
		abilityTrans.position = swarmTrans.position;
		abilityTrans.scale = glm::vec3(4.f, 4.f, 4.f);
		sceneHandler->getScene()->setComponent<Collider>(abilityEnt, Collider::createSphere(4.f, glm::vec3(0.f, 0.f, 0.f), true));
		sceneHandler->getScene()->setComponent<Rigidbody>(abilityEnt);
		Rigidbody& abilityRb = sceneHandler->getScene()->getComponent<Rigidbody>(abilityEnt);
		glm::vec3 spawnDir = glm::vec3((rand() % 201) * 0.01f - 1, 1, (rand() % 200) * 0.01f - 1);
		abilityRb.gravityMult = 4.f;
		abilityRb.velocity = glm::normalize(spawnDir) * 40.f;
		sceneHandler->getScene()->setComponent<Abilities>(abilityEnt, ability);
		sceneHandler->getScene()->setComponent<PointLight>(abilityEnt, glm::vec3(7.f, 9.f, 5.f));
	}

	//TODO: Sometgin goes wrong when we remove from group.
	//SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);

	SwarmGroup* swarmGroup = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID).group;
	swarmGroup->aliveMembers.pop();

	sceneHandler->getScene()->setInactive(entityID);



	return ret;
}

BTStatus SwarmBT::alerted(Entity entityID)
{
  std::cout << "alerted" << std::endl;
	BTStatus ret = BTStatus::Running;
	SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	Transform& playerTransform = BehaviorTree::sceneHandler->getScene()->getComponent<Transform>(getPlayerID(sceneHandler));
	Transform& swarmTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
	Collider& swarmCol = sceneHandler->getScene()->getComponent<Collider>(entityID);
	float toMove = (swarmCol.radius*2) * (1.0f - swarmComp.alertScale);
	
	swarmTrans.rotation.y = lookAtY(swarmTrans, playerTransform);
	swarmTrans.updateMatrix();

	if(!swarmComp.alertAtTop)
	{
        std::cout << swarmTrans.scale.y << std::endl;
		if(swarmTrans.scale.y >= swarmComp.alertScale &&
		swarmTrans.position.y >= (swarmComp.alertTempYpos + toMove))
		{
			swarmComp.alertAtTop = true;
		}
		else
		{
			if (swarmTrans.scale.y < swarmComp.alertScale)
			{
				swarmTrans.scale.y += swarmComp.alertAnimSpeed * Time::getDT();
			}
			if (swarmTrans.position.y < (swarmComp.alertTempYpos + toMove))
			{
				swarmTrans.position.y += swarmComp.alertAnimSpeed * Time::getDT();
			}
		}
	}
	else
	{
		if(swarmTrans.scale.y <= 1.0f)
		{
			swarmTrans.scale.y = 1.0f;
			swarmTrans.position.y = swarmComp.alertTempYpos;
			swarmComp.alertAtTop = false;
			swarmComp.alertDone = true;
			ret = BTStatus::Success;
            std::cout << "succ" << std::endl;
		}
		else
		{
			if(swarmTrans.scale.y > 1.0)
			{
				swarmTrans.scale.y -= swarmComp.alertAnimSpeed * Time::getDT();
			}
		}
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

void Swarm_alerted::start() {


	Sequence* root = c.c.sequence();
    
    // Compositors
	Task* play_alert_anim = c.l.task("Alerted", SwarmBT::alerted);
	
	root->addLeaf(play_alert_anim);

    this->setRoot(root);
}

void Swarm_combat::start() 
{

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
	//TODO : how to do this?
	//ResourceManager* resourceMng = sceneHandler->getResourceManager();
	//this->perkMeshes[0] = resourceMng->addMesh("assets/models/Perk_Hp.obj");
	//this->perkMeshes[1] = resourceMng->addMesh("assets/models/Perk_Dmg.obj");
	//this->perkMeshes[2] = resourceMng->addMesh("assets/models/Perk_AtkSpeed.obj");
	//this->abilityMeshes[0] = resourceMng->addMesh("assets/models/KnockbackAbility.obj");
	//this->abilityMeshes[1] = resourceMng->addMesh("assets/models/KnockbackAbility.obj");

	Sequence* root = c.c.sequence();

	Task* playDeathAnimTask = c.l.task("Play death animation", SwarmBT::playDeathAnim);
	Task* dieTask = c.l.task("die", SwarmBT::die);

	root->addLeafs({playDeathAnimTask, dieTask});

	this->setRoot(root);

}


#include "SwarmBTs.hpp"
#include "../../../Components/Abilities.h"
#include "../../../Components/Combat.h"
#include "../../../Components/Perks.h"
#include "SwarmFSM.hpp"
#include "../../../Network/ServerGameMode.h"
#include <limits>

int SwarmBT::getPlayerID(Entity entityID)
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
    }
    // else find player from script
    else
    {
        std::string playerString = "playerID";
        BehaviorTree::sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
    }
  
    return playerID;
}

float SwarmBT::get_dt()
{
	return BehaviorTree::sceneHandler->getAIHandler()->getDeltaTime();
}

Scene* SwarmBT::getTheScene()
{
	return BehaviorTree::sceneHandler->getScene();
}

void SwarmBT::rotateTowards(Entity entityID, glm::vec3 target, float rotSpeed, float precision)
{
	SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Transform& swarmTrans = getTheScene()->getComponent<Transform>(entityID);
	//Rotate towards target start
	swarmTrans.updateMatrix();
	glm::vec2 targetPos			= glm::vec2(target.x, target.z);
	glm::vec2 swarmpos			= glm::vec2(swarmTrans.position.x, swarmTrans.position.z);
	glm::vec2 curRot			= safeNormalize(glm::vec2(swarmTrans.forward().x, swarmTrans.forward().z));
	glm::vec2 swarm_to_friend	= safeNormalize(targetPos - swarmpos);

	float angle_between			= glm::degrees(glm::acos(glm::dot(swarm_to_friend, curRot)));
	swarmComp.tempRotAngle = angle_between;

	if(swarmComp.rotateLeft && angle_between >= precision)
	{
		swarmTrans.rotation.y += rotSpeed * get_dt();
	}
	else if(angle_between >= precision)
	{
		swarmTrans.rotation.y -= rotSpeed * get_dt();
	}

	//Check if we rotated in correct direction
	swarmTrans.updateMatrix();
	targetPos			= glm::vec2(target.x, target.z);
	swarmpos				= glm::vec2(swarmTrans.position.x, swarmTrans.position.z);
	curRot				= safeNormalize(glm::vec2(swarmTrans.forward().x, swarmTrans.forward().z));
	swarm_to_friend		= safeNormalize(targetPos - swarmpos);
	angle_between		= glm::degrees(glm::acos(glm::dot(swarm_to_friend, curRot))); 
	//If angle got bigger, then change direction
	if(swarmComp.tempRotAngle < angle_between)
	{
		if(swarmComp.rotateLeft)
		{
			swarmComp.rotateLeft = false;
		}
		else
		{
			swarmComp.rotateLeft = true;
		}
	}
	//Rotate towards target end
}

bool SwarmBT::outsideRadius(Entity entityID)
{
	bool ret = false;
	SwarmComponent& sawrmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Transform& swarmTrans = getTheScene()->getComponent<Transform>(entityID);

	float len = glm::length(sawrmComp.group->idleMidPos - swarmTrans.position);
	if(len >sawrmComp.group->idleRadius)
	{
		ret = true;
	}
	return ret;
}

bool SwarmBT::stuckInCorner(RayPayload& rp_left, RayPayload& rp_right, RayPayload& rp_forward)
{
	bool ret = false;
	if(rp_right.hit && rp_left.hit && !rp_forward.hit)
	{
		Collider& col1 = getTheScene()->getComponent<Collider>(rp_right.entity);
		Collider& col2 = getTheScene()->getComponent<Collider>(rp_left.entity);
		if(!col1.isTrigger && !col2.isTrigger)
		{
			ret = true;
		}
	}
	return ret;
}


int SwarmGroup::getNewId = 0;
int SwarmBT::perkMeshes[] = {0, 0, 0};
int SwarmBT::abilityMeshes[] = {0, 0};

void removeFromGroup(SwarmComponent& comp, Entity entityID)
{
    for (int i = 0; i < comp.group->members.size(); i++)
        {
            if (comp.group->members[i] == entityID)
                {
                    comp.group->members.erase(comp.group->members.begin() + i);
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
	    getTheScene()->getComponent<SwarmComponent>(entityID).group;
	if (groupPtr->members.size() <= 1)
	{
		return BTStatus::Failure;
	}
	return ret;
}
BTStatus SwarmBT::jumpInCircle(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    if (hasFriends(entityID) == BTStatus::Failure)
    {
        return BTStatus::Failure;
    }

	Transform& swarmTransform = getTheScene()->getComponent<Transform>(entityID);
	SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& swarmRB = getTheScene()->getComponent<Rigidbody>(entityID);
	Collider& swarmCol = getTheScene()->getComponent<Collider>(entityID);

	static Ray rayForward, rayForwardLeft, rayForwardRight;

	swarmTransform.updateMatrix();
	if(swarmComp.forward == glm::vec3(0.0f, 0.0f, 0.0f))
	{
		swarmComp.forward = swarmTransform.forward();
		swarmComp.right = swarmTransform.right();
	}

	glm::vec3 forward = -swarmComp.forward;
	glm::vec3 right = swarmComp.right;
	//Ray forward
	rayForward.pos = swarmTransform.position;
	rayForward.dir = forward;
	//Ray forward left
	rayForwardLeft.pos = swarmTransform.position;
	rayForwardLeft.dir = safeNormalize(forward - right);
	//Ray forward right
	rayForwardRight.pos = swarmTransform.position;
	rayForwardRight.dir = safeNormalize(forward + right);
	
	float maxDist = swarmCol.radius + 1.5f;
	RayPayload rpForward = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayForward, maxDist);
	RayPayload rpLeft = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayForwardLeft, maxDist);
	RayPayload rpRight = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayForwardRight, maxDist);


	if(swarmComp.dir == glm::vec3(0.0f, 0.0f, 0.0f))
	{
		swarmComp.dir = rayForward.dir;
	}

	bool foundSpecialCase = false;
	if(outsideRadius(entityID))
	{
		swarmComp.dir = safeNormalize(swarmComp.group->idleMidPos - swarmTransform.position);
		foundSpecialCase = true;
	}
	else if(stuckInCorner(rpLeft, rpRight, rpForward) && !swarmComp.idleIgnoreCol)
	{
		swarmComp.idleIgnoreCol = true;
		swarmComp.ignoreColTimer = swarmComp.ignoreColTimerOrig;
		swarmComp.dir = -rayForward.dir;
		foundSpecialCase = true;
	}

	//Tick down ignore timer
	if(swarmComp.ignoreColTimer > 0.0f)
	{
		swarmComp.ignoreColTimer -= get_dt();
	}
	

	if( swarmComp.group->aliveMembers.size() != 1 && !foundSpecialCase)
	{
		glm::vec3 swarmPosRayOffset = swarmTransform.position + rayForward.dir * maxDist;
		float len = glm::length(swarmComp.group->idleMidPos - swarmPosRayOffset);
		if(len > swarmComp.group->idleRadius)
		{
			swarmComp.dir = -rayForward.dir;
		}
		else if(rpForward.hit)
		{
			if(!getTheScene()->getComponent<Collider>(rpForward.entity).isTrigger)
			{
				swarmComp.dir = -rayForward.dir;
			}
		}
		else if(rpLeft.hit)
		{
			if(!getTheScene()->getComponent<Collider>(rpLeft.entity).isTrigger)
			{
				swarmComp.dir = rayForwardRight.dir;
			}
		}
		else if(rpRight.hit)
		{
			if(!getTheScene()->getComponent<Collider>(rpRight.entity).isTrigger)
			{
				swarmComp.dir = rayForwardLeft.dir;
			}
		}	
	}
	else if(!foundSpecialCase)
	{
		if(rpForward.hit)
		{
			if(!getTheScene()->getComponent<Collider>(rpForward.entity).isTrigger)
			{
				swarmComp.dir = -rayForward.dir;
			}
		}
		else if(rpLeft.hit)
		{
			if(!getTheScene()->getComponent<Collider>(rpLeft.entity).isTrigger)
			{
				swarmComp.dir = rayForwardRight.dir;
			}
		}
		else if(rpRight.hit)
		{
			if(!getTheScene()->getComponent<Collider>(rpRight.entity).isTrigger)
			{
				swarmComp.dir = rayForwardLeft.dir;
			}
		}

		else if(swarmComp.lonelyTimer > swarmComp.lonelyTime)
		{	
			swarmComp.lonelyTimer = 0.f;
			swarmComp.lonelyDir = safeNormalize(glm::vec3(rand() * (rand() % 2 == 0 ? - 1 : 1), 0.0f, rand() * (rand() % 2 == 0 ? - 1 : 1)));	
			float a = 4.1f;
			swarmComp.idleMoveTo = swarmTransform.position + swarmComp.lonelyDir * std::numeric_limits<float>().max(); 
		}
		else
		{
			swarmComp.lonelyTimer += get_dt();
		}

	}

	Transform tempTrans = swarmTransform;
	float tempVelY = swarmRB.velocity.y;
	glm::vec3 target = swarmTransform.position + swarmComp.dir*10.0f;
	avoidStuff(entityID, BehaviorTree::sceneHandler, swarmComp.attackGoRight, target, swarmComp.dir);
	swarmRB.velocity = swarmComp.dir * swarmComp.idleSpeed;
	swarmRB.velocity.y = tempVelY;
	glm::vec3 to = swarmTransform.position + swarmComp.dir * 3.0f;
	rotateTowards(entityID, to, swarmComp.idleRotSpeed, 5.0f);
	swarmTransform.updateMatrix();
	swarmComp.forward = swarmTransform.forward();
	swarmComp.right = swarmTransform.right();
	swarmTransform = tempTrans;
	swarmTransform.updateMatrix();

	glm::vec3 targetRot = swarmTransform.position - swarmComp.forward;
	rotateTowards(entityID, targetRot, swarmComp.idleRotSpeed, 5.0f);

	return ret;
}
BTStatus SwarmBT::lookingForGroup(Entity entityID)
{
    BTStatus ret = BTStatus::Running;

    if (hasFriends(entityID) == BTStatus::Failure)
        {
            return BTStatus::Failure;
        }
    return ret;
}
BTStatus SwarmBT::JoinGroup(Entity entityID)
{
    BTStatus ret = BTStatus::Running;

	SwarmComponent& thisSwarmComp =
	    getTheScene()->getComponent<SwarmComponent>(entityID);

	Transform& thisTransform =
	    getTheScene()->getComponent<Transform>(entityID);

    getTheScene()->getSceneReg().view<SwarmComponent, Transform>(entt::exclude<Inactive>).each(
	    [&](const auto& entity, SwarmComponent& swComp, Transform& trans)
	    {
		    if (static_cast<int>(entity) != entityID)
		    {
			    if ((thisTransform.position - trans.position).length() <=
			        thisSwarmComp.sightRadius)
			    {

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

	SwarmComponent& thisSwarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Transform& thisTransform      = getTheScene()->getComponent<Transform>(entityID);

    thisSwarmComp.groupsInSight.clear();

    sceneHandler->getScene()
        ->getSceneReg()
        .view<SwarmComponent, Transform>(entt::exclude<Inactive>)
        .each(
            [&](const auto& entity, SwarmComponent& swComp, Transform& trans)
            {
                if (static_cast<int>(entity) != entityID)
                    {
                        if ((thisTransform.position - trans.position)
                                .length() <= thisSwarmComp.sightRadius)
                            {
                                thisSwarmComp.groupsInSight.push_back(
                                    swComp.group
                                );
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
	    getTheScene()->getComponent<SwarmComponent>(entityID);

    SwarmGroup* nearestGroup = nullptr;
    for (auto& f : thisSwarmComp.groupsInSight)
        {
            nearestGroup = f;
            break;
        }

	Transform& thisTransform =
	    getTheScene()->getComponent<Transform>(entityID);

	glm::vec3 swarmMidPoint;
	for (auto& blob : nearestGroup->members)
	{
		swarmMidPoint += getTheScene()->getComponent<Transform>(blob).position;
	}
	swarmMidPoint /= nearestGroup->members.size();
	
	 if ((thisTransform.position - swarmMidPoint).length() <=
	    thisSwarmComp.sightRadius)
	{

            return BTStatus::Success;
        }


    return ret;
}
BTStatus SwarmBT::escapeFromPlayer(Entity entityID)
{
	BTStatus ret = BTStatus::Running;
    int player = getPlayerID(entityID);
	if(player == -1){return ret;}

	Transform& thisTransform = getTheScene()->getComponent<Transform>(entityID);
	SwarmComponent& thisSwarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Transform playerTransform = getTheScene()->getComponent<Transform>(player);
	SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& rigidbody = getTheScene()->getComponent<Rigidbody>(entityID);

    if (glm::length((thisTransform.position - playerTransform.position)) >
        thisSwarmComp.sightRadius)
    {
        return BTStatus::Success;
    }

	glm::vec3 direction = safeNormalize(thisTransform.position - playerTransform.position);
	glm::vec3 target = thisTransform.position + direction * 2.0f;
    rotateTowards(entityID, target, swarmComp.escapeRotSpeed, 5.0f);
    thisTransform.updateMatrix();

    glm::vec3 dir = safeNormalize(thisTransform.position - playerTransform.position);
	avoidStuff(entityID, BehaviorTree::sceneHandler, thisSwarmComp.attackGoRight, target, dir);
    dir.y = 0;
    float tempYvel = rigidbody.velocity.y;
    rigidbody.velocity = dir * swarmComp.speed;
    rigidbody.velocity.y = tempYvel;



	return ret;
}

BTStatus SwarmBT::informFriends(Entity entityID)
{
    BTStatus ret = BTStatus::Success;

	SwarmComponent& thisSwarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	if(thisSwarmComp.getGroupHealth(getTheScene()) > thisSwarmComp.LOW_HEALTH)
	{
		for (auto& f : thisSwarmComp.group->members)
		{
			SwarmComponent& curSwarmComp = getTheScene()->getComponent<SwarmComponent>(f);
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

    BTStatus ret = BTStatus::Running;

	
	int player_id = getPlayerID(entityID);
	if(player_id == -1){return ret;}
	Collider& entityCollider = getTheScene()->getComponent<Collider>(entityID);
	Collider& playerCollider = getTheScene()->getComponent<Collider>(player_id);
	Transform& entityTransform = getTheScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = getTheScene()->getComponent<Transform>(player_id);
	SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& rigidbody = getTheScene()->getComponent<Rigidbody>(entityID);
    
	entityTransform.updateMatrix();
	glm::vec3 from = playerTransform.position;
	from = from + playerTransform.up() * 3.0f;
	glm::vec3 to = entityTransform.position;
	float maxDist = glm::length(to - from);
	glm::vec3 dir = safeNormalize(to - from);
	Ray rayToPlayer{from, dir};    
	Ray rayRight{to, entityTransform.right()};    
	Ray rayLeft{to, entityTransform.right()};    
	float left_right_maxDist = entityCollider.radius + 1.5f;
    RayPayload rp = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayToPlayer, maxDist);
	
	
	if(rp.hit)
	{
		if(!getTheScene()->getComponent<Collider>(rp.entity).isTrigger &&
			rp.entity != entityID)
		{
			entityTransform.updateMatrix();

			if(swarmComp.attackGoRight)
			{
				dir -= entityTransform.right();
				//Check if we collide on right side
				RayPayload r_right = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayRight, left_right_maxDist);
				
				if(r_right.hit && !getTheScene()->getComponent<Collider>(r_right.entity).isTrigger)
				{
					swarmComp.attackGoRight = false;
					dir += entityTransform.right();
				}
			}
			else
			{
				dir += entityTransform.right();
				//Check if we collide on left side
				RayPayload r_left = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayLeft, left_right_maxDist);
				
				if(r_left.hit && !getTheScene()->getComponent<Collider>(r_left.entity).isTrigger)
				{
					swarmComp.attackGoRight = true;
					dir -= entityTransform.right();
				}
			}

		}
	}

	rotateTowards(entityID, playerTransform.position, swarmComp.idleRotSpeed, 5.0f);
	safeNormalize(dir);
	dir.y = 0;
    float tempYvel =  rigidbody.velocity.y;
    rigidbody.velocity = -dir * swarmComp.speed;
    rigidbody.velocity.y = tempYvel;

    if (closeEnoughToPlayer(entityID) == BTStatus::Success)
    {
        return BTStatus::Success;
    }

    return ret;
}
BTStatus SwarmBT::closeEnoughToPlayer(Entity entityID)
{
    BTStatus ret = BTStatus::Failure;
	int playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}
	SwarmComponent& thisSwarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Transform& thisTransform = getTheScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = getTheScene()->getComponent<Transform>(playerID);

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

	int playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}
	Transform& thisTransform = getTheScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = getTheScene()->getComponent<Transform>(playerID);
	SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Rigidbody& rigidbody = getTheScene()->getComponent<Rigidbody>(entityID);
	Collider& sawrmCollider = getTheScene()->getComponent<Collider>(entityID);

    glm::vec3 playerTargetPos{playerTransform.position.x,playerTransform.position.y + SwarmComponent::aimAtPlayerYOffset, playerTransform.position.z};
    glm::vec3 dir = playerTargetPos - thisTransform.position;
    dir.y += swarmComp.jumpY;
    dir = safeNormalize(dir);

	rotateTowards(entityID, playerTransform.position, swarmComp.attackRotSpeed, 5.0f);
    thisTransform.updateMatrix();

    static float initialFriction = rigidbody.friction;

    static Ray downRay{thisTransform.position, glm::vec3(0.0f, -1.0f, 0.0f)};

	if(swarmComp.grounded && swarmComp.timer > 0.0f)
	{
		swarmComp.timer -= get_dt();
		if(thisTransform.scale.y > (swarmComp.origScaleY * 0.5f))
		{
			thisTransform.scale.y -= swarmComp.chargeAnimSpeed * get_dt();
		}

		if(glm::length(playerTransform.position - thisTransform.position) < swarmComp.attackRange/2.0f)
		{
			glm::vec3 direction = safeNormalize(thisTransform.position - playerTransform.position);
			avoidStuffBackwards(entityID, BehaviorTree::sceneHandler, swarmComp.attackGoRight, playerTransform.position, direction);
			rigidbody.velocity =  direction * swarmComp.speed;
		}
	}
	else if(!swarmComp.inAttack && !swarmComp.touchedPlayer && swarmComp.grounded)
	{
		//JUMP!
		swarmComp.grounded = false;
		thisTransform.scale.y = swarmComp.origScaleY;
		rigidbody.velocity = dir * swarmComp.jumpForce;
		swarmComp.inAttack = true; 
		rigidbody.friction = 0.0f;

		ServerGameMode* netScene = dynamic_cast<ServerGameMode*>(getTheScene());
		if (netScene)
		{
			netScene->addEvent({ (int)GameEvent::PLAY_ENEMY_SOUND, entityID, 0, 1, 0 });
		}
		else
		{
			sceneHandler->getAudioHandler()->playSound(entityID, SwarmComponent::s_attack, 10.f);
		}
	
		Log::write("ATTACK!!!!", BT_FILTER);
		ret = BTStatus::Success;
	}
	else if (swarmComp.grounded)
    {
		swarmComp.inAttack = false; 
		swarmComp.touchedPlayer = false; 
		rigidbody.friction = initialFriction;
		swarmComp.timer = swarmComp.lightAttackTime;
    }

	return ret;
}

BTStatus SwarmBT::playDeathAnim(Entity entityID)
{
    BTStatus ret = BTStatus::Running;
    SwarmComponent& swarmComp =
        sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
    Transform& swarmTrans =
        sceneHandler->getScene()->getComponent<Transform>(entityID);
    if (swarmTrans.scale.y <= 0.0f)
        {
            ret = BTStatus::Success;
        }
    else
        {
            swarmTrans.rotation.y += 1000 * swarmComp.deathAnimSpeed * get_dt();
            swarmTrans.scale.y -= swarmComp.deathAnimSpeed * get_dt();
        }

    return ret;
}

BTStatus SwarmBT::die(Entity entityID)
{
    BTStatus ret = BTStatus::Success;
	int playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}
    HealthComp& playerHealth = sceneHandler->getScene()->getComponent<HealthComp>(playerID);
    if (playerHealth.health <= (playerHealth.maxHealth - 10))
    {
        playerHealth.health += 10;
    }
	static int chanceToSpawnPerk = 2;
	static int chanceToSpawnability = 1;
    int spawnLoot = rand() % 10;
    ServerGameMode* serverScene = dynamic_cast<ServerGameMode*>(sceneHandler->getScene());

    if (serverScene != nullptr)
    {
        int itemID, type = -1, otherType;
        float multiplier = 0.f;
        if (spawnLoot < chanceToSpawnPerk)
        {
            type = (int)ItemType::PERK;
            otherType = rand() % PerkType::emptyPerk;
			switch (otherType)
			{
			case PerkType::hpUpPerk:
				multiplier = 0.1f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.1f;
				break;
			case PerkType::dmgUpPerk:
				multiplier = 0.1f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.1f;
				break;
			case PerkType::attackSpeedUpPerk:
				multiplier = 0.05f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.05f;
				break;
			case PerkType::movementUpPerk:
				multiplier = 0.2f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.1f;
				break;
			case PerkType::staminaUpPerk:
				multiplier = 0.1f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.1f;
				break;
			}
			if (otherType == PerkType::attackSpeedUpPerk && getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel > 6)
			{
				multiplier = 0.4f;
			}
			else if (otherType == PerkType::movementUpPerk && getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel > 7)
			{
				multiplier = 1.f;
			}
        }
        else if (spawnLoot < chanceToSpawnPerk + chanceToSpawnability)
        {
            type = (int)ItemType::ABILITY;
            otherType = (AbilityType)(rand() % 2);
            multiplier = 0.0f;
        }
        if (type != -1)
        {
            glm::vec3 spawnPos = sceneHandler->getScene()->getComponent<Transform>(entityID).position;
            glm::vec3 spawnDir = glm::vec3((rand() % 201) * 0.01f - 1, 1, (rand() % 200) * 0.01f - 1);
            itemID = serverScene->spawnItem((ItemType)type, otherType, multiplier);
            serverScene->addEvent(
                {(int)GameEvent::SPAWN_ITEM,
                itemID,
                type, 
                otherType}, 
                {multiplier,
                spawnPos.x, spawnPos.y, spawnPos.z,
                spawnDir.x, spawnDir.y, spawnDir.z}
            );
        }
    }
    else
    {
		PerkType otherType;
		float multiplier;
		if (spawnLoot < 2)
		{
			otherType = PerkType(rand() % PerkType::emptyPerk);
			NetworkHandlerGame* network = dynamic_cast<NetworkHandlerGame*>(sceneHandler->getNetworkHandler());
			Transform& swarmTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);

			switch (otherType)
			{
			case PerkType::hpUpPerk:
				multiplier = 0.1f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.1f;
				break;
			case PerkType::dmgUpPerk:
				multiplier = 0.1f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.1f;
				break;
			case PerkType::attackSpeedUpPerk:
				multiplier = 0.05f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.05f;
				break;
			case PerkType::movementUpPerk:
				multiplier = 0.2f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.1f;
				break;
			case PerkType::staminaUpPerk:
				multiplier = 0.1f + getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel * 0.1f;
				break;
			}
			if (otherType == PerkType::attackSpeedUpPerk && getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel > 6)
			{
				multiplier = 0.4f;
			}
			else if (otherType == PerkType::movementUpPerk && getTheScene()->getComponent<SwarmComponent>(entityID).currentLevel > 7)
			{
				multiplier = 1.f;
			}

			network->spawnItemRequest(otherType, multiplier, swarmTrans.position,
				glm::vec3((rand() % 201) * 0.01f - 1, 1, (rand() % 200) * 0.01f - 1));
		}
		else if (spawnLoot == 2)
		{
			NetworkHandlerGame* network = dynamic_cast<NetworkHandlerGame*>(sceneHandler->getNetworkHandler());
			Transform& swarmTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
			network->spawnItemRequest((AbilityType)(rand() % 2), swarmTrans.position,
				glm::vec3((rand() % 201) * 0.01f - 1, 1, (rand() % 200) * 0.01f - 1));
		}
    }

    SwarmGroup* swarmGroup = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID).group;
    swarmGroup->aliveMembers.pop();

    if (serverScene != nullptr) 
    {
        serverScene->addEvent({(int)GameEvent::INACTIVATE, entityID});
    }
    sceneHandler->getScene()->setInactive(entityID);
	ret = BTStatus::Success;
	if(ret==BTStatus::Success)
	{
		SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
		if(swarmComp.isElite)
		{
			swarmComp.removeEliteStats(sceneHandler->getScene(), entityID);
		}
	}

    return ret;
}

BTStatus SwarmBT::alerted(Entity entityID)
{
	BTStatus ret = BTStatus::Running;
	int playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}
	SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(entityID);
	Transform& playerTransform = getTheScene()->getComponent<Transform>(playerID);
	Transform& swarmTrans = getTheScene()->getComponent<Transform>(entityID);
	Collider& swarmCol = getTheScene()->getComponent<Collider>(entityID);
	float toMove = (swarmCol.radius*2) * (swarmComp.origScaleY - (swarmComp.origScaleY*swarmComp.alertScale));
	
	rotateTowards(entityID, playerTransform.position, swarmComp.attackRotSpeed, 5.0f);
	swarmTrans.updateMatrix();


    if (!swarmComp.alertAtTop)
        {
            if (swarmTrans.scale.y >= (swarmComp.origScaleY*swarmComp.alertScale) &&
                swarmTrans.position.y >= (swarmComp.alertTempYpos + toMove))
                {
                    swarmComp.alertAtTop = true;
                }
            else
                {
                    if (swarmTrans.scale.y < (swarmComp.origScaleY*swarmComp.alertScale))
                        {
                            swarmTrans.scale.y +=
                                swarmComp.alertAnimSpeed * get_dt();
                        }
                    if (swarmTrans.position.y <
                        (swarmComp.alertTempYpos + toMove))
                        {
                            swarmTrans.position.y +=
                                swarmComp.alertAnimSpeed * get_dt();
                        }
                }
        }
    else
        {
            if (swarmTrans.scale.y <= swarmComp.origScaleY)
                {
                    swarmTrans.scale.y = swarmComp.origScaleY;
                    swarmTrans.position.y = swarmComp.alertTempYpos;
                    swarmComp.alertAtTop = false;
                    swarmComp.alertDone = true;
                    ret = BTStatus::Success;
                }
            else
                {
                    if (swarmTrans.scale.y > swarmComp.origScaleY)
                        {
                            swarmTrans.scale.y -=
                                swarmComp.alertAnimSpeed * get_dt();
                        }
                }
        }

    return ret;
}

void Swarm_idle::start()
{

    Selector* root = c.c.selector();

    // Compositors
    Sequence* socializing = c.c.sequence();
    Sequence* groupExploring = c.c.sequence();

    // Leafs
    Condition* has_friends = c.l.condition("Has firends", SwarmBT::hasFriends);
    Task* jump_in_circle = c.l.task("Jump in circle", SwarmBT::jumpInCircle);
    Task* lfg = c.l.task("Looking for group", SwarmBT::lookingForGroup);
    Task* join_group = c.l.task("Join group", SwarmBT::JoinGroup);

    // Relations
    root->addCompositors({socializing, groupExploring});
    socializing->addLeafs({has_friends, jump_in_circle});
    groupExploring->addLeafs({lfg, join_group});

    this->setRoot(root);
}

void Swarm_alerted::start()
{

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
    Condition* close_enough_to_player =
        c.l.condition("Close enough to player", SwarmBT::closeEnoughToPlayer);
    Task* jump_towards_player =
        c.l.task("Jump towards player", SwarmBT::jumpTowardsPlayer);

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
    Task* escape_from_player =
        c.l.task("Escape from plater", SwarmBT::escapeFromPlayer);

    Condition* sees_friends =
        c.l.condition("Sees firends", SwarmBT::seesNewFriends);
    Task* escape_to_friends =
        c.l.task("Escape to friends", SwarmBT::escapeToFriends);

    // Relations
    root->addCompositors({escape_to_friends_if_possible});
    root->addLeaf(escape_from_player);

    escape_to_friends_if_possible->addLeafs({sees_friends, escape_to_friends});

    this->setRoot(root);
}

void Swarm_dead::start()
{
    Sequence* root = c.c.sequence();

    Task* playDeathAnimTask =
        c.l.task("Play death animation", SwarmBT::playDeathAnim);
    Task* dieTask = c.l.task("die", SwarmBT::die);

    root->addLeafs({playDeathAnimTask, dieTask});

    this->setRoot(root);
}
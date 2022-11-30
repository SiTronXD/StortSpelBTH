#include "TankBTs.hpp"
#include "TankFSM.hpp"

Scene* TankBT::getTheScene()
{
    return BehaviorTree::sceneHandler->getScene();
}

void TankBT::updateCanBeHit(Entity entityID)
{
	int playerID = getPlayerID();
	Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	glm::vec3 tank_player_vec = playerTrans.position - tankTrans.position;
	float tank_player_len = glm::length(tank_player_vec);
	tank_player_vec = glm::normalize(tank_player_vec);
	float hitDeg = (360.0f - tankComp.shieldAngle)/2.0f;
	hitDeg = 180 - hitDeg;
	if(tank_player_len < tankComp.peronalSpaceRadius)
	{
		if(getAngleBetween(tank_player_vec, -tankTrans.forward()) >= hitDeg)
		{
			tankComp.canBeHit = true;
		}
		else
		{
			tankComp.canBeHit = false;
		}
	}
}

int	TankBT::numActiveHumps(Entity entityID)
{
	int ret = 0;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	for(auto e: tankComp.humpEnteties)
	{
		if(getTheScene()->isActive(e))
		{
			ret++;
		}
	}
	return ret;
}
uint32_t TankBT::activateHump(Entity entityID)
{
	uint32_t ret = -1;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	for(auto e: tankComp.humpEnteties)
	{
		if(!getTheScene()->isActive(e))
		{
			getTheScene()->setActive(e);
			ret = e;
			//std::cout<<"New hump!\nNum Humps active: "<<numActiveHumps(entityID)<<"Num actual humps: "<<tankComp.humps.size()<<std::endl;
			break;
		}
	}
	return ret;
}
void TankBT::deactivateHump(Entity entityID, uint32_t what)
{
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	for(auto e: tankComp.humpEnteties)
	{
		if(e == what)
		{
			getTheScene()->setInactive(e);
			//std::cout<<"Removing hump!\n";
			break;
		}
	}
}
void TankBT::updateHump(Entity entityID, uint32_t what)
{
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);

	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
	Transform& trans = getTheScene()->getComponent<Transform>(what);
	trans.scale.x = trans.scale.z = tankComp.humps[what];
	trans.position.x = tankTrans.position.x;
	trans.position.z = tankTrans.position.z;
}
bool TankBT::canActivateNewHump(Entity entityID)
{
	bool ret = false;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	for(auto e: tankComp.humpEnteties)
	{
		if(!getTheScene()->isActive(e))
		{
			ret = true;
			break;
		}
	}
	return ret;
}
void TankBT::groundHumpShortcut(Entity entityID, float maxRad)
{
	Collider& tankCol = getTheScene()->getComponent<Collider>(entityID);
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	tankComp.chargeTimer = tankComp.chargeTimerOrig;
	tankComp.hasRunTarget = false;

	if(tankComp.groundHumpTimer <= 0)
	{
		if(canActivateNewHump(entityID))
		{
			
			Log::write("Stomp!", BT_FILTER);
			uint32_t newHump = activateHump(entityID);
			Transform& hTrans = getTheScene()->getComponent<Transform>(newHump);
			hTrans.position = getTheScene()->getComponent<Transform>(entityID).position;
			hTrans.position.y -= tankCol.radius - 0.5f;
			tankComp.humps.insert({newHump, 1.0f});
			tankComp.groundHumpTimer = tankComp.groundHumpTimerOrig;
		}
		else
		{
			std::cout<<"No avaliable humps!\n";
		}
       
	}
	else
	{
		tankComp.groundHumpTimer -= get_dt();
	}

	int playerID = getPlayerID();
	
	Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
	Collider& playerCol = getTheScene()->getComponent<Collider>(playerID);
	Rigidbody& playerRB = getTheScene()->getComponent<Rigidbody>(playerID);
	Script& playerScript = getTheScene()->getComponent<Script>(playerID);
	bool playerGrounded = false;
	BehaviorTree::sceneHandler->getScriptHandler()->getScriptComponentValue(playerScript, playerGrounded, "onGround");
	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
	std::vector<int> toRemove;


	rotateTowards(entityID, playerTrans.position, tankComp.combatRotSpeed, 5.0f);

	for(auto& h: tankComp.humps)
	{
		h.second += tankComp.humpShockwaveSpeed * get_dt();

		updateHump(entityID, h.first);

		float dist = glm::length(playerTrans.position - tankTrans.position);
		float minHitDist = dist - playerCol.radius;
		float maxHitDist = dist + playerCol.radius;

		if(h.second/2.0f >= maxRad)
		{
			toRemove.push_back(h.first);
		}
		else if(h.second/2.0f >= minHitDist && h.second/2.0f <= maxHitDist && playerGrounded)
		{
			//PlayerHit!
			Script& playerScript = getTheScene()->getComponent<Script>(playerID);
			BehaviorTree::sceneHandler->getScriptHandler()->setScriptComponentValue(playerScript , 1.0f, "pushTimer");
			glm::vec3 to = playerTrans.position;
			glm::normalize(to);
			getTheScene()->getComponent<Combat>(playerID).health -= (int)tankComp.humpHit;

			glm::vec3 dir = glm::normalize(to - tankTrans.position);
			playerRB.velocity = dir * tankComp.humpForce;
			playerRB.velocity.y += tankComp.humpYForce;
			//toRemove.push_back(h.first);
		}
	}
	for(auto r: toRemove)
	{
		deactivateHump(entityID, r);
		tankComp.humps.erase(r);
	}


	updateCanBeHit(entityID);
}
void TankBT::drawRaySimple(Ray& ray, float dist, glm::vec3 color)
{
	//Draw ray
	BehaviorTree::sceneHandler->getDebugRenderer()->renderLine(
	ray.pos,
	ray.pos + ray.dir * dist,
	glm::vec3(1.0f, 0.0f, 0.0f));
}

bool TankBT::rayChecking(Entity entityID, glm::vec3& moveDir)
{
	bool ret = true;
	bool somethingInTheWay = false;
	bool canGoForward=true;	
	bool canGoRight=true;
	bool canGoLeft=true;

	int player_id = getPlayerID();
	Collider& entityCollider = getTheScene()->getComponent<Collider>(entityID);
	Collider& playerCollider = getTheScene()->getComponent<Collider>(player_id);
	Transform& entityTransform = getTheScene()->getComponent<Transform>(entityID);
	Transform& playerTransform = getTheScene()->getComponent<Transform>(player_id);
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    
	entityTransform.updateMatrix();
	glm::vec3 from = playerTransform.position;
	from = from + playerTransform.up() * 3.0f;
	glm::vec3 to = entityTransform.position;
	float maxDist = glm::length(to - from);
	glm::vec3 dir = glm::normalize(from - to);
	Ray rayToPlayer{from, -dir};    
	Ray rayRight{to, entityTransform.right()};    
	Ray rayLeft{to, -entityTransform.right()};    
	float left_right_maxDist = entityCollider.radius + 3.0f;
    RayPayload rp = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayToPlayer, maxDist);
	drawRaySimple(rayToPlayer, maxDist);
	if(rp.hit)
	{
		
		if(!getTheScene()->getComponent<Collider>(rp.entity).isTrigger &&
			rp.entity != entityID)
		{
			ret = false;
			somethingInTheWay = true;
			entityTransform.updateMatrix();

			RayPayload r_right= BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayRight, left_right_maxDist);
			RayPayload r_left = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayLeft, left_right_maxDist);
			RayPayload r_forward = BehaviorTree::sceneHandler->getPhysicsEngine()->raycast(rayToPlayer, left_right_maxDist);
			drawRaySimple(rayToPlayer, left_right_maxDist);
			drawRaySimple(rayRight, left_right_maxDist);
			drawRaySimple(rayLeft, left_right_maxDist);

			if(r_forward.hit && !getTheScene()->getComponent<Collider>(r_forward.entity).isTrigger)
			{
				canGoForward = false;
			}
			if(r_right.hit && !getTheScene()->getComponent<Collider>(r_right.entity).isTrigger)
			{
				canGoRight = false;
				tankComp.attackGoRight = false;
			}
			if(r_left.hit && !getTheScene()->getComponent<Collider>(r_left.entity).isTrigger)
			{
				canGoLeft = false;
				tankComp.attackGoRight = true;
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

		if(canGoRight && tankComp.attackGoRight)
		{
			dir += entityTransform.right();
		}
		else if(canGoLeft && !tankComp.attackGoRight)
		{
			dir -= entityTransform.right();
		}
	}

	if(dir == glm::vec3(0.0f, 0.0f, 0.0f))
	{
		dir = glm::vec3(1.0f, 0.0f, 0.0f);
	}
	rotateTowards(entityID, playerTransform.position, tankComp.idleRotSpeed, 5.0f);
	glm::normalize(dir);
	dir.y = 0;
	moveDir = dir;


	return ret;
}

void TankBT::giveFriendsHealth(Entity entityID)
{
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
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
					swarmComp.life += toAdd;
				}
		        
		    }
		    else if(f.second.type == "Lich")
		    {
				LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(f.first);
		        lichComp.shieldedByTank = true;
				int toAdd = tankComp.friendHealthRegen;
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
		tankComp.friendHealTimer -= get_dt();
	}
}

float TankBT::get_dt()
{
    return BehaviorTree::sceneHandler->getAIHandler()->getDeltaTime();
}

int TankBT::getPlayerID()
{
    int playerID = -1;
    std::string playerId_str = "playerID";
    BehaviorTree::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str);
    return playerID;
}

void TankBT::rotateTowardsTarget(Entity entityID, float precision)
{
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
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
		tankTrans.rotation.y += tankComp.idleRotSpeed * get_dt();
	}
	else if(angle_between >= precision)
	{
		tankTrans.rotation.y -= tankComp.idleRotSpeed * get_dt();
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
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
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
		tankTrans.rotation.y += rotSpeed * get_dt();
	}
	else if(angle_between >= precision)
	{
		tankTrans.rotation.y -= rotSpeed * get_dt();
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

bool TankBT::rotationDone(Entity entityID, glm::vec3 target, float rotSpeed, float precision)
{
	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
	tankTrans.updateMatrix();
	glm::vec2 targetPos			= glm::vec2(target.x, target.z);
	glm::vec2 tankPos			= glm::vec2(tankTrans.position.x, tankTrans.position.z);
	glm::vec2 curRot			= -glm::normalize(glm::vec2(tankTrans.forward().x, tankTrans.forward().z));
	glm::vec2 tank_to_friend	= glm::normalize(targetPos - tankPos);

	float angle_between			= glm::degrees(glm::acos(glm::dot(tank_to_friend, curRot)));

	if(angle_between <= precision)
	{
		return true;
	}
	return false;
}

void TankBT::registerEntityComponents(Entity entityId)
{
	addRequiredComponent<TankComponent>(entityId);
	addRequiredComponent<AiCombatTank>(entityId);
}


BTStatus TankBT::HasFreindsTarget(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	if(tankComp.firendTarget.id != -1)
	{
		ret = BTStatus::Success;
	}
	return ret;
}

BTStatus TankBT::AreFriendsAlive(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	if( tankComp.allFriends.size() > 0)
	{
		ret = BTStatus::Success;
	}
	return ret;
}

BTStatus TankBT::PickNewFreinds(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;

	float minDist = 10000.0f;
	int minDistID = -1;
	std::string minDistType = "";
	Transform& tankTrans = getTheScene()->getComponent<Transform>(entityID);
	//Get all unvisited freinds
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
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
	TankComponent& tankComp	= getTheScene()->getComponent<TankComponent>(entityID);
	Transform& tankTrans	= getTheScene()->getComponent<Transform>(entityID);
	tankComp.firendTarget	= {entityID, tankTrans.position};
	return ret;
}

BTStatus TankBT::MoveAround(Entity entityID)
{
	BTStatus ret = BTStatus::Running;

	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	if(tankComp.firendTarget.id == entityID)
	{
		tankComp.firendTarget.id = -1;
		return ret;
	}
	Transform& tankTrans	= getTheScene()->getComponent<Transform>(entityID);
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

	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    int playerID = getPlayerID();
    Transform& playerTrans  = getTheScene()->getComponent<Transform>(playerID);
    Transform& tankTrans    = getTheScene()->getComponent<Transform>(entityID);
    float tank_player_dist	= glm::length(playerTrans.position - tankTrans.position);
    if(tank_player_dist <= tankComp.peronalSpaceRadius)
    {
        ret = BTStatus::Success;
    }

	return ret;
}

BTStatus TankBT::GroundHump(Entity entityID)
{
	BTStatus ret = BTStatus::Running;

	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	groundHumpShortcut(entityID, tankComp.humpShockwaveAttackRadius);

	return ret;
}

BTStatus TankBT::playerOutsidePersonalSpace(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
    int playerID = getPlayerID();
    Transform& playerTrans  = getTheScene()->getComponent<Transform>(playerID);
    Transform& tankTrans    = getTheScene()->getComponent<Transform>(entityID);
    float tank_player_dist	= glm::length(playerTrans.position - tankTrans.position);
    if(tank_player_dist > tankComp.peronalSpaceRadius)
    {
        ret = BTStatus::Success;
    }
	return ret;
}

BTStatus TankBT::ChargeAndRun(Entity entityID)
{
	BTStatus ret = BTStatus::Running;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	int playerID			= getPlayerID();
    Transform& playerTrans  = getTheScene()->getComponent<Transform>(playerID);
    Transform& tankTrans    = getTheScene()->getComponent<Transform>(entityID);
	Collider& tankCol = getTheScene()->getComponent<Collider>(entityID);
	Rigidbody& rb = getTheScene()->getComponent<Rigidbody>(entityID);

	tankComp.humps.clear();
	for(auto h: tankComp.humpEnteties)
	{
		getTheScene()->setInactive(h);
	}
	tankComp.groundHumpTimer = 0.0f;

	glm::vec3 dir;
	if(!rayChecking(entityID, dir))
	{
		rb.velocity = dir * tankComp.idleSpeed;
		return ret;
	}


	if(!tankComp.hasRunTarget && (tankComp.chargeTimer > 0.0f || !rotationDone(entityID, playerTrans.position, tankComp.idleRotSpeed, 5.0f)))
	{
		rotateTowards(entityID, playerTrans.position, tankComp.combatRotSpeed, 5.0f);
		tankComp.chargeTimer -= get_dt();
		return ret;
	}
	if(!tankComp.hasRunTarget)
	{
		tankComp.runTarget = playerTrans.position;
		tankComp.runOrigin = tankTrans.position;
		tankComp.runDist = glm::length(playerTrans.position - tankTrans.position);
		tankComp.runDir = glm::normalize(playerTrans.position - tankTrans.position);
		tankComp.hasRunTarget = true;
		tankComp.canAttack = true;
	}

	

	if(glm::length(tankComp.runOrigin - tankTrans.position) < tankComp.runDist &&
		tankComp.runTimer > 0.0f)
	{
		tankComp.runTimer -= get_dt();
		rb.velocity = tankComp.runDir * tankComp.cahargeSpeed;
	}
	else
	{
		tankComp.chargeTimer = tankComp.chargeTimerOrig;
		tankComp.runTimer = tankComp.runTimerOrig;
		tankComp.hasRunTarget = false;
		tankComp.canAttack = false;
	}


	return ret;
}

BTStatus TankBT::getNearestGroupToPlayer(Entity entityID)
{
	BTStatus ret = BTStatus::Success;

	glm::vec3 average		= glm::vec3(0.0f, 0.0f, 0.0f);
	int num					= 0;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	for(auto f: tankComp.friendsInSight)
	{
		if(f.second.type == "Swarm")
		{
			SwarmComponent& swarmComp = getTheScene()->getComponent<SwarmComponent>(f.first);
			if(swarmComp.inCombat)
			{
				average += getTheScene()->getComponent<Transform>(f.first).position;
				num++;
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
		average = getTheScene()->getComponent<Transform>(entityID).position;
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
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	Transform& tankTrans	= getTheScene()->getComponent<Transform>(entityID);
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
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	Transform& tankTrans	= getTheScene()->getComponent<Transform>(entityID);
	Rigidbody& tankRb		= getTheScene()->getComponent<Rigidbody>(entityID);
	glm::vec3 moveDir		= pathFindingManager.getDirTo(tankTrans.position, tankComp.shieldTargetPos);
	moveDir					= glm::normalize(moveDir);
	tankRb.velocity			= moveDir * tankComp.shieldSpeed;
	return ret;
}

BTStatus TankBT::HoldShield(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;
	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);

	giveFriendsHealth(entityID);
	

	int playerID = getPlayerID();
	Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
	rotateTowards(entityID, playerTrans.position, tankComp.shildRotSpeed, 5.0f);

	groundHumpShortcut(entityID, tankComp.humpShockwaveShieldRadius);

	updateCanBeHit(entityID);

	return ret;
}

BTStatus TankBT::playAlertAnim(Entity entityID)
{
	BTStatus ret = BTStatus::Running;

	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	int playerID = getPlayerID();
	Transform& playerTransform = getTheScene()->getComponent<Transform>(playerID);
	Transform& tankTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
	Collider& tankCol = sceneHandler->getScene()->getComponent<Collider>(entityID);
	float toMove = (tankCol.radius*2) * (1.0f - tankComp.origScaleY + tankComp.alertScale);
	
	tankTrans.rotation.y = lookAtY(tankTrans, playerTransform);
	tankTrans.updateMatrix();

	if(!tankComp.alertAtTop)
	{
		if(tankTrans.scale.y >= tankComp.origScaleY + tankComp.alertScale &&
		tankTrans.position.y >= (tankComp.alertTempYpos + toMove))
		{
			tankComp.alertAtTop = true;
		}
		else
		{
			if (tankTrans.scale.y < tankComp.origScaleY + tankComp.alertScale)
			{
				tankTrans.scale.y += tankComp.alertAnimSpeed * get_dt();
			}
			if (tankTrans.position.y < (tankComp.alertTempYpos + toMove))
			{
				tankTrans.position.y += tankComp.alertAnimSpeed * get_dt();
			}
		}
	}
	else
	{
		if(tankTrans.scale.y <= tankComp.origScaleY)
		{
			tankTrans.scale.y = tankComp.origScaleY;
			tankTrans.position.y = tankComp.alertTempYpos;
			tankComp.alertAtTop = false;
			tankComp.alertDone = true;
			ret = BTStatus::Success;
		}
		else
		{
			if(tankTrans.scale.y > 1.0)
			{
				tankTrans.scale.y -= tankComp.alertAnimSpeed * get_dt();
			}
		}
	}

	return ret;
}

BTStatus TankBT::playDeathAnim(Entity entityID)
{
	BTStatus ret = BTStatus::Failure;

	TankComponent& tankComp = getTheScene()->getComponent<TankComponent>(entityID);
	Transform& tankTrans = sceneHandler->getScene()->getComponent<Transform>(entityID);
	if(tankTrans.scale.y <= 0.0f)
	{
		ret = BTStatus::Success;
	}
	else
	{
		tankTrans.rotation.y +=  1000*tankComp.deathAnimSpeed*get_dt();
		tankTrans.scale.y -= tankComp.deathAnimSpeed*get_dt();
	}

	return ret;
}

BTStatus TankBT::die(Entity entityID)
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

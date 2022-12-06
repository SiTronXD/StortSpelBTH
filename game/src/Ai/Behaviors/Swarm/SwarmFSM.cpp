#include "SwarmFSM.hpp"
#include "../../../Scenes/GameScene.h"
#include "../../../Network/NetworkHandlerGame.h"

Entity SwarmFSM::getPlayerID(Entity entityID){
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
        FSM::sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
    }
  
    return playerID;
}

float SwarmFSM::getEntityDist(Entity one, Entity two)
{
	Transform& trans1 = FSM::sceneHandler->getScene()->getComponent<Transform>(one);
	Transform& trans2 = FSM::sceneHandler->getScene()->getComponent<Transform>(two);
	return glm::length(trans1.position - trans2.position);
}

bool SwarmFSM::idle_alerted(Entity entityID)
{
	bool ret = false;
	updateSwarmGrounded(entityID);

	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());
	if(enemySwarmComp.life <= 0 || groupHealth < enemySwarmComp.LOW_HEALTH || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}
	int playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}
    float swarmPlayerLen = getEntityDist(entityID, playerID);

	if (swarmPlayerLen <= enemySwarmComp.sightRadius || 
		enemySwarmComp.group->inCombat)
    {
		ret = true;
    }


	if(ret == true)
	{
		enemySwarmComp.alertTempYpos = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID).position.y;
		enemySwarmComp.alertDone = false;
		enemySwarmComp.alertAtTop = false;
	}

	return ret;
}

bool SwarmFSM::alerted_combat(Entity entityID)
{
	bool ret = false;
	updateSwarmGrounded(entityID);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}

	if(enemySwarmComp.alertDone)
	{
		enemySwarmComp.alertDone = false;
		enemySwarmComp.group->inCombat = true;
		ret = true;
	}
	return ret;
}

bool SwarmFSM::idle_escape(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}
    int playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}
	float swarmPlayerLen = getEntityDist(entityID, playerID);
	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());

	if (swarmPlayerLen <= enemySwarmComp.sightRadius && 
		groupHealth <= enemySwarmComp.LOW_HEALTH)
    {
		ret = true;
    }

	return ret;
}

bool SwarmFSM::combat_idle(Entity entityID)
{
	bool ret = false;
	updateSwarmGrounded(entityID);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}
	Entity playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}

	Transform& playerTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);

    bool noPlayerInSight = true;
	bool canSePlayer = false;
	float swarmPlayerLen = getEntityDist(entityID, playerID);
	if (swarmPlayerLen <= enemySwarmComp.sightRadius)
	{
		canSePlayer = true;
	}


	if(!canSePlayer && !enemySwarmComp.group->inCombat)
	{
		//I cant see player and my group is not incombat
		ret = true;
	}
	else if(canSePlayer)
	{
		//I can see player
		ret = false;
	}
	else if(enemySwarmComp.group->inCombat)
	{
		//Group may see player
		for(auto& p: enemySwarmComp.group->members)
		{
			Transform& enmyTrans = FSM::sceneHandler->getScene()->getComponent<Transform>(p);
			
			SwarmComponent& enmyswarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(p);
			if (glm::length(enmyTrans.position - playerTransform.position) > enmyswarmComp.sightRadius)
			{
				ret = true;
			}
			else
			{
				noPlayerInSight = false;
				ret = false;
				break;
			}
		}
	}

	if(ret == true)
	{
		enemySwarmComp.forcedToAttack = false;
		enemySwarmComp.group->inCombat = false;	

		//Set idle mid pos
		enemySwarmComp.group->idleMidPos = glm::vec3(0.0f, 0.0f, 0.0f);
		int numAlive = 0;
		for(auto b: enemySwarmComp.group->members)
		{
			if(FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(b).life > 0)
			{
				enemySwarmComp.group->idleMidPos += FSM::sceneHandler->getScene()->getComponent<Transform>(b).position;
				numAlive++;
			}
		}
		enemySwarmComp.group->idleMidPos /= numAlive;

		//Set move to
		enemySwarmComp.idleMoveTo = enemySwarmComp.group->idleMidPos;
		glm::vec3 dir = genRandomDir(glm::vec3{1.f,0.f,1.f});
		enemySwarmComp.idleMoveTo = enemySwarmComp.idleMoveTo + dir * enemySwarmComp.group->idleRadius;
		


		//Reset friction
		FSM::sceneHandler->getScene()->getComponent<Rigidbody>(entityID).friction = 1.0f;

		//Reset Combat timer
		AiCombatSwarm& combat = FSM::sceneHandler->getScene()->getComponent<AiCombatSwarm>(entityID);
		combat.timer = combat.lightAttackTime;
		//Reset scale
		FSM::sceneHandler->getScene()->getComponent<Transform>(entityID).scale.y = 1.0f;
	}

	return ret;
}

bool SwarmFSM::combat_escape(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}
	Entity playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}
	bool ShouldEscape = false;
	float swarmPlayerLen = getEntityDist(entityID, playerID);
	if (swarmPlayerLen <= enemySwarmComp.sightRadius || enemySwarmComp.group->inCombat)
	{
		ShouldEscape = true;
	}

	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());
    
	if(groupHealth < enemySwarmComp.LOW_HEALTH &&
		!enemySwarmComp.forcedToAttack &&
		ShouldEscape)
	{
		ret = true;
	}

	if(ret == true)
	{
		//Reset Combat timer
		AiCombatSwarm& combat = FSM::sceneHandler->getScene()->getComponent<AiCombatSwarm>(entityID);
		combat.timer = combat.lightAttackTime;

		//Reset scale
		FSM::sceneHandler->getScene()->getComponent<Transform>(entityID).scale.y = 1.0f;
	}

	return ret;
}

bool SwarmFSM::escape_idle(Entity entityID)
{
	bool ret = false;
	updateSwarmGrounded(entityID);
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return false;
	}

	Rigidbody& enemyRb = FSM::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
	int playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}

    float velAbs = abs(glm::length(enemyRb.velocity));
    float swarmPlayerDist = getEntityDist(entityID, playerID);
	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());

	if (swarmPlayerDist > enemySwarmComp.sightRadius &&
		groupHealth < enemySwarmComp.LOW_HEALTH &&
		velAbs > 0.001f)
    {
		ret = true;
    }

	if(ret == true)
	{
		//Set idle mid pos
		enemySwarmComp.group->idleMidPos = glm::vec3(0.0f, 0.0f, 0.0f);
		int numAlive = 0;
		for(auto b: enemySwarmComp.group->members)
		{
			if(FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(b).life > 0)
			{
				enemySwarmComp.group->idleMidPos += FSM::sceneHandler->getScene()->getComponent<Transform>(b).position;
				numAlive++;
			}
		}
		enemySwarmComp.group->idleMidPos /= numAlive;


		//Set move to
		enemySwarmComp.idleMoveTo = enemySwarmComp.group->idleMidPos;
		glm::vec3 dir = genRandomDir({1.f,0.f,1.f});
		enemySwarmComp.idleMoveTo = enemySwarmComp.idleMoveTo + dir * enemySwarmComp.group->idleRadius;

		//Reset friction
		FSM::sceneHandler->getScene()->getComponent<Rigidbody>(entityID).friction = 1.0f;

		//set scale
		FSM::sceneHandler->getScene()->getComponent<Transform>(entityID).scale.x = 1.0f;
	}

	return ret;
}

bool SwarmFSM::escape_combat(Entity entityID)
{
	bool ret = false;
	SwarmComponent& enemySwarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(enemySwarmComp.life <= 0 || !FSM::sceneHandler->getScene()->isActive(entityID))
	{
		return ret;
	}
	int playerID = getPlayerID(entityID);
	if(playerID == -1){return ret;}
	Rigidbody& enemyRb = FSM::sceneHandler->getScene()->getComponent<Rigidbody>(entityID);
	Transform& enemyTransform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
    Transform& playerTransform =FSM::sceneHandler->getScene()->getComponent<Transform>(playerID);

    float velAbs = abs(glm::length(enemyRb.velocity));
	float groupHealth = enemySwarmComp.getGroupHealth(FSM::sceneHandler->getScene());

	if(groupHealth >= enemySwarmComp.LOW_HEALTH)
	{
		ret = true;
		enemySwarmComp.group->inCombat = true;
	}
	else if(velAbs <= 0.000001f)
	{
		ret = true;
		enemySwarmComp.forcedToAttack = true;
	}

	if(ret == true)
	{
		enemyTransform.scale.x = 1.0f;
	}

	return ret;
}

bool SwarmFSM::dead(Entity entityID)
{
	bool ret = false;

	SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(swarmComp.life <= 0)
	{
		ret = true;
	}

	if(ret == true)
	{
		swarmComp.alertDone = false;
		//Reset Combat timer
		AiCombatSwarm& combat = FSM::sceneHandler->getScene()->getComponent<AiCombatSwarm>(entityID);
		combat.timer = combat.lightAttackTime;
	}

	return ret;
}

bool SwarmFSM::revive(Entity entityID)
{
	bool ret = false;

	SwarmComponent& swarmComp = sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	if(swarmComp.life > 0)
	{
		ret = true;
	}

	return ret;
}

void SwarmFSM::updateSwarmGrounded(Entity entityID)
{
	Transform& transform = FSM::sceneHandler->getScene()->getComponent<Transform>(entityID);
	Collider& collider = FSM::sceneHandler->getScene()->getComponent<Collider>(entityID);
	SwarmComponent& swarmComp = FSM::sceneHandler->getScene()->getComponent<SwarmComponent>(entityID);
	glm::vec3 posToUse = transform.position;
    static Ray downRay;
	downRay.pos = posToUse; 
	downRay.dir = glm::vec3(0.0f,-1.0f,0.0f); 
        
    float maxDist = collider.radius + 1.0f;
    RayPayload rp = FSM::sceneHandler->getPhysicsEngine()->raycast(downRay,maxDist);   

    if(rp.hit && swarmComp.groundTimer <= 0.0f)
    {
		Scene* scene = FSM::sceneHandler->getScene();
		Collider& hitCol = scene->getComponent<Collider>(rp.entity);
		float dist = glm::length(rp.hitPoint - posToUse);
		if(dist < (collider.radius + 0.5f) && !hitCol.isTrigger)
		{
			swarmComp.grounded = true;	
			swarmComp.groundTimer = swarmComp.groundTimerOrig;

			NetworkScene* s = dynamic_cast<NetworkScene*>(sceneHandler->getScene());
            if (s == nullptr)
            {
                // Spawn particle system when grounded
				if (!scene->hasComponents<ParticleSystem>(entityID))
				{
					scene->setComponent<ParticleSystem>(entityID);
					ParticleSystem& partSys = scene->getComponent<ParticleSystem>(entityID);
					partSys = ((NetworkHandlerGame*)scene->getNetworkHandler())->getSwarmParticleSystem();
					partSys.spawn = true;
				}
			}
            else
            {
                s->addEvent({(int)GameEvent::PLAY_PARTICLE, (int)ParticleTypes::SWARM, (int)entityID});
			}
			
		}
    }

	if(!swarmComp.grounded && swarmComp.groundTimer > 0.0f)
	{
		swarmComp.groundTimer -= FSM::sceneHandler->getAIHandler()->getDeltaTime();
	}
}

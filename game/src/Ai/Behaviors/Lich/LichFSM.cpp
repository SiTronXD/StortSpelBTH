#include "LichFSM.hpp"
#include "../../../Components/Combat.h"


<<<<<<<
int	LichFSM::getPlayerID()
=======

int	LichFSM::getPlayerID(Entity entityID)
>>>>>>>
{
    // if network exist take player from there
    NetworkScene* s = dynamic_cast<NetworkScene*>(sceneHandler->getScene());
    if (s != nullptr)
    {
            return s->getNearestPlayer(entityID);
    }

    // else find player from script
    int playerID = -1;
    std::string playerString = "playerID";
    FSM::sceneHandler->getScriptHandler()->getGlobal(playerID, playerString);
    return playerID;
}
float LichFSM::get_dt()
{
    return FSM::sceneHandler->getAIHandler()->getDeltaTime();
}
Scene* LichFSM::getTheScene()
{
     return FSM::sceneHandler->getScene();
}
bool LichFSM::falseIfDead(Entity entityID)
{
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.isDead())
    {return false;}
    else if(!getTheScene()->isActive(entityID))
    {return false;}
    else
    {return true;}
}

void LichFSM::updateAttackColldowns(Entity entityID)
{
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    for(auto& atck: lichComp.attacks)
    {
        if(atck.second.cooldownTimer > 0.0f)
        {
            atck.second.cooldownTimer -= get_dt();
        }
    }

   /* if(lichComp.lightning.cooldownTimer > 0.0f)
        lichComp.lightning.cooldownTimer -= get_dt();
    if(lichComp.ice.cooldownTimer > 0.0f)
        lichComp.ice.cooldownTimer -= get_dt();
    if(lichComp.fire.cooldownTimer > 0.0f)
        lichComp.fire.cooldownTimer -= get_dt();*/
}

bool LichFSM::idleToCreep(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
     updateAttackColldowns(entityID);
    int playerID = getPlayerID(entityID);    
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans   = getTheScene()->getComponent<Transform>(entityID);
    auto lichComp = getTheScene()->getComponent<LichComponent>(entityID);

    if( glm::length(playerTrans.position - lichTrans.position) < lichComp.sightRadius)
    {
        return true;
    }
    

    return false;
}

bool LichFSM::creepToAlerted(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
     updateAttackColldowns(entityID);
    bool ret = false;
    int playerID = getPlayerID(entityID);  
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans   = getTheScene()->getComponent<Transform>(entityID);
    auto lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    float dist = glm::length(playerTrans.position - lichTrans.position);
    if(dist < lichComp.peronalSpaceRadius)
    {
        ret = true;
    }    

    if(ret)
    {
         getTheScene()->getComponent<LichComponent>(entityID).alertDone = false;
    }

    return ret;
}

bool LichFSM::alertToHunt(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
     updateAttackColldowns(entityID);
    bool ret = false;

    int playerID = getPlayerID(entityID);     
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans   = getTheScene()->getComponent<Transform>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);

    //TODO: Add animation
    if(lichComp.alertDone)
    {
        ret = true;
    }


    if(ret)
    {
        lichComp.alertDone = false;
    }

    return ret;
    
}


bool LichFSM::huntToIdle(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    int playerID = getPlayerID(entityID);         
    bool ret = false;
    auto playerCombat = getTheScene()->getComponent<Combat>(playerID);
    if(playerCombat.health <= 0)
    {
        ret = true;
    }

    
    return ret;
}

bool LichFSM::huntToCombat(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}

    updateAttackColldowns(entityID);
    int playerID = getPlayerID(entityID);  
    auto playerHealth = getTheScene()->getComponent<HealthComp>(playerID);
    if(playerHealth.health > 0){return true;}

    int playerID        = getPlayerID();  
    auto playerCombat   = getTheScene()->getComponent<Combat>(playerID);
    auto playerTrans    = getTheScene()->getComponent<Transform>(playerID);
    auto lichTrans      = getTheScene()->getComponent<Transform>(entityID);
    auto lichComp       = getTheScene()->getComponent<LichComponent>(entityID);

    float dist = glm::length(playerTrans.position - lichTrans.position);
    if(playerCombat.health > 0 &&   dist <= lichComp.sightRadius)
    {
        return true;
    }

    return false;
}

bool LichFSM::escapeToCombat(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    updateAttackColldowns(entityID);
    int playerID = getPlayerID(entityID);  
    auto playerHealth = getTheScene()->getComponent<HealthComp>(playerID);
    auto lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    auto playerTrans = getTheScene()->getComponent<Transform>(playerID);
    auto lichTrans = getTheScene()->getComponent<Transform>(entityID);

    float dist = glm::length(playerTrans.position - lichTrans.position);
    if(lichComp.life > lichComp.BACK_TO_FIGHT_HEALTH && dist <= lichComp.sightRadius)
    {
       ret = true;
    }

    return ret;
}

bool LichFSM::escapeToIdle(Entity entityID)
{
   if(!falseIfDead(entityID)){return false;}
   bool ret = false;
    int playerID = getPlayerID(entityID);  
    auto lichComp       = getTheScene()->getComponent<LichComponent>(entityID);
    auto playerTrans    = getTheScene()->getComponent<Transform>(playerID);
    auto lichTrans      = getTheScene()->getComponent<Transform>(entityID);
    
    float dist = glm::length(playerTrans.position - lichTrans.position);
    if( lichComp.life == lichComp.FULL_HEALTH && dist > lichComp.sightRadius)
    {
        ret = true;
    }

    return ret;
}

bool LichFSM::combatToEscape(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    updateAttackColldowns(entityID);
    bool ret = false;

    int playerID = getPlayerID();  
    auto playerCombat   = getTheScene()->getComponent<Combat>(playerID);   

    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.life <= lichComp.ESCAPE_HEALTH && playerCombat.health > 0)
    {
        ret = true;
    }

    return ret;
}

bool LichFSM::combatToIdle(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    int playerID = getPlayerID(entityID);  
    auto playerHealth = getTheScene()->getComponent<HealthComp>(playerID);
    
    if(playerHealth.health <= 0)
    {return true;}
    
    return false;
}

bool LichFSM::combatToHunt(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    bool ret = false;
    int playerID = getPlayerID();  
    Combat playerCombat     = getTheScene()->getComponent<Combat>(playerID);
    LichComponent lichComp  = getTheScene()->getComponent<LichComponent>(entityID);
    Transform playerTrans   = getTheScene()->getComponent<Transform>(playerID);
    Transform lichTrans     = getTheScene()->getComponent<Transform>(entityID);

    float dist = glm::length(playerTrans.position - lichTrans.position);
    if(dist > lichComp.sightRadius && playerCombat.health > 0 && lichComp.life > lichComp.ESCAPE_HEALTH)
    {
        ret = true;
    }

    return ret;
}

bool LichFSM::toDead(Entity entityID)
{
    bool ret = false;
    auto lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.life <= 0)
    {
        ret = true;
    }

    return ret;
}

bool LichFSM::revive(Entity entityID)
{
     bool ret = false;

	LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);
	if(lichComp.life > 0)
	{
		ret = true;
	}

	return ret;
}
bool LichFSM::creepToIdle(Entity entityID)
{
    if (!falseIfDead(entityID))
        {
            return false;
        }
    int playerID = getPlayerID();
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans = getTheScene()->getComponent<Transform>(entityID);
    auto lichComp = getTheScene()->getComponent<LichComponent>(entityID);

    if (glm::length(playerTrans.position - lichTrans.position) >
        lichComp.sightRadius)
        {
            return true;
        }

    return false;
}

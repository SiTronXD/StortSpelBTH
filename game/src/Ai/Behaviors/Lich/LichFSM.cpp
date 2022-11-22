#include "LichFSM.hpp"
#include "../../../Components/Combat.h"

//TODO: Change this to funtions!!!!
//#define getLichComponent() FSM::sceneHandler->getScene()->getComponent<LichComponent>(entityID)
//#define getPlayerID(playerID) std::string playerId_str = "playerID";FSM::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str)
//#define getPlayerTrans(playerID) FSM::sceneHandler->getScene()->getComponent<Transform>(playerID) 
//#define getPlayerCombat(playerID) FSM::sceneHandler->getScene()->getComponent<Combat>(playerID) 
//#define getLichTrans() FSM::sceneHandler->getScene()->getComponent<Transform>(entityID) 
//#define falseIfDead() LichComponent& lichComp_____macro = getLichComponent();if(lichComp_____macro.isDead()) {return false;}


int	LichFSM::getPlayerID(Entity entityID)
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

bool LichFSM::idleToCreep(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
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

    return ret;
}

bool LichFSM::alertToHunt(Entity entityID)
{
   if(!falseIfDead(entityID)){return false;}
    int playerID = getPlayerID(entityID);     
    bool ret = false;
    Transform& playerTrans = getTheScene()->getComponent<Transform>(playerID);
    Transform& lichTrans   = getTheScene()->getComponent<Transform>(entityID);
    LichComponent& lichComp = getTheScene()->getComponent<LichComponent>(entityID);

    //TODO: Add animation
    if(lichComp.alertDone)
    {
        ret = true;
    }

    return ret;
    
}


bool LichFSM::huntToIdle(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}
    int playerID = getPlayerID(entityID);         
    auto playerCombat = getTheScene()->getComponent<Combat>(playerID);
    if(playerCombat.health <= 0){return true;}

    
    return false;
}

bool LichFSM::huntToCombat(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}

     int playerID = getPlayerID(entityID);  
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

    int playerID = getPlayerID(entityID);  
    auto playerCombat = getTheScene()->getComponent<Combat>(playerID);
    auto lichComp = getTheScene()->getComponent<LichComponent>(entityID);
    auto playerTrans = getTheScene()->getComponent<Transform>(playerID);
    auto lichTrans = getTheScene()->getComponent<Transform>(entityID);

    
    if( playerCombat.health >= lichComp.life &&
        lichComp.life > lichComp.ESCAPE_HEALTH &&  
        glm::length(playerTrans.position - lichTrans.position) <= lichComp.sightRadius)
    {return true;}

    return false;
}

bool LichFSM::escapeToIdle(Entity entityID)
{
   if(!falseIfDead(entityID)){return false;}

    int playerID = getPlayerID(entityID);  
    auto lichComp       = getTheScene()->getComponent<LichComponent>(entityID);
    auto playerTrans    = getTheScene()->getComponent<Transform>(playerID);
    auto lichTrans      = getTheScene()->getComponent<Transform>(entityID);
    
    if( lichComp.life == lichComp.FULL_HEALTH &&
        glm::length(playerTrans.position - lichTrans.position) > lichComp.sightRadius)
    {return true;}

    return false;
}

bool LichFSM::combatToIdle(Entity entityID)
{
   if(!falseIfDead(entityID)){return false;}

    int playerID = getPlayerID(entityID);  
    auto playerCombat   = getTheScene()->getComponent<Combat>(playerID);   
    
    if( playerCombat.health <= 0)
    {return true;}
    
    return false;
}

bool LichFSM::combatToHunt(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}

    int playerID = getPlayerID(entityID);  
    auto playerCombat   = getTheScene()->getComponent<Combat>(playerID);
    auto lichComp       = getTheScene()->getComponent<LichComponent>(entityID);
    auto playerTrans    = getTheScene()->getComponent<Transform>(playerID);
    auto lichTrans      = getTheScene()->getComponent<Transform>(entityID);

    if( glm::length(playerTrans.position - lichTrans.position) > lichComp.sightRadius)
    {return true;}

    return false;
}

bool LichFSM::toDead(Entity entityID)
{
    if(!falseIfDead(entityID)){return false;}

    auto lichComp       = getTheScene()->getComponent<LichComponent>(entityID);
    if(lichComp.isDead())
    {return true;}

    return false;
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

#include "LichFSM.hpp"
#include "../../../Components/Combat.h"
#define getLichComponent() FSM::sceneHandler->getScene()->getComponent<LichComponent>(entityID)
#define getPlayerID(playerID) std::string playerId_str = "playerID";FSM::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str)
#define getPlayerTrans(playerID) FSM::sceneHandler->getScene()->getComponent<Transform>(playerID) 
#define getPlayerCombat(playerID) FSM::sceneHandler->getScene()->getComponent<Combat>(playerID) 
#define getLichTrans() FSM::sceneHandler->getScene()->getComponent<Transform>(entityID) 
#define falseIfDead() LichComponent& lichComp_____macro = getLichComponent();if(lichComp_____macro.isDead()) {return false;}

bool LichFSM::idleToCreep(Entity entityID)
{
    falseIfDead();
    int playerID = -1;
    getPlayerID(playerID);    
    Transform& playerTrans = getPlayerTrans(playerID);
    Transform& lichTrans   = getLichTrans();
    auto lichComp = getLichComponent();

    if( glm::length(playerTrans.position - lichTrans.position) < lichComp.sightRadius)
    {
        return true;
    }
    

    return false;
}

bool LichFSM::creepToAlerted(Entity entityID)
{
    falseIfDead();
    int playerID = -1;
    getPlayerID(playerID);    
    Transform& playerTrans = getPlayerTrans(playerID);
    Transform& lichTrans   = getLichTrans();
    auto lichComp = getLichComponent();

    if( glm::length(playerTrans.position - lichTrans.position) < lichComp.peronalSpaceRadius)
    {
        return true;
    }    

    return false;
}

bool LichFSM::alertToHunt(Entity entityID)
{
    falseIfDead();
    int playerID = -1;
    getPlayerID(playerID);    
    Transform& playerTrans = getPlayerTrans(playerID);
    Transform& lichTrans   = getLichTrans();
    

    //TODO: Add animation

    return true;
    
}


bool LichFSM::huntToIdle(Entity entityID)
{
    falseIfDead();
    int playerID = -1;
    getPlayerID(playerID);        
    auto playerCombat = getPlayerCombat(playerID);
    if(playerCombat.health <= 0){return true;}

    
    return false;
}

bool LichFSM::huntToCombat(Entity entityID)
{
    falseIfDead();
    return false;
}

bool LichFSM::escapeToCombat(Entity entityID)
{
    falseIfDead();
    return false;
}

bool LichFSM::escapeToIdle(Entity entityID)
{
    falseIfDead();
    return false;
}

bool LichFSM::combatToIdle(Entity entityID)
{
    falseIfDead();
    return false;
}

bool LichFSM::combatToHunt(Entity entityID)
{
    falseIfDead();
    return false;
}

bool LichFSM::toDead(Entity entityID)
{
    falseIfDead();
    return false;
}

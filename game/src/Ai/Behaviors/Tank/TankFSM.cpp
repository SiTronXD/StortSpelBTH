#include "TankFSM.hpp"
#include "../../../Components/Combat.h"
#define getTankComponent() FSM::sceneHandler->getScene()->getComponent<TankComponent>(entityID)
#define getPlayerID(playerID) std::string playerId_str = "playerID";FSM::sceneHandler->getScriptHandler()->getGlobal(playerID, playerId_str)
#define getPlayerTrans(playerID) FSM::sceneHandler->getScene()->getComponent<Transform>(playerID) 
#define getPlayerCombat(playerID) FSM::sceneHandler->getScene()->getComponent<Combat>(playerID) 
#define getTankTrans() FSM::sceneHandler->getScene()->getComponent<Transform>(entityID) 
#define falseIfDead() TankComponent& tankComp_____macro = getTankComponent();if(tankComp_____macro.isDead()) {return false;}

bool TankFSM::idleToAler(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::alertToCombat(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::alertToShield(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::combatToIdel(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::combatToShield(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::shieldToCombat(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::shieldToIdle(Entity entityID)
{
    falseIfDead();
    return false;
}

bool TankFSM::toDead(Entity entityID)
{
    falseIfDead();
    return false;
}


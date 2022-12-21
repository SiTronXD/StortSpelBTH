#include "CombatFunctionProfiler.h"

CombatFunctionProfiler::CombatFunctionProfiler(const std::string& funcName) :
    m_funcName(funcName), m_started(Clock::now())
{
}

CombatFunctionProfiler::~CombatFunctionProfiler()
{
    Clock::time_point ended = Clock::now();
    Clock::duration elapsedTime = ended - m_started;
    CombatLogger::getInstance()->printData(elapsedTime, m_funcName);
}
#include "CombatLogger.h"
#include <chrono>

#define LOG_FUNCTION() CombatFunctionProfiler profiler(__FUNCTION__)
using Clock = std::chrono::high_resolution_clock;

class CombatFunctionProfiler
{
  public:
    CombatFunctionProfiler(const std::string& funcName);
    ~CombatFunctionProfiler();

  private:
    std::string m_funcName;
    Clock::time_point m_started;
};
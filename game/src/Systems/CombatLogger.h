#include <chrono>
#include <iostream>

using Clock = std::chrono::high_resolution_clock;
class CombatLogger
{
  private:
    CombatLogger(){};
    ~CombatLogger(){};

  public:
    CombatLogger(CombatLogger const&) = delete;
    CombatLogger& operator=(CombatLogger const&) = delete;

    static CombatLogger* getInstance()
    {
        static CombatLogger* instance;
        return instance;
    };

    void printData(Clock::duration data, const std::string funcName) 
    { 
        std::cout << funcName << " took " << data << std::endl; 
    };
};
#pragma once

#include "vengine.h"

struct HealthComp
{
    int health = 100;
    int maxHealth = 100;

    Entity srcDmgEntity = (Entity) -1;
};
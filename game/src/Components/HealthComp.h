#pragma once

#include "vengine.h"

struct HealthComp
{
    float health = 100.0f;
    float maxHealth = 100.0f;

    Entity srcDmgEntity = (Entity) -1;
};
#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include "glm/glm.hpp"
#include "Perks.h"
#include "Abilities.h"
#include "HealthComp.h"

enum ActiveAttack { lightActive, heavyActive, comboActive1, comboActive2, comboActive3, knockbackActive, noActive };

struct Combat
{
	// Damage for each kind of hit
    // 0 = Light Hit
    // 1 = Heavy Hit
    // 2 = Light Combo Hit
    // 3 = Mix Combo Hit
    // 4 = Heavy Combo Hit
    // 5 = Knockback Ability Hit
    float dmgArr[6] = { 50.f, 75.f, 50.f, 125.f, 150.f, 25.f };

    // Amount of time each attack takes
    float lightAttackCd = 1.f;
    float heavyAttackCd = 1.5f;
    float comboLightCd = 1.f;
    float comboHeavyCd = 1.f;
    float comboMixCd = 1.f;
    float knockbackCd = 1.f;
    float healCd = 10.f;

    // Different types of combos aviable
    std::string comboOrder;
    std::vector<std::string> combos;

    // Timers to clear combo. Too long time between attacks = Combo cleared
    float attackTimer = -1.f;
    float knockbackTimer = -1.f;
    float healTimer = 0.f;
    float comboClearTimer = 0.f;
    float comboClearDelay = 2.f;

	// Perks and their default multipliers.
	Perks perks[4];
	float hpMultiplier = 1.f;
	float staminaMultiplier = 1.f;
	float movementMultiplier = 1.f;
	float totalMoveMulti = 1.f;
	float dmgMultiplier = 1.f;
	float attackSpeedMultiplier = 1.f;
	float totalAtkSpeedMulti = 1.f;
	float animationMultiplier[6] = { 0.807f, 0.957f, 1.816f, 2.1f, 2.1f, 1.f };

    // Abilities
    Abilities ability;

    // Knockback values for each attack
    // Starting with:
    // 0 = Light Attack
    // 1 = Heavy Attack
    // 2 = Light Combo Attack
    // 3 = Mix Combo Attack
    // 4 = Heavy Combo Attack
    // 5 = Knockback Ability
    float knockbackArr[6] = { 50.f, 80.f, 120.f, 50.f, 50.f, 500.f };

    ActiveAttack activeAttack = noActive;
    ActiveAttack nextAttack = noActive;
    bool normalAttack = false;
};
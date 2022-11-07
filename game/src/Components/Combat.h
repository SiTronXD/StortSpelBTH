#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include "glm/glm.hpp"
#include "../Components/Perks.h"

enum ActiveAttack { noActive, lightActive, heavyActive, comboActive, knockbackActive };

struct Combat
{
	int maxHealth = 100;
	int health = 100;
	
	// Damage for each kind of hit
	float lightHit = 50.f;
	float heavyHit = 75.f;
	float comboLightHit = 50.f;
	float comboMixHit = 125.f;
	float comboHeavyHit = 150.f;
	float knockbackHit = 25.f;

	// Amount of time each attack takes
	float lightAttackCd = 0.5f;
	float heavyAttackCd = 1.f;
	float comboLightCd = 1.5f;
	float comboHeavyCd = 2.f;
	float comboMixCd = 2.f;
	float knockbackCd = 5.f;

	// Different types of combos aviable
	std::string comboOrder;
	std::vector<std::string> combos;

	// Timers to clear combo. Too long time between attacks = Combo cleared
	float attackTimer = 0.f;
	float knockbackTimer = 0.f;
	float comboClearTimer = 0.f;
	float comboClearDelay = 2.f;

	// Perks and their default multipliers.
	Perks perks[3];
	float hpMultiplier = 1.f;
	float dmgMultiplier = 1.f;
	float attackSpeedMultiplier = 1.f;

	// Knockback values for each attack
	float lightKnockback = 50.f;
	float heavyKnockback = 80.f;
	float comboKnockback = 120.f;
	float specialKnockback = 100.f;

	ActiveAttack activeAttack = noActive;
};
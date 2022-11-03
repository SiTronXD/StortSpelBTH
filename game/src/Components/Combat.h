#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include "glm/glm.hpp"

enum ActiveAttack { noActive, spinActive, lightActive, heavyActive, comboActive };

struct Combat
{
	float maxHealth = 100.f;
	float health = 100.f;
	
	float lightHit = 50.f;
	float heavyHit = 75.f;

	float comboLightHit = 100.f;
	float comboMixHit = 125.f;
	float comboHeavyHit = 150.f;

	float lightAttackTime = 0.5f;
	float heavyAttackTime = 1.f;

	// Probably make more combo timers
	// Depends on if the combos should take different amount of time.
	float comboLightTime = 1.5f;
	float comboHeavyTime = 2.f;
	float comboMixTime = 2.f;

	std::string comboOrder;
	std::vector<std::string> combos;

	float attackTimer = 0.f;
	float comboClearTimer = 0.f;
	float comboClearDelay = 2.f;

	float hpMultiplier = 1.f;
	float dmgMultiplier = 1.f;
	float attackSpeedMultiplier = 1.f;

	ActiveAttack activeAttack = noActive;
};
#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include "glm/glm.hpp"

enum ActiveAttack { noActive, spinActive, lightActive, heavyActive, comboActive };

struct Combat
{
	float health = 100.f;
	
	float lightHit = 1.f;
	float heavyHit = 10.f;

	float comboLightHit = 20.f;
	float comboMixHit = 30.f;
	float comboHeavyHit = 50.f;

	float lightAttackTime = 0.5f;
	float heavyAttackTime = 1.f;

	// Probably make more combo timers
	// Depends on if the combos should take different amount of time.
	float comboLightTime = 1.5f;
	float comboHeavyTime = 2.f;
	float comboMixTime = 2.f;

	std::string comboOrder;
	std::vector<std::string> combos;

	float timer = 0;

	ActiveAttack activeAttack = noActive;
};
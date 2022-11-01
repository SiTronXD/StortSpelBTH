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

	float comboLightHit = 100.f;
	float comboMixHit = 150.f;
	float comboHeavyHit = 125.f;

	float lightAttackTime = 0.5f;
	float heavyAttackTime = 1.f;

	// Probably make more combo timers
	// Depends on if the combos should take different amount of time.
	float comboAttackTime = 7.f;

	std::string comboOrder;
	std::vector<std::string> combos;

	float timer = 0;

	ActiveAttack activeAttack = noActive;
};
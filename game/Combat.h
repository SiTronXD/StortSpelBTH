#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include "glm/glm.hpp"

enum ActiveAttack { noActive, lightActive, heavyActive, comboActive };

struct Combat
{
	float health = 100.f;
	
	float lightHit = 3.f;
	float heavyHit = 5.f;

	float comboLightHit = 20.f;
	float comboMixHit = 40.f;
	float comboHeavyHit = 60.f;

	float heavyAttackTime = 5.f;
	float lightAttackTime = 3.f;

	// Probably make more combo timers
	// Depends on if the combos should take different amount of time.
	float comboAttackTime = 7.f;

	std::string comboOrder;
	std::vector<std::string> combos;

	std::chrono::time_point<std::chrono::system_clock> timer;
	std::chrono::duration<float> hitTimer;

	ActiveAttack activeAttack = noActive;
};
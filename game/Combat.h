#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include "glm/glm.hpp"

enum ActiveAttack { noActive, lightActive, heavyActive, comboActive };

struct Combat
{
	float health = 0.f;
	
	float lightHit = 0.f;
	float heavyHit = 0.f;

	float comboLightHit = 0.f;
	float comboMixHit = 0.f;
	float comboHeavyHit = 0.f;

	float heavyAttackTime = 0.f;
	float lightAttackTime = 0.f;

	// Probably make more combo timers
	// Depends on if the combos should take different amount of time.
	float comboAttackTime = 0.f;

	std::string comboOrder;
	std::vector<std::string> combos;

	std::chrono::time_point<std::chrono::system_clock> timer;
	std::chrono::duration<float> hitTimer;

	ActiveAttack activeAttack = noActive;
};
#pragma once
#include <iostream>
#include <vector>
#include <chrono>

struct AiCombat
{
	float lightHit = 0.f;
	float lightAttackTime = 0.f;
	bool attackActive = false;
	std::chrono::time_point<std::chrono::system_clock> timer;
	std::chrono::duration<float> hitTimer;
};
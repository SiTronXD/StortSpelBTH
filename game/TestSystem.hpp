#pragma once

#include "vengine.h"

class TestSystem : public System
{
private:
	float timeLeft = 5.0f;
public:
	bool update(entt::registry& reg, float deltaTime) final
	{
		Log::write("Test");
		this->timeLeft -= deltaTime;
		if (this->timeLeft <= 0.0f) { Log::write("Die"); }
		return this->timeLeft <= 0.0f;
	}
};


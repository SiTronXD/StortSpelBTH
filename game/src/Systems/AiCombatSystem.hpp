#pragma once

#include <vengine.h>
#include "../Components/AiCombat.h"

class AiCombatSystem : public System
{
private:
	Scene* scene;
public:
	AiCombatSystem(Scene* scene) : scene(scene)
	{
	}

	bool update(entt::registry& reg, float dt) final
	{
		auto view = reg.view<AiCombat>();
		auto foo = [&](AiCombat& combat)
		{
		};
		view.each(foo);

		return false;
	}

	void attack(AiCombat& combat)
	{
		if (!combat.attackActive)
		{
			combat.hitTimer = std::chrono::duration<float>(combat.lightAttackTime);
			combat.timer = std::chrono::system_clock::now();

			// Attack
		}
		else if ()
		{

		}
	}
};
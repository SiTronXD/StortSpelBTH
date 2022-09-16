#pragma once

#include <vengine.h>
#include "Combat.h"

class CombatSystem : public System
{
private:

	Scene* scene;

public:

	CombatSystem(Scene* scene)
		:scene(scene) {}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Combat>();

		auto foo = []()
		{

		};
		view.each(foo);

		view.each([&](Combat& combat)
			{

			});
	}

};
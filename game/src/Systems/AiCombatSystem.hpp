#pragma once

#include <vengine.h>
#include "../Components/AiCombat.h"

class AiCombatSystem : public System
{
private:
	Scene* scene;
public:
	AiCombatSystem(Scene* scene, int ID)
		:scene(scene)
	{
		if (scene->hasComponents<AiCombat>(ID))
		{
			AiCombat& combat = scene->getComponent<AiCombat>(ID);
			combat.normalAttack = 10.f;
		}
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

	}
};
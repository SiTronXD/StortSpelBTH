#pragma once

#include <vengine.h>
#include "../Components/AiCombat.h"
#include "../Components/AiMovement.h"
#include "../Components/Combat.h"

class AiCombatSystem : public System
{
private:
	Scene* scene;
	SceneHandler* sceneHandler;
	int playerID = -1;
public:
	AiCombatSystem(SceneHandler* sceneHandler, int playerID) : sceneHandler(sceneHandler), playerID(playerID)
	{
		scene = sceneHandler->getScene();
	}

	bool update(entt::registry& reg, float dt) final
	{
		auto view = reg.view<AiCombat, AiMovement>();
		auto foo = [&](AiCombat& combat, AiMovement& movement)
		{
			if (combat.timer > 0.f)
			{
				combat.timer -= dt;
			}
			else if (movement.distance <= 10.f)
			{
				attack(combat, movement);
			}
		};
		view.each(foo);

		return false;
	}

	void attack(AiCombat& combat, AiMovement& movement)
	{
		Combat& playerCombat = scene->getComponent<Combat>(playerID);
		playerCombat.health -= combat.lightHit;
		combat.timer = combat.lightAttackTime;
	}
};
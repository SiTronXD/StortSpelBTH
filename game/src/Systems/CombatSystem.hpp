#pragma once

#include <vengine.h>
#include "../Components/Combat.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include <string>

class CombatSystem : public System
{
private:

	Scene* scene;
	Entity playerID;

public:

	CombatSystem(Scene* scene, Entity playerID)
		: scene(scene), playerID(playerID)
	{
		if (scene->hasComponents<Combat>(playerID))
		{
			Combat& combat = scene->getComponent<Combat>(playerID);
			combat.combos.emplace_back("Light Light Light ");
			combat.combos.emplace_back("Light Heavy Light ");
			combat.combos.emplace_back("Heavy Light Heavy ");
		}
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Combat>();
		auto foo = [&](Combat& combat)
		{
			if (combat.timer > -1.f)
			{
				combat.timer -= deltaTime;
			}

			if (Input::isMouseButtonPressed(Mouse::LEFT))
			{
				lightAttack(combat);
			}
			else if (Input::isMouseButtonPressed(Mouse::RIGHT))
			{
				heavyAttack(combat);
			}
		};
		view.each(foo);

		return false;
	}

	float& getHealth(Combat& combat)
	{
		return combat.health;
	};

	void checkActiveAttack(Combat& combat)
	{
		switch (combat.activeAttack)
		{
		case lightActive:
			// If it takes too long between attacks, resets combo.
			if (combat.timer > 1.f)
			{
				combat.activeAttack = noActive;
				combat.comboOrder.clear();
			}
			else if (combat.timer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			break;
		case heavyActive:
			// If it takes too long between attacks, resets combo.
			if (combat.timer > 1.f)
			{
				combat.activeAttack = noActive;
				combat.comboOrder.clear();
			}
			else if (combat.timer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			break;
		case comboActive:
			if (combat.timer < 0.f)
			{
				combat.activeAttack = noActive;
			}
		}
	};

	bool lightAttack(Combat& combat)
	{
		checkActiveAttack(combat);

		if (combat.activeAttack == noActive)
		{
			combat.timer = combat.lightAttackTime;
			combat.comboOrder.append("Light ");

			// If combo is lhh there can be no combo, combo is reset.
			if (combat.comboOrder == "Light Heavy Heavy ")
			{
				combat.comboOrder.clear();
			}

			combat.activeAttack = lightActive;

			if (checkCombo(combat))
			{
				return true;
			}
			else
			{
				combat.health -= combat.lightHit;
				return true;
			}
		}
		return false;
	};

	bool heavyAttack(Combat& combat)
	{
		checkActiveAttack(combat);

		if (combat.activeAttack == noActive)
		{
			combat.timer = combat.heavyAttackTime;
			combat.comboOrder.append("Heavy ");

			// If combo starts with Heavy Heavy , there can be no combo, combo is reset.
			if (combat.comboOrder == "Heavy Heavy ") { combat.comboOrder.clear(); }

			combat.activeAttack = heavyActive;

			if (checkCombo(combat)) { return true; }
			else
			{
				combat.health -= combat.heavyHit;
				return true;
			}
		}

		return false;
	};

	// Executes combo attack.
	void comboAttack(Combat& combat, int idx)
	{
		if (idx == 0)
		{
			combat.timer = combat.comboLightTime;
			combat.health -= combat.comboLightHit;
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 1)
		{
			combat.timer = combat.comboMixTime;
			combat.health -= combat.comboMixHit;
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 2)
		{
			combat.timer = combat.comboHeavyTime;
			combat.health -= combat.comboHeavyHit;
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
	};

	// Check if there is a combo and if it is, execute the combo.
	bool checkCombo(Combat& combat)
	{
		for (int i = 0; i < combat.combos.size(); i++)
		{
			if (combat.comboOrder.c_str() == combat.combos[i])
			{
				comboAttack(combat, i);
				return true;
			}
		}
		return false;
	};

};
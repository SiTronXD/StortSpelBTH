#pragma once

#include <vengine.h>
#include "Combat.h"
#include <string>

class CombatSystem : public System
{
private:

	Scene* scene;
	float gotHit = true;

public:

	CombatSystem(Scene* scene, int playerID)
		:scene(scene) 
	{
		if (scene->hasComponents<Combat>(playerID))
		{
			scene->getComponent<Combat>(playerID).combos.emplace_back("Light Light ");
			scene->getComponent<Combat>(playerID).combos.emplace_back("Light Heavy Light ");
			scene->getComponent<Combat>(playerID).combos.emplace_back("Heavy Light Heavy ");
		}
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Combat>();
		auto foo = [&](Combat& combat)
		{
			if (Input::isMouseButtonPressed(Mouse::LEFT) && gotHit == true)
			{
				lightAttack(combat, gotHit);
			}
			else if (Input::isMouseButtonPressed(Mouse::RIGHT) && gotHit == true)
			{
				heavyAttack(combat, gotHit);
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
		combat.hitTimer = std::chrono::system_clock::now() - combat.timer;

		switch (combat.activeAttack)
		{
		case lightActive:
			// If it takes too long between attacks, resets combo.
			if (combat.hitTimer.count() > combat.lightAttackTime + 2.f)
			{
				combat.activeAttack = noActive;
				combat.comboOrder.clear();
			}
			else if (combat.hitTimer.count() > combat.lightAttackTime)
			{
				combat.activeAttack = noActive;
			}
			else
			{
				std::cout << "Light attack already in use\n";
				Log::write(std::to_string(combat.hitTimer.count()) + " Over 3 seconds and you can attack again\n" + combat.comboOrder + "\n\n");
			}
			break;
		case heavyActive:
			// If it takes too long between attacks, resets combo.
			if (combat.hitTimer.count() > combat.heavyAttackTime + 2.f)
			{
				combat.activeAttack = noActive;
				combat.comboOrder.clear();
			}
			else if (combat.hitTimer.count() > combat.heavyAttackTime)
			{
				combat.activeAttack = noActive;
			}
			else
			{
				std::cout << "Heavy attack already in use\n";
				Log::write(std::to_string(combat.hitTimer.count()) + " Over 5 seconds and you can attack again\n" + "\n\n");
			}
			break;
		case comboActive:
			if (combat.hitTimer.count() > combat.comboAttackTime)
			{
				combat.activeAttack = noActive;
			}
			else
			{
				std::cout << "Combo attack already in use\n";
				Log::write(std::to_string(combat.hitTimer.count()) + " Over 7 seconds and you can attack again\n" + "\n\n");
			}
		}
	};

	bool lightAttack(Combat& combat, bool gotHit)
	{
		checkActiveAttack(combat);

		if (combat.activeAttack == noActive)
		{
			combat.hitTimer = std::chrono::duration<float>(combat.lightAttackTime);
			combat.timer = std::chrono::system_clock::now();
			combat.comboOrder.append("Light ");

			// If combo is lhh there can be no combo, combo is reset.
			if (combat.comboOrder == "Light Heavy Heavy ")
			{
				combat.comboOrder.clear();
			}

			combat.activeAttack = lightActive;

			if (gotHit)
			{
				if (checkCombo(combat))
				{
					return true;
				}
				else
				{
					combat.health -= combat.lightHit;
					std::cout << combat.comboOrder + "\nYou hit the enemy! New health is " + std::to_string(combat.health) + "\n\n";
					return true;

				}
			}
		}
		return false;
	};
	bool heavyAttack(Combat& combat, bool gotHit)
	{
		checkActiveAttack(combat);

		if (combat.activeAttack == noActive)
		{
			combat.hitTimer = std::chrono::duration<float>(combat.heavyAttackTime);
			combat.timer = std::chrono::system_clock::now();
			combat.comboOrder.append("Heavy ");

			// If combo starts with hh, there can be no combo, combo is reset.
			if (combat.comboOrder == "Heavy Heavy ")
			{
				combat.comboOrder.clear();
			}

			combat.activeAttack = heavyActive;

			if (gotHit)
			{
				if (checkCombo(combat))
				{
					return true;
				}
				else
				{
					combat.health -= combat.heavyHit;
					std::cout << combat.comboOrder + "\nYou hit the enemy! New health is " + std::to_string(combat.health) + "\n\n";
					return true;
				}
			}
		}

		return false;
	};

	// Executes combo attack.
	void comboAttack(Combat& combat, int idx)
	{
		if (idx == 0)
		{
			combat.health -= combat.comboLightHit;
			std::cout << "You hit the enemy with a light combo! New health is " + std::to_string(combat.health) + "\n\n";
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 1)
		{
			combat.health -= combat.comboMixHit;
			std::cout << "You hit the enemy with a mix combo! New health is " + std::to_string(combat.health) + "\n\n";
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 2)
		{
			combat.health -= combat.comboHeavyHit;
			std::cout << "You hit the enemy with a heavy combo! New health is " + std::to_string(combat.health) + "\n\n";
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
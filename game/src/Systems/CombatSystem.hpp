#pragma once

#include <vengine.h>
#include "../Components/Combat.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include <string>

class CombatSystem : public System
{
private:

	Scene* scene;
	bool gotHit = true;
	Entity playerID;
	int standardAnim;
	int spinningAnim;

public:

	CombatSystem(Scene* scene, Entity playerID, int standardAnim, int spinningAnim)
		: scene(scene), playerID(playerID), standardAnim(standardAnim), spinningAnim(spinningAnim)
	{
		if (scene->hasComponents<Combat>(playerID))
		{
			Combat& combat = scene->getComponent<Combat>(playerID);
			combat.combos.emplace_back("Light Light ");
			combat.combos.emplace_back("Light Heavy Light ");
			combat.combos.emplace_back("Heavy Light Heavy ");
		}
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Combat>();
		auto foo = [&](Combat& combat)
		{
			if (Input::isMouseButtonPressed(Mouse::LEFT))
			{
				//lightAttack(combat, gotHit);
				spinAttack(reg, combat);
			}
			//else if (Input::isMouseButtonPressed(Mouse::RIGHT) && gotHit == true)
			//{
			//	heavyAttack(combat, gotHit);
			//}
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
		case spinActive:
			if (combat.hitTimer.count() > combat.spinAttackTime + 1.f)
			{
				combat.activeAttack = noActive;
			}
			else
			{
				scene->setComponent<MeshComponent>(this->playerID, this->standardAnim);
			}
			break;
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
			break;
		case comboActive:
			if (combat.hitTimer.count() > combat.comboAttackTime)
			{
				combat.activeAttack = noActive;
			}
		}
	};

	bool spinAttack(entt::registry& reg, Combat& combat)
	{
		checkActiveAttack(combat);

		if (combat.activeAttack == noActive)
		{
			combat.hitTimer = std::chrono::duration<float>(combat.spinAttackTime);
			combat.timer = std::chrono::system_clock::now();

			scene->getComponent<MeshComponent>(this->playerID).meshID = this->spinningAnim;
			auto view = reg.view<SwarmComponent, Transform>();
			auto foo = [&](SwarmComponent& swarm, Transform& swarmTrans)
			{
				Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
				float distance = glm::length(playerTrans.position - swarmTrans.position);
				if (distance <= 15.f)
				{
					swarm.life -= combat.spinHit;
					return true;
				}
			};
			view.each(foo);
		}

		return false;
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

			// If combo starts with Heavy Heavy , there can be no combo, combo is reset.
			if (combat.comboOrder == "Heavy Heavy ") { combat.comboOrder.clear(); }

			combat.activeAttack = heavyActive;

			if (gotHit)
			{
				if (checkCombo(combat)) { return true; }
				else
				{
					combat.health -= combat.heavyHit;
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
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 1)
		{
			combat.health -= combat.comboMixHit;
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 2)
		{
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
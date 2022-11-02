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
	Collider sword;
	Collider swordSpin;
	PhysicsEngine* physics;
	DebugRenderer* debug;

public:

	CombatSystem(Scene* scene, Entity playerID, PhysicsEngine* physics, DebugRenderer* debug)
		: scene(scene), playerID(playerID), physics(physics), debug(debug)
	{
		if (scene->hasComponents<Combat>(playerID))
		{
			Combat& combat = scene->getComponent<Combat>(playerID);
			combat.combos.emplace_back("Light Light Light ");
			combat.combos.emplace_back("Light Heavy Light ");
			combat.combos.emplace_back("Heavy Light Heavy ");
			sword = Collider::createCapsule(1.f, 6.f, true);
			swordSpin = Collider::createCapsule(10.f, 0.1f, true);
		}
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Combat>();
		auto foo = [&](Combat& combat)
		{
			Transform& playerTrans = scene->getComponent<Transform>(playerID);
			playerTrans.updateMatrix();
			debug->renderCapsule(playerTrans.position, glm::vec3(0.f, 0.f, 0.f), 0.1f, 10.f, glm::vec3(0.5f, 0.5f, 0.5f));
			if (combat.attackTimer > -1.f)
			{
				combat.attackTimer -= deltaTime;
			}
			if (combat.comboClearTimer > -1.f)
			{
				combat.comboClearTimer -= deltaTime;
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
			if (combat.comboClearTimer <= 0.f)
			{
				combat.activeAttack = noActive;
				combat.comboOrder.clear();
			}
			else if (combat.attackTimer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			break;
		case heavyActive:
			// If it takes too long between attacks, resets combo.
			if (combat.comboClearTimer <= 0.f)
			{
				combat.activeAttack = noActive;
				combat.comboOrder.clear();
			}
			else if (combat.attackTimer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			break;
		case comboActive:
			if (combat.attackTimer < 1.f)
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
			combat.attackTimer = combat.lightAttackTime;
			combat.comboClearTimer = combat.comboClearDelay;
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
				Transform& playerTrans = scene->getComponent<Transform>(playerID);
				playerTrans.updateMatrix();
				std::vector<int> hitID = physics->testContact(sword, playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));
				for (size_t i = 0; i < hitID.size(); i++)
				{
					if (scene->hasComponents<SwarmComponent>(hitID[i]))
					{
						SwarmComponent& enemy = scene->getComponent<SwarmComponent>(hitID[i]);
						enemy.life -= (int)combat.lightHit;
						return true;
					}
				}
			}
		}
		return false;
	};

	bool heavyAttack(Combat& combat)
	{
		checkActiveAttack(combat);

		if (combat.activeAttack == noActive)
		{
			combat.attackTimer = combat.heavyAttackTime;
			combat.comboClearTimer = combat.comboClearDelay;
			combat.comboOrder.append("Heavy ");

			// If combo starts with Heavy Heavy , there can be no combo, combo is reset.
			if (combat.comboOrder == "Heavy Heavy ") { combat.comboOrder.clear(); }

			combat.activeAttack = heavyActive;

			if (checkCombo(combat)) { return true; }
			else
			{
				Transform& playerTrans = scene->getComponent<Transform>(playerID);
				playerTrans.updateMatrix();
				std::vector<int> hitID = physics->testContact(sword, playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));
				for (size_t i = 0; i < hitID.size(); i++)
				{
					if (scene->hasComponents<SwarmComponent>(hitID[i]))
					{
						SwarmComponent& enemy = scene->getComponent<SwarmComponent>(hitID[i]);
						enemy.life -= (int)combat.heavyHit;
						return true;
					}
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
			combat.attackTimer = combat.comboLightTime;
			Transform& playerTrans = scene->getComponent<Transform>(playerID);
			playerTrans.updateMatrix();
			std::vector<int> hitID = physics->testContact(swordSpin, playerTrans.position, glm::vec3(0.f, 0.f, 0.f));
			for (size_t i = 0; i < hitID.size(); i++)
			{
				if (scene->hasComponents<SwarmComponent>(hitID[i]))
				{
					SwarmComponent& enemy = scene->getComponent<SwarmComponent>(hitID[i]);
					enemy.life -= (int)combat.comboLightHit;
				}
			}
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 1)
		{
			combat.attackTimer = combat.comboMixTime;
			Transform& playerTrans = scene->getComponent<Transform>(playerID);
			playerTrans.updateMatrix();
			std::vector<int> hitID = physics->testContact(sword, playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));
			for (size_t i = 0; i < hitID.size(); i++)
			{
				if (scene->hasComponents<SwarmComponent>(hitID[i]))
				{
					SwarmComponent& enemy = scene->getComponent<SwarmComponent>(hitID[i]);
					enemy.life -= (int)combat.comboMixHit;
				}
			}
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 2)
		{
			combat.attackTimer = combat.comboHeavyTime;
			Transform& playerTrans = scene->getComponent<Transform>(playerID);
			playerTrans.updateMatrix();
			std::vector<int> hitID = physics->testContact(sword, playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));
			for (size_t i = 0; i < hitID.size(); i++)
			{
				if (scene->hasComponents<SwarmComponent>(hitID[i]))
				{
					SwarmComponent& enemy = scene->getComponent<SwarmComponent>(hitID[i]);
					enemy.life -= (int)combat.comboMixHit;
				}
			}
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
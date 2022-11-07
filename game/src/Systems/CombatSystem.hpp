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
			sword = Collider::createCapsule(1.f, 6.f, glm::vec3(0,0,0), true);
            swordSpin =
                Collider::createCapsule(10.f, 0.1f, glm::vec3(0, 0, 0), true);
			
			for (size_t i = 0; i < 3; i++)
			{
				combat.perks[i].multiplier = 0;
				combat.perks[i].perkType = empty;
			}
		}
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Combat>();
		auto foo = [&](Combat& combat)
		{
			checkPerkCollision(combat);
			decreaseTimers(combat, deltaTime);

			// Check if player is trying to attack
			if (Input::isMouseButtonPressed(Mouse::LEFT))
			{
				lightAttack(combat);
			}
			else if (Input::isMouseButtonPressed(Mouse::RIGHT))
			{
				heavyAttack(combat);
			}
			else if (Input::isKeyPressed(Keys::F))
			{
				knockbackAttack(combat);
			}
			// Check if player wants to drop a perk
			if (Input::isKeyPressed(Keys::ONE))
			{
				removePerk(combat, combat.perks[0]);
			}
			if (Input::isKeyPressed(Keys::TWO))
			{
				removePerk(combat, combat.perks[1]);
			}
			if (Input::isKeyPressed(Keys::THREE))
			{
				removePerk(combat, combat.perks[2]);
			}
		};
		view.each(foo);

		return false;
	}

	int& getHealth(Combat& combat)
	{
		return combat.health;
	};

	ActiveAttack checkActiveAttack(Combat& combat)
	{
		switch (combat.activeAttack)
		{
		case noActive:
			return combat.activeAttack;
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
			return combat.activeAttack;
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
			return combat.activeAttack;
		case comboActive:
			if (combat.attackTimer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case knockbackActive:
			if (combat.knockbackTimer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		}
	};

	bool lightAttack(Combat& combat)
	{
		if (checkActiveAttack(combat) == noActive)
		{
			combat.attackTimer = combat.lightAttackCd;
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
				Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
				playerTrans.updateMatrix();
				std::vector<int> hitID = physics->testContact(this->sword, playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));
				for (size_t i = 0; i < hitID.size(); i++)
				{
					if (scene->hasComponents<SwarmComponent>(hitID[i]))
					{
						SwarmComponent& enemy = this->scene->getComponent<SwarmComponent>(hitID[i]);
						enemy.life -= (int)combat.lightHit;
						Rigidbody& enemyRB = this->scene->getComponent<Rigidbody>(hitID[i]);
						Transform& enemyTrans = this->scene->getComponent<Transform>(hitID[i]);
						glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
						enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * combat.lightKnockback;
					}
				}
				return true;
			}
		}
		return false;
	};

	bool heavyAttack(Combat& combat)
	{
		if (checkActiveAttack(combat) == noActive)
		{
			combat.attackTimer = combat.heavyAttackCd;
			combat.comboClearTimer = combat.comboClearDelay;
			combat.comboOrder.append("Heavy ");

			// If combo starts with Heavy Heavy , there can be no combo, combo is reset.
			if (combat.comboOrder == "Heavy Heavy ") { combat.comboOrder.clear(); }

			combat.activeAttack = heavyActive;

			if (checkCombo(combat)) { return true; }
			else
			{
				Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
				playerTrans.updateMatrix();
				std::vector<int> hitID = physics->testContact(this->sword, playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));
				for (size_t i = 0; i < hitID.size(); i++)
				{
					if (scene->hasComponents<SwarmComponent>(hitID[i]))
					{
						SwarmComponent& enemy = this->scene->getComponent<SwarmComponent>(hitID[i]);
						enemy.life -= (int)combat.heavyHit;
						Rigidbody& enemyRB = this->scene->getComponent<Rigidbody>(hitID[i]);
						Transform& enemyTrans = this->scene->getComponent<Transform>(hitID[i]);
						glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
						enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * combat.heavyKnockback;
					}
				}
				return true;
			}
		}

		return false;
	};

	bool knockbackAttack(Combat& combat)
	{
		if (checkActiveAttack(combat) == noActive)
		{
			combat.knockbackTimer = combat.knockbackCd;
			combat.activeAttack = knockbackActive;

			if (checkCombo(combat)) { return true; }
			else
			{
				Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
				playerTrans.updateMatrix();
				std::vector<int> hitID = physics->testContact(this->swordSpin, playerTrans.position, glm::vec3(0.f));
				for (size_t i = 0; i < hitID.size(); i++)
				{
					if (scene->hasComponents<SwarmComponent>(hitID[i]))
					{
						SwarmComponent& enemy = this->scene->getComponent<SwarmComponent>(hitID[i]);
						enemy.life -= (int)combat.knockbackHit;
						Rigidbody& enemyRB = this->scene->getComponent<Rigidbody>(hitID[i]);
						Transform& enemyTrans = this->scene->getComponent<Transform>(hitID[i]);
						glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
						enemyRB.velocity = glm::vec3(-newDir.x, 0.2f, -newDir.z) * combat.specialKnockback;
					}
				}
				return true;
			}
		}

		return false;
	}

	// Executes combo attack.
	void comboAttack(Combat& combat, int idx)
	{
		if (idx == 0)
		{
			combat.attackTimer = combat.comboLightCd;
			Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
			playerTrans.updateMatrix();
			std::vector<int> hitID = physics->testContact(this->swordSpin, playerTrans.position, glm::vec3(0.f));
			for (size_t i = 0; i < hitID.size(); i++)
			{
				if (scene->hasComponents<SwarmComponent>(hitID[i]))
				{
					SwarmComponent& enemy = this->scene->getComponent<SwarmComponent>(hitID[i]);
					enemy.life -= (int)combat.comboLightHit;
					Rigidbody& enemyRB = this->scene->getComponent<Rigidbody>(hitID[i]);
					Transform& enemyTrans = this->scene->getComponent<Transform>(hitID[i]);
					glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
					enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * combat.comboKnockback;
				}
			}
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 1)
		{
			combat.attackTimer = combat.comboMixCd;
			Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
			playerTrans.updateMatrix();
			std::vector<int> hitID = physics->testContact(this->sword, playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));
			for (size_t i = 0; i < hitID.size(); i++)
			{
				if (scene->hasComponents<SwarmComponent>(hitID[i]))
				{
					SwarmComponent& enemy = this->scene->getComponent<SwarmComponent>(hitID[i]);
					enemy.life -= (int)combat.comboHeavyHit;
					Rigidbody& enemyRB = this->scene->getComponent<Rigidbody>(hitID[i]);
					Transform& enemyTrans = this->scene->getComponent<Transform>(hitID[i]);
					glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
					enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * combat.comboKnockback;
				}
			}
			combat.comboOrder.clear();
			combat.activeAttack = comboActive;
		}
		else if (idx == 2)
		{
			combat.attackTimer = combat.comboHeavyCd;
			Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
			playerTrans.updateMatrix();
			std::vector<int> hitID = physics->testContact(sword, playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));
			for (size_t i = 0; i < hitID.size(); i++)
			{
				if (scene->hasComponents<SwarmComponent>(hitID[i]))
				{
					SwarmComponent& enemy = this->scene->getComponent<SwarmComponent>(hitID[i]);
					enemy.life -= (int)combat.comboMixHit;
					Rigidbody& enemyRB = this->scene->getComponent<Rigidbody>(hitID[i]);
					Transform& enemyTrans = this->scene->getComponent<Transform>(hitID[i]);
					glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
					enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * combat.comboKnockback;
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

	void updateHealth(Combat& combat, Perks& perk, bool doUpgrade = true)
	{
		if (doUpgrade)
		{
			setDefaultHp(combat);
			combat.hpMultiplier += perk.multiplier;
		}
		float tempHp = (float)combat.maxHealth * combat.hpMultiplier;
		combat.maxHealth = (int)tempHp;
	}

	void updateDmg(Combat& combat, Perks& perk, bool doUpgrade = true)
	{
		if (doUpgrade)
		{
			setDefaultDmg(combat);
			combat.dmgMultiplier += perk.multiplier;
		}
		combat.lightHit *= combat.dmgMultiplier;
		combat.heavyHit *= combat.dmgMultiplier;
		combat.comboLightHit *= combat.dmgMultiplier;
		combat.comboHeavyHit *= combat.dmgMultiplier;
		combat.comboMixHit *= combat.dmgMultiplier;
	}

	void updateAttackSpeed(Combat& combat, Perks& perk, bool doUpgrade = true)
	{
		if (doUpgrade)
		{
			setDefaultAtttackSpeed(combat);
			combat.attackSpeedMultiplier -= perk.multiplier;
		}
		combat.lightAttackCd *= combat.attackSpeedMultiplier;
		combat.heavyAttackCd *= combat.attackSpeedMultiplier;
		combat.comboLightCd *= combat.attackSpeedMultiplier;
		combat.comboHeavyCd *= combat.attackSpeedMultiplier;
		combat.comboMixCd *= combat.attackSpeedMultiplier;
		combat.knockbackCd *= combat.attackSpeedMultiplier;
	}

	void setDefaultHp(Combat& combat)
	{
		float tempHp = (float)combat.maxHealth / combat.hpMultiplier;
		combat.maxHealth = (int)tempHp;
		combat.health = std::min(combat.health, combat.maxHealth);
	}

	void setDefaultDmg(Combat& combat)
	{
		combat.lightHit /= combat.dmgMultiplier;
		combat.heavyHit /= combat.dmgMultiplier;
		combat.comboLightHit /= combat.dmgMultiplier;
		combat.comboHeavyHit /= combat.dmgMultiplier;
		combat.comboMixHit /= combat.dmgMultiplier;
		combat.knockbackHit /= combat.dmgMultiplier;
	}

	void setDefaultAtttackSpeed(Combat& combat)
	{
		combat.lightAttackCd /= combat.attackSpeedMultiplier;
		combat.heavyAttackCd /= combat.attackSpeedMultiplier;
		combat.comboLightCd /= combat.attackSpeedMultiplier;
		combat.comboHeavyCd /= combat.attackSpeedMultiplier;
		combat.comboMixCd /= combat.attackSpeedMultiplier;
		combat.knockbackCd /= combat.attackSpeedMultiplier;
	}

	void removePerk(Combat& combat, Perks& perk)
	{
		if (perk.perkType != empty)
		{
			switch (perk.perkType)
			{
			case hpUp:
				setDefaultHp(combat);
				combat.hpMultiplier -= perk.multiplier;
				updateHealth(combat, perk, false);
				break;
			case dmgUp:
				setDefaultDmg(combat);
				combat.dmgMultiplier -= perk.multiplier;
				updateDmg(combat, perk, false);
				break;
			case attackSpeedUp:
				setDefaultAtttackSpeed(combat);
				combat.attackSpeedMultiplier += perk.multiplier;
				updateAttackSpeed(combat, perk, false);
			}
			perk.multiplier = 0;
			perk.perkType = empty;
		}
	}

	void checkPerkCollision(Combat& combat)
	{
		Collider& playerColl = scene->getComponent<Collider>(playerID);
		Transform& playerTrans = scene->getComponent<Transform>(playerID);
		std::vector<int> hitID = physics->testContact(playerColl, playerTrans.position, playerTrans.rotation);
		for (size_t i = 0; i < hitID.size(); i++)
		{
			if (scene->hasComponents<Perks>(hitID[i]))
			{
				Perks& perk = scene->getComponent<Perks>(hitID[i]);
				for (size_t j = 0; j < 3; j++)
				{
					if (combat.perks[j].perkType == empty)
					{
						combat.perks[j] = perk;
						switch (combat.perks[j].perkType)
						{
						case hpUp:
							updateHealth(combat, combat.perks[j]);
							break;
						case dmgUp:
							updateDmg(combat, combat.perks[j]);
							break;
						case attackSpeedUp:
							updateAttackSpeed(combat, combat.perks[j]);
							break;
						}
						j = 3;
					}
				}
				scene->removeEntity(hitID[i]);
			}
		}
	}

	void decreaseTimers(Combat& combat, float deltaTime) 
	{
		if (combat.attackTimer > -1.f)
		{
			combat.attackTimer -= deltaTime;
		}
		if (combat.knockbackTimer > -1.f)
		{
			combat.knockbackTimer -= deltaTime;
		}
		if (combat.comboClearTimer > -1.f)
		{
			combat.comboClearTimer -= deltaTime;
		}
	}
};
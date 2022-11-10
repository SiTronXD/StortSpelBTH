#pragma once

#include <vengine.h>
#include "../Components/Combat.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include <string>

class CombatSystem : public System
{
private:

	Scene* scene;
	ResourceManager* resourceMng;
	Entity playerID;
	Collider sword;
	Collider swordSpin;
	PhysicsEngine* physics;
	UIRenderer* uiRenderer;
	DebugRenderer* debug;

	int perkMeshes[3];
	int abilityMeshes[1];
	int healingMesh;
	Entity heal = -1;

public:

	CombatSystem(Scene* scene, ResourceManager* resourceMng, Entity playerID, PhysicsEngine* physics, UIRenderer* uiRenderer, DebugRenderer* debug)
		: scene(scene), resourceMng(resourceMng), playerID(playerID), physics(physics), uiRenderer(uiRenderer), debug(debug)
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
			
			combat.ability.abilityType = emptyAbility;
			for (size_t i = 0; i < 4; i++)
			{
				combat.perks[i].multiplier = 0;
				combat.perks[i].perkType = emptyPerk;
			}
			perkMeshes[0] = this->resourceMng->addMesh("assets/models/Perk_Hp.obj");
			perkMeshes[1] = this->resourceMng->addMesh("assets/models/Perk_Dmg.obj");
			perkMeshes[2] = this->resourceMng->addMesh("assets/models/Perk_AtkSpeed.obj");
			abilityMeshes[0] = this->resourceMng->addMesh("assets/models/KnockbackAbility.obj");
			healingMesh = this->resourceMng->addMesh("assets/models/HealingAbility.obj");
		}
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Combat>();
		auto foo = [&](Combat& combat)
		{
			checkPerkCollision(combat);
			checkAbilityCollision(combat);
			decreaseTimers(combat, deltaTime);

			if (combat.isHealing)
			{
				Transform& healTrans = this->scene->getComponent<Transform>(this->heal);
				Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
				if (combat.health < combat.maxHealth)
				{
					float dist = glm::length(healTrans.position - playerTrans.position);
					if (dist < combat.healRadius)
					{
						float newHealth = combat.hpRegen * deltaTime;
						combat.health += (int)newHealth;
					}
				}
			}

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
				useAbility(combat);
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
			if (Input::isKeyPressed(Keys::FOUR))
			{
				removePerk(combat, combat.perks[3]);
			}
			if (Input::isKeyPressed(Keys::FIVE))
			{
				removeAbility(combat, combat.ability);
			}
		};
		view.each(foo);

		return false;
	}

	int getHealth(Combat& combat)
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

	bool useAbility(Combat& combat)
	{
		if (combat.ability.abilityType == knockbackAbility)
		{
			useKnockbackAbility(combat);
			return true;
		}
		else if (combat.ability.abilityType == healAbility)
		{
			useHealingAbility(combat);
			return true;
		}
		return false;
	}

	bool useKnockbackAbility(Combat& combat)
	{
		if (combat.ability.abilityType == knockbackAbility)
		{
			if (checkActiveAttack(combat) == noActive)
			{
				combat.knockbackTimer = combat.knockbackCd;
				combat.activeAttack = knockbackActive;
				combat.ability.abilityType = emptyAbility;

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
		}
		return false;
	}

	bool useHealingAbility(Combat& combat)
	{
		if (combat.ability.abilityType == healAbility)
		{
			if (checkActiveAttack(combat) == noActive)
			{
				combat.healTimer = combat.healCd;
				combat.isHealing = true;
				combat.ability.abilityType = emptyAbility;

				this->heal = this->scene->createEntity();
				this->scene->setComponent<MeshComponent>(this->heal, this->healingMesh);
				Transform& healTrans = this->scene->getComponent<Transform>(this->heal);
				Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
				healTrans.position = glm::vec3(playerTrans.position.x, 0.f, playerTrans.position.z);

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
		combat.health = std::min(combat.health, combat.maxHealth);
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
		combat.maxHealth = (int)(tempHp + 0.5f);
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

	void setupPerkSpawn(Entity& entity, Perks& perk)
	{
		this->scene->setComponent<Collider>(entity, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
		this->scene->setComponent<Rigidbody>(entity);
		this->scene->setComponent<Perks>(entity, perk);
		Transform& perkTrans = this->scene->getComponent<Transform>(entity);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		Rigidbody& perkRb = this->scene->getComponent<Rigidbody>(entity);
		perkTrans.position = playerTrans.position;
		perkTrans.scale = glm::vec3(2.f, 2.f, 2.f);
		playerTrans.updateMatrix();
		glm::vec3 throwDir = glm::normalize(playerTrans.forward());
		perkRb.gravityMult = 6.f;
		perkRb.velocity = glm::vec3(throwDir.x * 20.f, 30.f, throwDir.z * 20.f);
	}

	void spawnPerk(Perks& perk)
	{
		switch (perk.perkType)
		{
		case hpUpPerk:
		{
			Entity newPerk = this->scene->createEntity();
			this->scene->setComponent<MeshComponent>(newPerk, this->perkMeshes[hpUpPerk]);
			setupPerkSpawn(newPerk, perk);
		}
			break;
		case dmgUpPerk:
		{
			Entity newPerk = this->scene->createEntity();
			this->scene->setComponent<MeshComponent>(newPerk, this->perkMeshes[dmgUpPerk]);
			setupPerkSpawn(newPerk, perk);
		}
			break;
		case attackSpeedUpPerk:
		{
			Entity newPerk = this->scene->createEntity();
			this->scene->setComponent<MeshComponent>(newPerk, this->perkMeshes[attackSpeedUpPerk]);
			setupPerkSpawn(newPerk, perk);
		}
			break;
		}
	}

	void removePerk(Combat& combat, Perks& perk)
	{
		if (perk.perkType != emptyPerk)
		{
			switch (perk.perkType)
			{
			case hpUpPerk:
				setDefaultHp(combat);
				combat.hpMultiplier -= perk.multiplier;
				updateHealth(combat, perk, false);
				spawnPerk(perk);
				break;
			case dmgUpPerk:
				setDefaultDmg(combat);
				combat.dmgMultiplier -= perk.multiplier;
				updateDmg(combat, perk, false);
				spawnPerk(perk);
				break;
			case attackSpeedUpPerk:
				setDefaultAtttackSpeed(combat);
				combat.attackSpeedMultiplier += perk.multiplier;
				updateAttackSpeed(combat, perk, false);
				spawnPerk(perk);
			}
			perk.multiplier = 0;
			perk.perkType = emptyPerk;
		}
	}

	void setupAbilitySpawn(Entity& entity, Abilities& ability)
	{
		this->scene->setComponent<Collider>(entity, Collider::createSphere(4.f, glm::vec3(0, 0, 0), true));
		this->scene->setComponent<Rigidbody>(entity);
		this->scene->setComponent<Abilities>(entity, ability);
		Transform& perkTrans = this->scene->getComponent<Transform>(entity);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		Rigidbody& perkRb = this->scene->getComponent<Rigidbody>(entity);
		perkTrans.position = playerTrans.position;
		perkTrans.scale = glm::vec3(4.f, 4.f, 4.f);
		playerTrans.updateMatrix();
		glm::vec3 throwDir = glm::normalize(playerTrans.forward());
		perkRb.gravityMult = 6.f;
		perkRb.velocity = glm::vec3(throwDir.x * 20.f, 30.f, throwDir.z * 20.f);
	}

	void spawnAbility(Abilities& ability)
	{
		switch (ability.abilityType)
		{
		case knockbackAbility:
		{
			Entity newAbility = this->scene->createEntity();
			this->scene->setComponent<MeshComponent>(newAbility, this->abilityMeshes[knockbackAbility]);
			setupAbilitySpawn(newAbility, ability);
		}
		break;
		case healAbility:
		{
			Entity newAbility = this->scene->createEntity();
			this->scene->setComponent<MeshComponent>(newAbility, this->abilityMeshes[knockbackAbility]);
			setupAbilitySpawn(newAbility, ability);
		}
		break;
		}
	}

	void removeAbility(Combat& combat, Abilities& ability)
	{
		if (ability.abilityType != emptyAbility)
		{
			switch (ability.abilityType)
			{
			case knockbackAbility:
				spawnAbility(ability);
				combat.ability.abilityType = emptyAbility;
				break;
			case healAbility:
				spawnAbility(ability);
				combat.ability.abilityType = emptyAbility;
				break;
			}
		}
	}

	void checkPerkCollision(Combat& combat)
	{
		Collider& playerColl = this->scene->getComponent<Collider>(this->playerID);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		std::vector<int> hitID = this->physics->testContact(playerColl, playerTrans.position, playerTrans.rotation);
		for (size_t i = 0; i < hitID.size(); i++)
		{
			if (scene->hasComponents<Perks>(hitID[i]))
			{
				glm::vec3 pos = this->scene->getComponent<Transform>(hitID[i]).position;
				Perks& perk = this->scene->getComponent<Perks>(hitID[i]);

				this->uiRenderer->renderString(
					PERK_NAMES[perk.perkType] + " boost of " + std::to_string((int)((perk.multiplier + 1) * 100.0f)) + "%",
					pos + glm::vec3(0.0f, 7.5f, 0.0f), glm::vec2(100.0f));
				this->uiRenderer->renderString("press e to pick up", pos + glm::vec3(0.0f, 5.0f, 0.0f), glm::vec2(100.0f));

				if (Input::isKeyPressed(Keys::E))
				{
					for (size_t j = 0; j < 4; j++)
					{
						if (combat.perks[j].perkType == emptyPerk)
						{
							combat.perks[j] = perk;
							switch (combat.perks[j].perkType)
							{
							case hpUpPerk:
								updateHealth(combat, combat.perks[j]);
								break;
							case dmgUpPerk:
								updateDmg(combat, combat.perks[j]);
								break;
							case attackSpeedUpPerk:
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
	}

	void checkAbilityCollision(Combat& combat)
	{
		Collider& playerColl = this->scene->getComponent<Collider>(this->playerID);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		std::vector<int> hitID = this->physics->testContact(playerColl, playerTrans.position, playerTrans.rotation);
		for (size_t i = 0; i < hitID.size(); i++)
		{
			if (scene->hasComponents<Abilities>(hitID[i]))
			{
				glm::vec3 pos = this->scene->getComponent<Transform>(hitID[i]).position;
				Abilities& ability = this->scene->getComponent<Abilities>(hitID[i]);

				this->uiRenderer->renderString(
					ABILITY_NAMES[ability.abilityType] + " ability",
					pos + glm::vec3(0.0f, 7.5f, 0.0f), glm::vec2(100.0f));
				this->uiRenderer->renderString("press e to pick up", pos + glm::vec3(0.0f, 5.0f, 0.0f), glm::vec2(100.0f));

				if (Input::isKeyPressed(Keys::E))
				{
					Abilities& ability = this->scene->getComponent<Abilities>(hitID[i]);
					if (combat.ability.abilityType == emptyAbility)
					{
						combat.ability = ability;
						switch (combat.ability.abilityType)
						{
						case knockbackAbility:
							combat.ability.abilityType = knockbackAbility;
							break;
						case healAbility:
							combat.ability.abilityType = healAbility;
							break;
						}
					}
					scene->removeEntity(hitID[i]);
				}
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
		if (combat.healTimer > 0.f)
		{
			combat.healTimer -= deltaTime;
		}
		else
		{
			combat.isHealing = false;
			this->scene->removeEntity(this->heal);
		}
	}
};
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
	PhysicsEngine* physics;
	UIRenderer* uiRenderer;
	ScriptHandler* script;
	Entity playerID;
	Entity swordID;
	Entity heal;

	int perkMeshes[5];
	int abilityMeshes[2];

	std::vector<Entity> hitEnemies;
	bool canHit = true;

public:

	CombatSystem(Scene* scene, ResourceManager* resourceMng, Entity playerID, 
		PhysicsEngine* physics, UIRenderer* uiRenderer, ScriptHandler* script)
		: scene(scene), resourceMng(resourceMng), playerID(playerID), heal(-1),
		swordID(-1), physics(physics), uiRenderer(uiRenderer), script(script)
	{
		if (scene->hasComponents<Combat>(playerID))
		{
			Combat& combat = scene->getComponent<Combat>(playerID);
			combat.combos.emplace_back("Light Light Light ");
			combat.combos.emplace_back("Light Heavy Light ");
			combat.combos.emplace_back("Heavy Light Heavy ");

			this->swordID = this->scene->createEntity();
			combat.ability.abilityType = emptyAbility;
			for (size_t i = 0; i < 4; i++)
			{
				combat.perks[i].multiplier = 0;
				combat.perks[i].perkType = emptyPerk;
			}
			this->perkMeshes[0] = this->resourceMng->addMesh("assets/models/Perk_Hp.obj");
			this->perkMeshes[1] = this->resourceMng->addMesh("assets/models/Perk_Dmg.obj");
			this->perkMeshes[2] = this->resourceMng->addMesh("assets/models/Perk_AtkSpeed.obj");
			this->perkMeshes[3] = this->resourceMng->addMesh("assets/models/Perk_Movement.obj");
			this->perkMeshes[4] = this->resourceMng->addMesh("assets/models/Perk_Stamina.obj");
			this->abilityMeshes[0] = this->resourceMng->addMesh("assets/models/KnockbackAbility.obj");
			this->abilityMeshes[1] = this->resourceMng->addMesh("assets/models/HealingAbility.obj");
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

			if (checkActiveAttack(combat) != noActive)
			{
				dealDamage(combat);
			}

			if (combat.isHealing)
			{
				Transform& healTrans = this->scene->getComponent<Transform>(this->heal);
				Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
				if (combat.health < combat.maxHealth)
				{
					glm::vec3 playerToHeal = healTrans.position - playerTrans.position;
					float distSqrd = glm::dot(playerToHeal, playerToHeal);
					if (distSqrd < combat.healRadius * combat.healRadius)
					{
						combat.hpRegenConverter += combat.hpRegen * deltaTime;
						if (combat.hpRegenConverter > 1.f)
						{
							combat.health += (int)(combat.hpRegenConverter);
							combat.hpRegenConverter -= 1.f;
						}
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
			if (this->scene->hasComponents<Collider>(this->swordID))
			{
				this->scene->removeComponent<Collider>(this->swordID);
				this->hitEnemies.clear();
				this->canHit = true;
			}
			return combat.activeAttack;
		case lightActive:
			if (combat.attackTimer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case heavyActive:
			if (combat.attackTimer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case comboActive1:
			if (combat.attackTimer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case comboActive2:
			if (combat.attackTimer < 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case comboActive3:
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
		return combat.activeAttack;
	};

	void dealDamage(Combat& combat)
	{
		if (this->scene->hasComponents<Collider>(this->swordID))
		{
			Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
			playerTrans.updateMatrix();
			std::vector<int> hitID = physics->testContact(this->scene->getComponent<Collider>(this->swordID),
				playerTrans.position + playerTrans.forward() * 6.f, glm::vec3(90.f, playerTrans.rotation.y, 0.f));

			for (size_t i = 0; i < hitID.size(); i++)
			{
				if (scene->hasComponents<SwarmComponent>(hitID[i]))
				{
					for (size_t j = 0; j < this->hitEnemies.size(); j++)
					{
						if (this->hitEnemies[j] == hitID[i])
						{
							this->canHit = false;
						}
						else
						{
							this->canHit = true;
						}
					}
					if (this->canHit == true)
					{
						SwarmComponent& enemy = this->scene->getComponent<SwarmComponent>(hitID[i]);
						enemy.life -= (int)combat.dmgArr[combat.activeAttack];
						Rigidbody& enemyRB = this->scene->getComponent<Rigidbody>(hitID[i]);
						Transform& enemyTrans = this->scene->getComponent<Transform>(hitID[i]);
						glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
						enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * combat.knockbackArr[combat.activeAttack];
						this->hitEnemies.emplace_back(hitID[i]);
					}
				}
			}
		}
	}

	bool lightAttack(Combat& combat)
	{
		if (checkActiveAttack(combat) == noActive)
		{
			combat.attackTimer = combat.lightAttackCd;
			combat.comboClearTimer = combat.comboClearDelay;
			combat.comboOrder.append("Light ");

			// If combo is lhh there can be no combo, combo is reset.
			if (combat.comboOrder == "Heavy Light Light ")
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
				this->scene->setAnimation(this->playerID, "lightAttack");
				this->scene->getComponent<AnimationComponent>(this->playerID).timeScale = combat.animationMultiplier[lightActive];

				this->scene->setComponent<Collider>(this->swordID, Collider::createCapsule(3.f, 6.f, glm::vec3(0), true));
				Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
				playerTrans.updateMatrix();
				Transform& swordTrans = this->scene->getComponent<Transform>(this->swordID);
				swordTrans.position = playerTrans.position + playerTrans.forward() * 6.f;
				swordTrans.rotation = glm::vec3(90.f, playerTrans.rotation.y, 0.f);
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
			if (combat.comboOrder == "Heavy Heavy " || combat.comboOrder == "Light Light Heavy ")
			{ 
				combat.comboOrder.clear(); 
			}

			combat.activeAttack = heavyActive;

			if (checkCombo(combat)) { return true; }
			else
			{
				this->scene->setAnimation(this->playerID, "heavyAttack");
				this->scene->getComponent<AnimationComponent>(this->playerID).timeScale = combat.animationMultiplier[heavyActive];

				this->scene->setComponent<Collider>(this->swordID, Collider::createCapsule(3.f, 6.f, glm::vec3(0), true));
				Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
				playerTrans.updateMatrix();
				Transform& swordTrans = this->scene->getComponent<Transform>(this->swordID);
				swordTrans.position = playerTrans.position + playerTrans.forward() * 6.f;
				swordTrans.rotation = glm::vec3(90.f, playerTrans.rotation.y, 0.f);
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
					this->scene->setAnimation(this->playerID, "knockback");
					this->scene->getComponent<AnimationComponent>(this->playerID).timeScale = combat.animationMultiplier[knockbackActive];

					this->scene->setComponent<Collider>(this->swordID, Collider::createSphere(8.f, glm::vec3(0), true));
					Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
					playerTrans.updateMatrix();
					Transform& swordTrans = this->scene->getComponent<Transform>(this->swordID);
					swordTrans.position = playerTrans.position + playerTrans.forward() * 6.f;
					swordTrans.rotation = glm::vec3(90.f, playerTrans.rotation.y, 0.f);
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
				this->scene->setComponent<MeshComponent>(this->heal, this->abilityMeshes[healAbility]);
				this->scene->setComponent<PointLight>(this->heal, { glm::vec3(0.f, 10.f, 0.f), glm::vec3(9.f, 7.f, 9.f) });
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
			combat.comboOrder.clear();
			combat.activeAttack = comboActive1;
			this->scene->setAnimation(this->playerID, "spinAttack");
			this->scene->getComponent<AnimationComponent>(this->playerID).timeScale = combat.animationMultiplier[comboActive1];

			this->scene->setComponent<Collider>(this->swordID, Collider::createSphere(8.f, glm::vec3(0), true));
			Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
			playerTrans.updateMatrix();
			Transform& swordTrans = this->scene->getComponent<Transform>(this->swordID);
			swordTrans.position = playerTrans.position;
		}
		else if (idx == 1)
		{
			combat.attackTimer = combat.comboMixCd;
			combat.comboOrder.clear();
			combat.activeAttack = comboActive2;

			this->scene->setAnimation(this->playerID, "mixAttack");
			this->scene->getComponent<AnimationComponent>(this->playerID).timeScale = combat.animationMultiplier[comboActive2];

			this->scene->setComponent<Collider>(this->swordID, Collider::createCapsule(3.f, 6.f, glm::vec3(0), true));
			Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
			playerTrans.updateMatrix();
			Transform& swordTrans = this->scene->getComponent<Transform>(this->swordID);
			swordTrans.position = playerTrans.position + playerTrans.forward() * 6.f;
			swordTrans.rotation = glm::vec3(90.f, playerTrans.rotation.y, 0.f);
		}
		else if (idx == 2)
		{
			combat.attackTimer = combat.comboHeavyCd;
			combat.comboOrder.clear();
			combat.activeAttack = comboActive3;

			this->scene->setAnimation(this->playerID, "slashAttack");
			this->scene->getComponent<AnimationComponent>(this->playerID).timeScale = combat.animationMultiplier[comboActive3];

			this->scene->setComponent<Collider>(this->swordID, Collider::createCapsule(3.f, 6.f, glm::vec3(0), true));
			Transform& playerTrans = scene->getComponent<Transform>(this->playerID);
			playerTrans.updateMatrix();
			Transform& swordTrans = this->scene->getComponent<Transform>(this->swordID);
			swordTrans.position = playerTrans.position + playerTrans.forward() * 6.f;
			swordTrans.rotation = glm::vec3(90.f, playerTrans.rotation.y, 0.f);
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
		for (size_t i = 0; i < 6; i++)
		{
			combat.dmgArr[i] *= combat.dmgMultiplier;
		}
	}

	void updateAttackSpeed(Combat& combat, Perks& perk, bool doUpgrade = true)
	{
		if (doUpgrade)
		{
			setDefaultAtttackSpeed(combat);
			combat.attackSpeedMultiplier -= perk.multiplier;
			for (size_t i = 0; i < 6; i++)
			{
				combat.animationMultiplier[i] += perk.multiplier;
			}
			if (combat.attackSpeedMultiplier < 0.3f)
			{
				combat.attackSpeedMultiplier = 0.3f;
				combat.animationMultiplier[0] = 1.9f;
				combat.animationMultiplier[1] = 1.7f;
				combat.animationMultiplier[2] = 2.8f;
				combat.animationMultiplier[3] = 2.8f;
				combat.animationMultiplier[4] = 2.8f;
				combat.animationMultiplier[5] = 1.7f;
			}
		}
		combat.lightAttackCd *= combat.attackSpeedMultiplier;
		combat.heavyAttackCd *= combat.attackSpeedMultiplier;
		combat.comboLightCd *= combat.attackSpeedMultiplier;
		combat.comboHeavyCd *= combat.attackSpeedMultiplier;
		combat.comboMixCd *= combat.attackSpeedMultiplier;
		combat.knockbackCd *= combat.attackSpeedMultiplier;
	}

	void updateMovementSpeed(Combat& combat, Perks& perk, bool doUpgrade = true)
	{
		if (doUpgrade)
		{
			setDefaultMovementSpeed(combat);
			combat.movementMultiplier += perk.multiplier;
			Script& playerScript = this->scene->getComponent<Script>(this->playerID);

			float moveTimers[4] = { 0.f, 0.f, 0.f, 0.f };
			this->script->getScriptComponentValue(playerScript, moveTimers[0], "idleAnimTime");
			this->script->getScriptComponentValue(playerScript, moveTimers[1], "runAnimTime");
			this->script->getScriptComponentValue(playerScript, moveTimers[2], "sprintAnimTime");
			this->script->getScriptComponentValue(playerScript, moveTimers[3], "dodgeAnimTime");
			for (size_t i = 0; i < 4; i++)
			{
				moveTimers[i] += perk.multiplier;
			}
			this->script->setScriptComponentValue(playerScript, moveTimers[0], "idleAnimTime");
			this->script->setScriptComponentValue(playerScript, moveTimers[1], "runAnimTime");
			this->script->setScriptComponentValue(playerScript, moveTimers[2], "sprintAnimTime");
			this->script->setScriptComponentValue(playerScript, moveTimers[3], "dodgeAnimTime");
			if (combat.movementMultiplier < 0.3f)
			{
				combat.movementMultiplier = 0.3f;
				moveTimers[0] = 1.7f;
				moveTimers[1] = 1.4f;
				moveTimers[2] = 1.9f;
				moveTimers[3] = 3.2f;
				this->script->setScriptComponentValue(playerScript, moveTimers[0], "idleAnimTime");
				this->script->setScriptComponentValue(playerScript, moveTimers[1], "runAnimTime");
				this->script->setScriptComponentValue(playerScript, moveTimers[2], "sprintAnimTime");
				this->script->setScriptComponentValue(playerScript, moveTimers[3], "dodgeAnimTime");
			}
		}
		Script& playerScript = this->scene->getComponent<Script>(this->playerID);
		int maxSpeed = 0;
		int sprintSpeed = 0;
		int dodgeSpeed = 0;
		this->script->getScriptComponentValue(playerScript, maxSpeed, "maxSpeed");
		this->script->getScriptComponentValue(playerScript, sprintSpeed, "sprintSpeed");
		this->script->getScriptComponentValue(playerScript, dodgeSpeed, "dodgeSpeed");
		float tempMaxSpeed = (float)maxSpeed * combat.movementMultiplier;
		float tempSprintSpeed = (float)sprintSpeed * combat.movementMultiplier;
		float tempDodgeSpeed = (float)dodgeSpeed * combat.movementMultiplier;
		maxSpeed = (int)tempMaxSpeed;
		sprintSpeed = (int)tempSprintSpeed;
		dodgeSpeed = (int)tempDodgeSpeed;
		this->script->setScriptComponentValue(playerScript, maxSpeed, "maxSpeed");
		this->script->setScriptComponentValue(playerScript, sprintSpeed, "sprintSpeed");
		this->script->setScriptComponentValue(playerScript, dodgeSpeed, "dodgeSpeed");
	}

	void updateStamina(Combat& combat, Perks& perk, bool doUpgrade = true)
	{
		if (doUpgrade)
		{
			setDefaultStamina(combat);
			combat.staminaMultiplier += perk.multiplier;
		}
		Script& playerScript = this->scene->getComponent<Script>(this->playerID);
		int maxStam = 0;
		this->script->getScriptComponentValue(playerScript, maxStam, "maxStamina");
		float tempStam = (float)maxStam * combat.staminaMultiplier;
		maxStam = (int)tempStam;
		int currentStam = 0;
		script->getScriptComponentValue(playerScript, currentStam, "currentStamina");
		currentStam = std::min(currentStam, maxStam);
		this->script->setScriptComponentValue(playerScript, maxStam, "maxStamina");
		this->script->setScriptComponentValue(playerScript, currentStam, "currentStamina");
	}

	void setDefaultHp(Combat& combat)
	{
		float tempHp = (float)combat.maxHealth / combat.hpMultiplier;
		combat.maxHealth = (int)(tempHp + 0.5f);
	}

	void setDefaultDmg(Combat& combat)
	{
		for (size_t i = 0; i < 6; i++)
		{
			combat.dmgArr[i] /= combat.dmgMultiplier;
		}
	}

	void setDefaultAtttackSpeed(Combat& combat)
	{
		combat.lightAttackCd /= combat.attackSpeedMultiplier;
		combat.heavyAttackCd /= combat.attackSpeedMultiplier;
		combat.comboLightCd /= combat.attackSpeedMultiplier;
		combat.comboHeavyCd /= combat.attackSpeedMultiplier;
		combat.comboMixCd /= combat.attackSpeedMultiplier;
		combat.knockbackCd /= combat.attackSpeedMultiplier;
		combat.animationMultiplier[0] = 1.2f;
		combat.animationMultiplier[1] = 1.f;
		combat.animationMultiplier[2] = 2.1f;
		combat.animationMultiplier[3] = 2.1f;
		combat.animationMultiplier[4] = 2.1f;
		combat.animationMultiplier[5] = 1.f;
	}

	void setDefaultMovementSpeed(Combat& combat)
	{
		Script& playerScript = this->scene->getComponent<Script>(this->playerID);
		int maxSpeed = 0;
		int sprintSpeed = 0;
		int dodgeSpeed = 0;
		this->script->getScriptComponentValue(playerScript, maxSpeed, "maxSpeed");
		this->script->getScriptComponentValue(playerScript, sprintSpeed, "sprintSpeed");
		this->script->getScriptComponentValue(playerScript, dodgeSpeed, "dodgeSpeed");
		float tempMaxSpeed = (float)maxSpeed / combat.movementMultiplier;
		float tempSprintSpeed = (float)sprintSpeed / combat.movementMultiplier;
		float tempDodgeSpeed = (float)dodgeSpeed / combat.movementMultiplier;
		maxSpeed = (int)tempMaxSpeed;
		sprintSpeed = (int)tempSprintSpeed;
		dodgeSpeed = (int)tempDodgeSpeed;
		this->script->setScriptComponentValue(playerScript, maxSpeed, "maxSpeed");
		this->script->setScriptComponentValue(playerScript, sprintSpeed, "sprintSpeed");
		this->script->setScriptComponentValue(playerScript, dodgeSpeed, "dodgeSpeed");
	}

	void setDefaultStamina(Combat& combat)
	{
		Script& playerScript = this->scene->getComponent<Script>(this->playerID);
		int maxStam = 0;
		this->script->getScriptComponentValue(playerScript, maxStam, "maxStamina");
		float tempStam = (float)maxStam / combat.staminaMultiplier;
		maxStam = (int)(tempStam + 0.5f);
		this->script->setScriptComponentValue(playerScript, maxStam, "maxStamina");
	}

	void setupPerkSpawn(Entity& entity, Perks& perk)
	{
		this->scene->setComponent<Collider>(entity, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
		this->scene->setComponent<Rigidbody>(entity);
		this->scene->setComponent<Perks>(entity, perk);
		this->scene->setComponent<PointLight>(entity, { glm::vec3(0.f), glm::vec3(5.f, 7.f, 9.f) });
		Transform& perkTrans = this->scene->getComponent<Transform>(entity);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		Rigidbody& perkRb = this->scene->getComponent<Rigidbody>(entity);
		perkTrans.position = glm::vec3(playerTrans.position.x, playerTrans.position.y + 8.f, playerTrans.position.z);
		perkTrans.scale = glm::vec3(2.f, 2.f, 2.f);
		playerTrans.updateMatrix();
		glm::vec3 throwDir = glm::normalize(playerTrans.forward());
		perkRb.gravityMult = 6.f;
		perkRb.velocity = glm::vec3(throwDir.x * 20.f, 30.f, throwDir.z * 20.f);
		this->scene->setScriptComponent(entity, "scripts/spin.lua");
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
		case movementUpPerk:
		{
			Entity newPerk = this->scene->createEntity();
			this->scene->setComponent<MeshComponent>(newPerk, this->perkMeshes[movementUpPerk]);
			setupPerkSpawn(newPerk, perk);
		}
		break;
		case staminaUpPerk:
		{
			Entity newPerk = this->scene->createEntity();
			this->scene->setComponent<MeshComponent>(newPerk, this->perkMeshes[staminaUpPerk]);
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
				break;
			case movementUpPerk:
				setDefaultMovementSpeed(combat);
				combat.movementMultiplier -= perk.multiplier;
				updateMovementSpeed(combat, perk, false);
				spawnPerk(perk);
				break;
			case staminaUpPerk:
				setDefaultStamina(combat);
				combat.staminaMultiplier -= perk.multiplier;
				updateStamina(combat, perk, false);
				spawnPerk(perk);
				break;
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
		this->scene->setComponent<PointLight>(entity, { glm::vec3(0.f), glm::vec3(7.f, 9.f, 5.f) });
		Transform& abilityTrans = this->scene->getComponent<Transform>(entity);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		Rigidbody& abilityRb = this->scene->getComponent<Rigidbody>(entity);
		abilityTrans.position = glm::vec3(playerTrans.position.x, playerTrans.position.y + 8.f, playerTrans.position.z);
		abilityTrans.scale = glm::vec3(4.f, 4.f, 4.f);
		playerTrans.updateMatrix();
		glm::vec3 throwDir = glm::normalize(playerTrans.forward());
		abilityRb.gravityMult = 6.f;
		abilityRb.velocity = glm::vec3(throwDir.x * 20.f, 30.f, throwDir.z * 20.f);
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
			if (this->scene->hasComponents<Perks>(hitID[i]))
			{
				if (this->scene->entityValid(hitID[i]))
				{
					glm::vec3 pos = this->scene->getComponent<Transform>(hitID[i]).position;
					Perks& perk = this->scene->getComponent<Perks>(hitID[i]);

					this->uiRenderer->renderString(
						PERK_NAMES[perk.perkType] + " boost of " + std::to_string((int)((perk.multiplier) * 100.0f)) + "%",
						pos + glm::vec3(0.0f, 7.5f, 0.0f), glm::vec2(100.0f), 1.0f);
					this->uiRenderer->renderString("press e to pick up", pos + glm::vec3(0.0f, 5.0f, 0.0f), glm::vec2(100.0f), 1.0f);

					if (Input::isKeyPressed(Keys::E))
					{
						glm::vec3 pos = this->scene->getComponent<Transform>(hitID[i]).position;
						Perks& perk = this->scene->getComponent<Perks>(hitID[i]);

						this->uiRenderer->renderString(
							PERK_NAMES[perk.perkType] + " boost of " + std::to_string((int)((perk.multiplier + 1) * 100.0f)) + "%",
							pos + glm::vec3(0.0f, 7.5f, 0.0f), glm::vec2(100.0f));
						this->uiRenderer->renderString("press e to pick up", pos + glm::vec3(0.0f, 5.0f, 0.0f), glm::vec2(100.0f));

						for (size_t j = 0; j < 4; j++)
						{
							if (combat.perks[j].perkType == emptyPerk)
							{
								combat.perks[j] = perk;
								switch (combat.perks[j].perkType)
								{
								case hpUpPerk:
									updateHealth(combat, combat.perks[j]);
									this->scene->removeEntity(hitID[i]);
									break;
								case dmgUpPerk:
									updateDmg(combat, combat.perks[j]);
									this->scene->removeEntity(hitID[i]);
									break;
								case attackSpeedUpPerk:
									updateAttackSpeed(combat, combat.perks[j]);
									this->scene->removeEntity(hitID[i]);
									break;
								case movementUpPerk:
									updateMovementSpeed(combat, combat.perks[j]);
									this->scene->removeEntity(hitID[i]);
									break;
								case staminaUpPerk:
									updateStamina(combat, combat.perks[j]);
									this->scene->removeEntity(hitID[i]);
									break;
								}
								j = 4;
							}
						}
					}
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
			if (this->scene->hasComponents<Abilities>(hitID[i]))
			{
				if (this->scene->entityValid(hitID[i]))
				{
					glm::vec3 pos = this->scene->getComponent<Transform>(hitID[i]).position;
					Abilities& ability = this->scene->getComponent<Abilities>(hitID[i]);

					this->uiRenderer->renderString(
						ABILITY_NAMES[ability.abilityType] + " ability",
						pos + glm::vec3(0.0f, 7.5f, 0.0f), glm::vec2(100.0f), 1.0f);
					this->uiRenderer->renderString("press e to pick up", pos + glm::vec3(0.0f, 5.0f, 0.0f), glm::vec2(100.0f), 1.0f);

					if (Input::isKeyPressed(Keys::E))
					{
						glm::vec3 pos = this->scene->getComponent<Transform>(hitID[i]).position;
						Abilities& ability = this->scene->getComponent<Abilities>(hitID[i]);

						this->uiRenderer->renderString(
							ABILITY_NAMES[ability.abilityType] + " ability",
							pos + glm::vec3(0.0f, 7.5f, 0.0f), glm::vec2(100.0f));
						this->uiRenderer->renderString("press e to pick up", pos + glm::vec3(0.0f, 5.0f, 0.0f), glm::vec2(100.0f));

						if (combat.ability.abilityType == emptyAbility)
						{
							combat.ability = ability;
							switch (combat.ability.abilityType)
							{
							case knockbackAbility:
								combat.ability.abilityType = knockbackAbility;
								this->scene->removeEntity(hitID[i]);
								break;
							case healAbility:
								combat.ability.abilityType = healAbility;
								this->scene->removeEntity(hitID[i]);
								break;
							}
						}
					}
				}
			}
		}
	}

	void decreaseTimers(Combat& combat, float deltaTime)
	{
		if (combat.attackTimer > 0.f)
		{
			combat.attackTimer -= deltaTime;
		}
		if (combat.knockbackTimer > 0.f)
		{
			combat.knockbackTimer -= deltaTime;
		}
		if (combat.comboClearTimer > 0.f)
		{
			combat.comboClearTimer -= deltaTime;
		}
		else if(combat.comboClearTimer < 0.f)
		{
			combat.comboOrder.clear();
			combat.comboClearTimer = 0.f;
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

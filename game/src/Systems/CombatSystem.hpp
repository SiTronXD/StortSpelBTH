#pragma once

#include <vengine.h>
#include "../Components/Combat.h"
#include "../Components/HealthComp.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../Network/NetworkHandlerGame.h"
#include "../Ai/Behaviors/Tank/TankFSM.hpp"

#include "../Components/HealArea.h"

enum SoundEnum { takeDmg, swingSword };

class CombatSystem : public System
{
private:
	Scene* scene;
	ResourceManager* resourceMng;
	PhysicsEngine* physics;
	UIRenderer* uiRenderer;
	ScriptHandler* script;
	NetworkHandlerGame* networkHandler;
	Entity playerID;
	Entity swordID;
	const bool* paused;
	const bool* disabled;

	int lostHealth;

	Entity takeDmgAudioSource;
	Entity attackAudioSource;
	Entity moveAudioSource;
	std::vector<uint32_t> attackSounds;
	std::vector<uint32_t> moveSounds;
	std::vector<uint32_t> takeDmgSounds;

	int swordMesh;
	int perkMeshes[5];
	int abilityMeshes[2];

	std::vector<Entity> hitEnemies;
	bool canHit = true;

public:

	CombatSystem(Scene* scene, ResourceManager* resourceMng, Entity playerID, const bool* paused, const bool* disabled,
		PhysicsEngine* physics, UIRenderer* uiRenderer, ScriptHandler* script, NetworkHandlerGame* networkHandler)
		: scene(scene), resourceMng(resourceMng), playerID(playerID), paused(paused), disabled(disabled),
		swordID(-1), physics(physics), uiRenderer(uiRenderer), script(script), networkHandler(networkHandler)
	{
		this->networkHandler->setCombatSystem(this);
		if (scene->hasComponents<Combat>(playerID))
		{
			Combat& combat = scene->getComponent<Combat>(playerID);
			combat.combos.emplace_back("Light Light Light ");
			combat.combos.emplace_back("Light Heavy Light ");
			combat.combos.emplace_back("Heavy Light Heavy ");

			this->takeDmgSounds.emplace_back(this->resourceMng->addSound("assets/Sounds/OufSound.ogg"));
			this->attackSounds.emplace_back(this->resourceMng->addSound("assets/Sounds/SwishSound.ogg"));
			this->moveSounds.emplace_back(this->resourceMng->addSound("assets/Sounds/RunningSound.ogg"));
			this->attackAudioSource = this->scene->createEntity();
			this->takeDmgAudioSource = this->scene->createEntity();
			this->moveAudioSource = this->scene->createEntity();
			this->scene->setComponent<AudioSource>(this->attackAudioSource, this->attackSounds[0]);
			this->scene->setComponent<AudioSource>(this->takeDmgAudioSource, this->takeDmgSounds[0]);
			this->scene->setComponent<AudioSource>(this->moveAudioSource, this->moveSounds[0]);

			this->swordID = this->scene->createEntity();
			this->swordMesh = this->resourceMng->addMesh("assets/models/MainSword.fbx", "assets/textures");
			this->scene->setComponent<MeshComponent>(this->swordID, this->swordMesh);
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

			this->lostHealth = this->scene->getComponent<HealthComp>(this->playerID).health;
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
			if (!*this->disabled)
			{
				updateSwordPos();
			}
			else if (this->scene->isActive(this->swordID))
			{
				this->scene->setInactive(this->swordID);
			}

			// Return if paused of disabled
			if (*this->paused || *this->disabled)
			{
				return;
			}

			if (checkActiveAttack(combat) != noActive)
			{
				dealDamage(combat);
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

			if (this->scene->getAnimationStatus(this->playerID, "").animationName == "run" && !this->scene->getComponent<AudioSource>(this->moveAudioSource).isPlaying())
			{
				playerEffectSound(this->moveSounds[0], this->scene->getComponent<Transform>(this->playerID).position, this->moveAudioSource, 10.f);
			}
			else if (this->scene->getAnimationStatus(this->playerID, "").animationName != "run")
			{
				this->scene->getComponent<AudioSource>(this->moveAudioSource).stop();
			}

			HealthComp& healthComp = this->scene->getComponent<HealthComp>(this->playerID);
			if (this->lostHealth > healthComp.health)
			{
				this->lostHealth = healthComp.health;
				takeDmg();
			}

#ifdef _CONSOLE
			float animMultis[6] = { combat.animationMultiplier[0], combat.animationMultiplier[1], combat.animationMultiplier[2],
			combat.animationMultiplier[3], combat.animationMultiplier[4], combat.animationMultiplier[5] };

			ImGui::Begin("AnimationMultipliers");
			ImGui::SliderFloat("Light Anim", &animMultis[0], 0.f, 10.f);
			ImGui::SliderFloat("Heavy Anim", &animMultis[1], 0.f, 10.f);
			ImGui::SliderFloat("Combo1 Anim", &animMultis[2], 0.f, 10.f);
			ImGui::SliderFloat("Combo2 Anim", &animMultis[3], 0.f, 10.f);
			ImGui::SliderFloat("Combo3 Anim", &animMultis[4], 0.f, 10.f);
			ImGui::SliderFloat("Knockback Anim", &animMultis[5], 0.f, 10.f);
			ImGui::End();
			for (size_t i = 0; i < 6; i++)
			{
				combat.animationMultiplier[i] = animMultis[i];
			}
#endif
		};
		view.each(foo);

		return false;
	}

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
			if (combat.attackTimer <= 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case heavyActive:
			if (combat.attackTimer <= 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case comboActive1:
			if (combat.attackTimer <= 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case comboActive2:
			if (combat.attackTimer <= 0.f)
			{
				combat.activeAttack = noActive;
			}
			return combat.activeAttack;
		case comboActive3:
			if (combat.attackTimer <= 0.f)
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
			Transform& swordTrans = scene->getComponent<Transform>(this->swordID);
			swordTrans.updateMatrix();
			std::vector<int> hitID = physics->testContact(this->scene->getComponent<Collider>(this->swordID),
				swordTrans.position);
			for (size_t i = 0; i < hitID.size(); i++) 
			{
				this->canHit = true;
				for (size_t j = 0; j < this->hitEnemies.size(); j++)
				{
					if (this->hitEnemies[j] == hitID[i])
					{
					    this->canHit = false;
			            break;
					}
				}
				if (this->canHit)
				{
					//if (!this->scene->getComponent<AudioSource>(this->otherSound).isPlaying())
					//{
					//	playerEffectSound(this->otherSounds[0], this->scene->getComponent<Transform>(hitID[i]).position, this->otherSound, 12.f);
					//}
					this->networkHandler->sendHitOn(hitID[i], (int)combat.dmgArr[combat.activeAttack], combat.knockbackArr[combat.activeAttack]);
					this->hitEnemies.emplace_back(hitID[i]);
				}
			}
		}
	}

	void takeDmg()
	{
		if (!this->scene->getComponent<AudioSource>(this->takeDmgAudioSource).isPlaying())
		{
			playerEffectSound(this->takeDmgSounds[0], this->scene->getComponent<Transform>(this->playerID).position,
				this->takeDmgAudioSource, 10.f);
		}
	}

	void hitEnemy(Combat& combat, int ID)
	{
		Rigidbody& enemyRB = this->scene->getComponent<Rigidbody>(ID);
		Transform& enemyTrans = this->scene->getComponent<Transform>(ID);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
		enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * combat.knockbackArr[combat.activeAttack];
		this->hitEnemies.emplace_back(ID);
	}

	void setupAttack(std::string animName, int animIdx, float cdValue, float animMultiplier)
	{
		Script& playerScript = this->scene->getComponent<Script>(this->playerID);
		int currentAnimation = 0;
		this->script->getScriptComponentValue(playerScript, currentAnimation, "currentAnimation");

		if (animIdx == 6)
		{
			this->script->setScriptComponentValue(playerScript, true, "wholeBody");
			this->script->setScriptComponentValue(playerScript, false, "canMove");
			this->scene->blendToAnimation(this->playerID, animName, "", 0.18f, animMultiplier);
		}
		else
		{
			this->script->setScriptComponentValue(playerScript, false, "wholeBody");
			this->scene->blendToAnimation(this->playerID, animName, "UpperBody", 0.18f, animMultiplier);
		}
		this->script->setScriptComponentValue(playerScript, animIdx, "currentAnimation");
		this->script->setScriptComponentValue(playerScript, cdValue, "animTimer");
		Transform& swordTrans = this->scene->getComponent<Transform>(this->swordID);
		swordTrans.updateMatrix();
		this->scene->setComponent<Collider>(this->swordID, Collider::createCapsule(3.f, 12.f, (swordTrans.right() * swordTrans.forward()), true));
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
				playerEffectSound(this->attackSounds[0], 
					this->scene->getComponent<Transform>(this->playerID).position, this->attackAudioSource, 7.f);
				setupAttack("lightAttack", 5, combat.lightAttackCd, combat.animationMultiplier[lightActive]);
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
				setupAttack("heavyAttack", 5, combat.heavyAttackCd, combat.animationMultiplier[heavyActive]);
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
					setupAttack("knockback", 5, combat.knockbackCd, combat.animationMultiplier[knockbackActive]);
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
				combat.ability.abilityType = emptyAbility;

				glm::vec3& pos = this->scene->getComponent<Transform>(this->playerID).position;
				pos.y = 0.0f;
				this->networkHandler->useHealAbilityRequest(pos);

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
			setupAttack("spinAttack", 6, combat.comboLightCd, combat.animationMultiplier[comboActive1]);
		}
		else if (idx == 1)
		{
			combat.attackTimer = combat.comboMixCd;
			combat.comboOrder.clear();
			combat.activeAttack = comboActive2;
			setupAttack("mixAttack", 5, combat.comboMixCd, combat.animationMultiplier[comboActive2]);
		}
		else if (idx == 2)
		{
			combat.attackTimer = combat.comboHeavyCd;
			combat.comboOrder.clear();
			combat.activeAttack = comboActive3;
			setupAttack("slashAttack", 5, combat.comboHeavyCd, combat.animationMultiplier[comboActive3]);
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

	void updateHealth(Combat& combat, HealthComp& healthComp, Perks& perk, bool doUpgrade = true)
	{
		if (doUpgrade)
		{
			setDefaultHp(combat, healthComp);
			combat.hpMultiplier += perk.multiplier;
		}
		float tempHp = (float)healthComp.maxHealth * combat.hpMultiplier;
		healthComp.maxHealth = (int)tempHp;
		Script& playerScript = this->scene->getComponent<Script>(this->playerID);
		this->script->setScriptComponentValue(playerScript, healthComp.maxHealth, "maxHealth");
		healthComp.health = std::min(healthComp.health, healthComp.maxHealth);
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
			combat.totalAtkSpeedMulti -= perk.multiplier;
			if (combat.totalAtkSpeedMulti < 0.3f)
			{
				combat.attackSpeedMultiplier = 0.3f;
				combat.animationMultiplier[0] = 2.8f;
				combat.animationMultiplier[1] = 2.4f;
				combat.animationMultiplier[2] = 4.6f;
				combat.animationMultiplier[3] = 4.6f;
				combat.animationMultiplier[4] = 4.6f;
				combat.animationMultiplier[5] = 2.4f;
			}
			else
			{
				combat.attackSpeedMultiplier -= perk.multiplier;
				for (size_t i = 0; i < 6; i++)
				{
					combat.animationMultiplier[i] += perk.multiplier + perk.multiplier;
				}
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
		Script& playerScript = this->scene->getComponent<Script>(this->playerID);
		float moveTimers[4] = { 0.f, 0.f, 0.f, 0.f };
		if (doUpgrade)
		{
			setDefaultMovementSpeed(combat);
			combat.totalMoveMulti += perk.multiplier;
			if (combat.totalMoveMulti > 2.f)
			{
				combat.movementMultiplier = 2.f;
				moveTimers[0] = 2.f;
				moveTimers[1] = 1.4f;
				moveTimers[2] = 2.4f;
				this->script->setScriptComponentValue(playerScript, moveTimers[0], "idleAnimTime");
				this->script->setScriptComponentValue(playerScript, moveTimers[1], "runAnimTime");
				this->script->setScriptComponentValue(playerScript, moveTimers[2], "sprintAnimTime");
			}
			else
			{
				combat.movementMultiplier += perk.multiplier;
				this->script->getScriptComponentValue(playerScript, moveTimers[0], "idleAnimTime");
				this->script->getScriptComponentValue(playerScript, moveTimers[1], "runAnimTime");
				this->script->getScriptComponentValue(playerScript, moveTimers[2], "sprintAnimTime");
				moveTimers[0] += perk.multiplier;
				moveTimers[1] += perk.multiplier;
				moveTimers[2] += perk.multiplier;
				this->script->setScriptComponentValue(playerScript, moveTimers[0], "idleAnimTime");
				this->script->setScriptComponentValue(playerScript, moveTimers[1], "runAnimTime");
				this->script->setScriptComponentValue(playerScript, moveTimers[2], "sprintAnimTime");
			}
		}
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

	void setDefaultHp(Combat& combat, HealthComp& healthComp)
	{
		float tempHp = (float)healthComp.maxHealth / combat.hpMultiplier;
		healthComp.maxHealth = (int)(tempHp + 0.5f);
		Script& playerScript = this->scene->getComponent<Script>(this->playerID);
		this->script->setScriptComponentValue(playerScript, healthComp.maxHealth, "maxHealth");
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
		//int maxSpeed = 0;
		//int sprintSpeed = 0;
		//int dodgeSpeed = 0;
		//this->script->getScriptComponentValue(playerScript, maxSpeed, "maxSpeed");
		//this->script->getScriptComponentValue(playerScript, sprintSpeed, "sprintSpeed");
		//this->script->getScriptComponentValue(playerScript, dodgeSpeed, "dodgeSpeed");
		//float tempMaxSpeed = (float)maxSpeed / combat.movementMultiplier;
		//float tempSprintSpeed = (float)sprintSpeed / combat.movementMultiplier;
		//float tempDodgeSpeed = (float)dodgeSpeed / combat.movementMultiplier;
		//maxSpeed = (int)tempMaxSpeed;
		//sprintSpeed = (int)tempSprintSpeed;
		//dodgeSpeed = (int)tempDodgeSpeed;
		this->script->setScriptComponentValue(playerScript, 30, "maxSpeed");
		this->script->setScriptComponentValue(playerScript, 60, "sprintSpeed");
		this->script->setScriptComponentValue(playerScript, 100, "dodgeSpeed");

		this->script->setScriptComponentValue(playerScript, 1.f, "idleAnimTime");
		this->script->setScriptComponentValue(playerScript, 0.7f, "runAnimTime");
		this->script->setScriptComponentValue(playerScript, 1.2f, "sprintAnimTime");
		this->script->setScriptComponentValue(playerScript, 3.f, "dodgeAnimTime");
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
	}

	void spawnPerk(Perks& perk)
	{
		Entity newPerk = this->scene->createEntity();
		this->scene->setComponent<MeshComponent>(newPerk, this->perkMeshes[perk.perkType]);
		setupPerkSpawn(newPerk, perk);
	}

	void removePerk(Combat& combat, Perks& perk)
	{
		if (perk.perkType != emptyPerk)
		{
			Transform& t = this->scene->getComponent<Transform>(this->playerID);
			HealthComp& healthComp = this->scene->getComponent<HealthComp>(this->playerID);
			t.updateMatrix();
			this->networkHandler->spawnItemRequest(perk.perkType, perk.multiplier, t.position + glm::vec3(0.0f, 8.0f, 0.0f), t.forward());
			switch (perk.perkType)
			{
			case hpUpPerk:
				setDefaultHp(combat, healthComp);
				combat.hpMultiplier -= perk.multiplier;
				updateHealth(combat, healthComp, perk, false);
				break;
			case dmgUpPerk:
				setDefaultDmg(combat);
				combat.dmgMultiplier -= perk.multiplier;
				updateDmg(combat, perk, false);
				break;
			case attackSpeedUpPerk:
				setDefaultAtttackSpeed(combat);
				combat.totalAtkSpeedMulti += perk.multiplier;
				combat.attackSpeedMultiplier = glm::clamp(combat.totalAtkSpeedMulti, 0.3f, 1.f);
				updateAttackSpeed(combat, perk, false);
				break;
			case movementUpPerk:
				setDefaultMovementSpeed(combat);
				combat.totalMoveMulti -= perk.multiplier;
				combat.movementMultiplier = glm::clamp(combat.totalMoveMulti, 1.f, 2.f);
				updateMovementSpeed(combat, perk, false);
				break;
			case staminaUpPerk:
				setDefaultStamina(combat);
				combat.staminaMultiplier -= perk.multiplier;
				updateStamina(combat, perk, false);
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
		abilityTrans.scale = glm::vec3(3.f);
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
			Transform& t = this->scene->getComponent<Transform>(this->playerID);
			t.updateMatrix();
			this->networkHandler->spawnItemRequest(ability.abilityType, t.position + glm::vec3(0.0f, 8.0f, 0.0f), t.forward());
			combat.ability.abilityType = emptyAbility;
		}
	}

	void updateSwordPos()
	{
		// Put sword in characters hand and keep updating it
		this->scene->getComponent<Transform>(this->swordID).setMatrix(
			this->resourceMng->getJointTransform(
				this->scene->getComponent<Transform>(this->playerID),
				this->scene->getComponent<MeshComponent>(this->playerID),
				this->scene->getComponent<AnimationComponent>(this->playerID),
				"mixamorig:RightHand") * glm::translate(glm::mat4(1.f), glm::vec3(0.f, 1.f, 0.f)) *
			glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.f)));
	}

	void checkPerkCollision(Combat& combat)
	{
		Collider& playerColl = this->scene->getComponent<Collider>(this->playerID);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		glm::vec3 up = this->scene->getComponent<Transform>(this->scene->getMainCameraID()).up();
		std::vector<int> hitID = this->physics->testContact(playerColl, playerTrans.position, playerTrans.rotation);
		for (size_t i = 0; i < hitID.size(); i++)
		{
			if (this->scene->hasComponents<Perks>(hitID[i]))
			{
				glm::vec3 pos = this->scene->getComponent<Transform>(hitID[i]).position;
				Perks& perk = this->scene->getComponent<Perks>(hitID[i]);

				this->uiRenderer->renderString(
					PERK_NAMES[perk.perkType] + " boost of " + std::to_string((int)((perk.multiplier) * 100.0f)) + "%",
					pos + glm::vec3(0.0f, 7.5f, 0.0f), glm::vec2(100.0f), 1.0f);
				this->uiRenderer->renderString("press e to pick up", pos + glm::vec3(0.0f, 7.5f, 0.0f) - up * 2.5f, glm::vec2(100.0f), 1.0f);

				if (Input::isKeyPressed(Keys::E))
				{
					// Another check instead of checking every frame (only when E is pressed)
					if (canPickupPerk())
					{
						this->networkHandler->pickUpItemRequest(hitID[i], ItemType::PERK);
					}
				}
			}
		}
	}

	bool canPickupPerk()
	{
		Combat& combat = this->scene->getComponent<Combat>(this->playerID);
		for (size_t i = 0; i < 4; i++)
		{
			if (combat.perks[i].perkType == emptyPerk)
			{
				return true;
			}
		}
		return false;
	}

	void pickupPerk(Entity entity)
	{
		if (this->scene->hasComponents<Perks>(entity))
		{
			Combat& combat = this->scene->getComponent<Combat>(this->playerID);
			HealthComp& healthComp = this->scene->getComponent<HealthComp>(this->playerID);
			Perks& perk = this->scene->getComponent<Perks>(entity);
			for (size_t j = 0; j < 4; j++)
			{
				if (combat.perks[j].perkType == emptyPerk)
				{
					combat.perks[j] = perk;
					switch (combat.perks[j].perkType)
					{
					case hpUpPerk:
						updateHealth(combat, healthComp, combat.perks[j]);
						break;
					case dmgUpPerk:
						updateDmg(combat, combat.perks[j]);
						break;
					case attackSpeedUpPerk:
						updateAttackSpeed(combat, combat.perks[j]);
						break;
					case movementUpPerk:
						updateMovementSpeed(combat, combat.perks[j]);
						break;
					case staminaUpPerk:
						updateStamina(combat, combat.perks[j]);
						break;
					}
					this->scene->removeEntity(entity);
					j = 4;
				}
			}
		}
	}

	void checkAbilityCollision(Combat& combat)
	{
		Collider& playerColl = this->scene->getComponent<Collider>(this->playerID);
		Transform& playerTrans = this->scene->getComponent<Transform>(this->playerID);
		glm::vec3 up = this->scene->getComponent<Transform>(this->scene->getMainCameraID()).up();
		std::vector<int> hitID = this->physics->testContact(playerColl, playerTrans.position, playerTrans.rotation);
		for (size_t i = 0; i < hitID.size(); i++)
		{
			if (this->scene->hasComponents<Abilities>(hitID[i]))
			{
				glm::vec3 pos = this->scene->getComponent<Transform>(hitID[i]).position;
				Abilities& ability = this->scene->getComponent<Abilities>(hitID[i]);

				this->uiRenderer->renderString(
					ABILITY_NAMES[ability.abilityType] + " ability",
					pos + glm::vec3(0.0f, 7.5f, 0.0f), glm::vec2(100.0f), 1.0f);
				this->uiRenderer->renderString("press e to pick up", pos + glm::vec3(0.0f, 7.5f, 0.0f) - up * 2.5f, glm::vec2(100.0f), 1.0f);

				if (Input::isKeyPressed(Keys::E) && combat.ability.abilityType == emptyAbility)
				{
					this->networkHandler->pickUpItemRequest(hitID[i], ItemType::ABILITY);
				}
			}
		}
	}

	void playerEffectSound(uint32_t effect, glm::vec3 pos, Entity& soundSource, float volume)
	{
		Transform& soundTrans = this->scene->getComponent<Transform>(soundSource);
		soundTrans.position = pos;
		AudioSource& sound = this->scene->getComponent<AudioSource>(soundSource);
		//sound.setBuffer(effect);
		sound.setVolume(volume);
		sound.play();
	}
	
	void pickUpAbility(Entity entity)
	{
		Combat& combat = this->scene->getComponent<Combat>(this->playerID);
		if (this->scene->hasComponents<Abilities>(entity) && combat.ability.abilityType == emptyAbility)
		{
			Abilities& ability = this->scene->getComponent<Abilities>(entity);
			combat.ability = ability;
			this->scene->removeEntity(entity);
		}
	}

	void decreaseTimers(Combat& combat, float deltaTime)
	{
		if (combat.attackTimer > 0.f)
		{
			combat.attackTimer -= deltaTime;
		}
		else if (combat.attackTimer < 0.f)
		{
			Script& playerScript = this->scene->getComponent<Script>(this->playerID);
			this->script->setScriptComponentValue(playerScript, true, "canMove");
			this->script->setScriptComponentValue(playerScript, 4, "currentAnimation");
			this->scene->syncedBlendToAnimation(this->playerID, "LowerBody", "UpperBody", 0.3f);
			combat.attackTimer = 0.f;
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
	}
};

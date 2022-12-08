#pragma once

#include <vengine.h>
#include "../Components/HealthComp.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"

class HealthBarSystem : public System
{
private:
	int backgroundID;
	int healthBarID;
	Scene* scene;
	UIRenderer* uiRenderer;
public:
	HealthBarSystem(int backgroundID, int healthBarID, Scene* scene, UIRenderer* uiRenderer)
		: backgroundID(backgroundID), healthBarID(healthBarID), scene(scene), uiRenderer(uiRenderer)
	{ }

	virtual ~HealthBarSystem()
	{ }

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto tankView = reg.view<Transform, TankComponent>(entt::exclude<Inactive>);
		auto lichView = reg.view<Transform, LichComponent>(entt::exclude<Inactive>);
		auto swarmView = reg.view<Transform, SwarmComponent>(entt::exclude<Inactive>);
		if (!this->scene->entityValid(this->scene->getMainCameraID())) { return false; }

		uiRenderer->setTexture(backgroundID);
		auto tankBackground = [&](Transform& transform, TankComponent& tank)
		{
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 36.0f, 0.0f), glm::vec2(2000.0f, 150.0f));
		};
		tankView.each(tankBackground);
		auto lichBackground = [&](Transform& transform, LichComponent& lich)
		{
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 28.0f, 0.0f), glm::vec2(1750.0f, 150.0f));
		};
		lichView.each(lichBackground);
		auto swarmBackground = [&](Transform& transform, SwarmComponent& swarm)
		{
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 10.0f, 0.0f), glm::vec2(1500.0f, 150.0f));
		};
		swarmView.each(swarmBackground);

		uiRenderer->setTexture(healthBarID);
		Transform& camTransform = this->scene->getComponent<Transform>(this->scene->getMainCameraID());

		auto tankHealthBar = [&](Transform& transform, TankComponent& tank)
		{
			if (tank.life > tank.FULL_HEALTH)
			{
				tank.FULL_HEALTH = tank.life;
			}
			float percentage = std::max((float)tank.life / tank.FULL_HEALTH, 0.0f);
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 36.0f, 0.0f) + camTransform.right() * 9.0f * (1.0f - percentage), glm::vec2(1950.0f * percentage, 100.0f));
		};
		tankView.each(tankHealthBar);
		auto lichHealthBar = [&](Transform& transform, LichComponent& lich)
		{
			if (lich.life > lich.FULL_HEALTH)
			{
				lich.FULL_HEALTH = lich.life;
			}
			float percentage = std::max((float)lich.life / lich.FULL_HEALTH, 0.0f);
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 28.0f, 0.0f) + camTransform.right() * 7.75f * (1.0f - percentage), glm::vec2(1700.0f * percentage, 100.0f));
		};
		lichView.each(lichHealthBar);
		auto swarmHealthBar = [&](Transform& transform, SwarmComponent& swarm)
		{
			if (swarm.life > swarm.FULL_HEALTH)
			{
				swarm.FULL_HEALTH = swarm.life;
			}
			float percentage = std::max((float)swarm.life / swarm.FULL_HEALTH, 0.0f);
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 10.0f, 0.0f) + camTransform.right() * 6.75f * (1.0f - percentage), glm::vec2(1450.0f * percentage, 100.0f));
		};
		swarmView.each(swarmHealthBar);

		return false;
	}
};
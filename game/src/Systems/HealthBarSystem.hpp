#pragma once

#include <vengine.h>
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
		auto view = reg.view<Transform, SwarmComponent>(entt::exclude<Inactive>);
		if (!this->scene->entityValid(this->scene->getMainCameraID())) { return false; }

		uiRenderer->setTexture(backgroundID);
		auto background = [&](Transform& transform, SwarmComponent& swarm)
		{
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 10.0f, 0.0f), glm::vec2(1500.0f, 150.0f));
		};
		view.each(background);

		uiRenderer->setTexture(healthBarID);
		Transform& camTransform = this->scene->getComponent<Transform>(this->scene->getMainCameraID());

		auto healthBar = [&](Transform& transform, SwarmComponent& swarm)
		{
			float percentage = (float)swarm.life / swarm.FULL_HEALTH;
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 10.0f, 0.0f) + camTransform.right() * 6.75f * (1.0f - percentage), glm::vec2(1450.0f * percentage, 100.0f));
		};
		view.each(healthBar);

		// Tank Health Bar
		auto tankView = reg.view<Transform, TankComponent>(entt::exclude<Inactive>);
		if (!this->scene->entityValid(this->scene->getMainCameraID())) { return false; }

		uiRenderer->setTexture(backgroundID);
		auto tankBackground = [&](Transform& transform, TankComponent& tank)
		{
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 10.0f, 0.0f), glm::vec2(1500.0f, 150.0f));
		};
		tankView.each(tankBackground);

		uiRenderer->setTexture(healthBarID);

		auto tankHealthBar = [&](Transform& transform, TankComponent& tank)
		{
			float percentage = (float)tank.life / tank.FULL_HEALTH;
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 10.0f, 0.0f) + camTransform.right() * 6.75f * (1.0f - percentage), glm::vec2(1450.0f * percentage, 100.0f));
		};
		tankView.each(tankHealthBar);

		return false;
	}
};
#pragma once

#include <vengine.h>
#include "../Components/HealthComp.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"

class HealthBarSystem : public System
{
private:
	const static int MAX_ENTITIES = 30;
	const static int ALPHA_DIST = 100;
	const static int FALLOFF_DIST = 25;

	int backgroundID;
	int healthBarID;
	Scene* scene;
	UIRenderer* uiRenderer;
	std::vector<glm::vec4> backgroundRects;
public:
	HealthBarSystem(int backgroundID, int healthBarID, Scene* scene, UIRenderer* uiRenderer)
		: backgroundID(backgroundID), healthBarID(healthBarID), scene(scene), uiRenderer(uiRenderer)
	{
		this->backgroundRects.resize(MAX_ENTITIES);
	}

	virtual ~HealthBarSystem()
	{ }

	void renderHealth(Transform& transform, Transform& camTransform, int index, float percentage)
	{
		float dotDist = glm::dot(transform.position - camTransform.position, camTransform.forward());
		float alpha = std::clamp((ALPHA_DIST + FALLOFF_DIST - dotDist) / (float)FALLOFF_DIST, 0.0f, 1.0f);
		float margin = 250.0f / dotDist;
		glm::vec2 pos = glm::vec2(backgroundRects[index].x, backgroundRects[index].y);
		glm::vec2 size = glm::vec2(backgroundRects[index].z, backgroundRects[index].w);
		if (size != glm::vec2(0.0f))
		{
			size -= glm::vec2(margin);
		}

		uiRenderer->renderTexture(glm::vec2(pos.x - size.x * 0.5f * (1.0f - percentage), pos.y), glm::vec2(size.x * percentage, size.y),
			glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, alpha));
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		if (!this->scene->entityValid(this->scene->getMainCameraID())) { return false; }

		auto tankView = reg.view<Transform, TankComponent>(entt::exclude<Inactive>);
		auto lichView = reg.view<Transform, LichComponent>(entt::exclude<Inactive>);
		auto swarmView = reg.view<Transform, SwarmComponent>(entt::exclude<Inactive>);
		int counter = 0;
		Transform& camTransform = this->scene->getComponent<Transform>(this->scene->getMainCameraID());

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
		
		auto tankHealthBar = [&](Transform& transform, TankComponent& tank)
		{
			float percentage = std::max((float)tank.life / tank.FULL_HEALTH, 0.0f);
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 36.0f, 0.0f) + camTransform.right() * 9.0f * (1.0f - percentage), glm::vec2(1950.0f * percentage, 100.0f));
		};
		tankView.each(tankHealthBar);
		auto lichHealthBar = [&](Transform& transform, LichComponent& lich)
		{
			float percentage = std::max((float)lich.life / lich.FULL_HEALTH, 0.0f);
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 28.0f, 0.0f) + camTransform.right() * 7.75f * (1.0f - percentage), glm::vec2(1700.0f * percentage, 100.0f));
		};
		lichView.each(lichHealthBar);
		auto swarmHealthBar = [&](Transform& transform, SwarmComponent& swarm)
		{
			float percentage = std::max((float)swarm.life / swarm.FULL_HEALTH, 0.0f);
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 10.0f, 0.0f) + camTransform.right() * 6.75f * (1.0f - percentage), glm::vec2(1450.0f * percentage, 100.0f));
		};
		swarmView.each(swarmHealthBar);

		//backgroundRects.assign(counter, glm::vec4(0.0f));
		std::fill(backgroundRects.begin(),backgroundRects.end(), glm::vec4(0.0f));
		return false;
	}
};
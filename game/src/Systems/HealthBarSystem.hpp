#pragma once

#include <vengine.h>
#include "../Components/HealthComp.h"
#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"

class HealthBarSystem : public System
{
private:
	const static int MAX_ENTITIES = 30;

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
			float dotDist = glm::dot(transform.position - camTransform.position, camTransform.forward());
			float alpha = dotDist <= 100.0f ? 1.0f : std::max(125.0f - dotDist, 0.0f) / 25.0f;
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 42.0f, 0.0f), glm::vec2(2000.0f, 150.0f), backgroundRects[counter++],
				glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, alpha));
		};
		tankView.each(tankBackground);
		auto lichBackground = [&](Transform& transform, LichComponent& lich)
		{
			float dotDist = glm::dot(transform.position - camTransform.position, camTransform.forward());
			float alpha = dotDist <= 100.0f ? 1.0f : std::max(125.0f - dotDist, 0.0f) / 25.0f;
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 28.0f, 0.0f), glm::vec2(1750.0f, 150.0f), backgroundRects[counter++],
				glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, alpha));
		};
		lichView.each(lichBackground);
		auto swarmBackground = [&](Transform& transform, SwarmComponent& swarm)
		{
			float dotDist = glm::dot(transform.position - camTransform.position, camTransform.forward());
			float alpha = dotDist <= 100.0f ? 1.0f : std::max(125.0f - dotDist, 0.0f) / 25.0f;
			uiRenderer->renderTexture(transform.position + glm::vec3(0.0f, 10.0f, 0.0f), glm::vec2(1500.0f, 150.0f), backgroundRects[counter++],
				glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, alpha));
		};
		swarmView.each(swarmBackground);


		counter = 0;
		uiRenderer->setTexture(healthBarID);
		auto tankHealthBar = [&](Transform& transform, TankComponent& tank)
		{
			float percentage = std::max((float)tank.life / tank.FULL_HEALTH, 0.0f);
			float dotDist = glm::dot(transform.position - camTransform.position, camTransform.forward());
			float alpha = dotDist <= 100.0f ? 1.0f : std::max(125.0f - dotDist, 0.0f) / 25.0f;
			float margin = 250.0f / dotDist;
			glm::vec2 pos = glm::vec2(backgroundRects[counter].x, backgroundRects[counter].y);
			glm::vec2 size = glm::vec2(backgroundRects[counter].z, backgroundRects[counter].w) - glm::vec2(margin);

			uiRenderer->renderTexture(glm::vec2(pos.x - size.x * 0.5f * (1.0f - percentage), pos.y), glm::vec2(size.x * percentage, size.y),
				glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, alpha));
			counter++;
		};
		tankView.each(tankHealthBar);
		auto lichHealthBar = [&](Transform& transform, LichComponent& lich)
		{
			float percentage = std::max((float)lich.life / lich.FULL_HEALTH, 0.0f);
			float dotDist = glm::dot(transform.position - camTransform.position, camTransform.forward());
			float alpha = dotDist <= 100.0f ? 1.0f : std::max(125.0f - dotDist, 0.0f) / 25.0f;
			float margin = 250.0f / dotDist;
			glm::vec2 pos = glm::vec2(backgroundRects[counter].x, backgroundRects[counter].y);
			glm::vec2 size = glm::vec2(backgroundRects[counter].z, backgroundRects[counter].w) - glm::vec2(margin);

			uiRenderer->renderTexture(glm::vec2(pos.x - size.x * 0.5f * (1.0f - percentage), pos.y), glm::vec2(size.x * percentage, size.y),
				glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, alpha));
			counter++;
		};
		lichView.each(lichHealthBar);
		auto swarmHealthBar = [&](Transform& transform, SwarmComponent& swarm)
		{
			float percentage = std::max((float)swarm.life / swarm.FULL_HEALTH, 0.0f);
			float dotDist = glm::dot(transform.position - camTransform.position, camTransform.forward());
			float alpha = dotDist <= 100.0f ? 1.0f : std::max(125.0f - dotDist, 0.0f) / 25.0f;
			float margin = 250.0f / dotDist;
			glm::vec2 pos = glm::vec2(backgroundRects[counter].x, backgroundRects[counter].y);
			glm::vec2 size = glm::vec2(backgroundRects[counter].z, backgroundRects[counter].w) - glm::vec2(margin);

			uiRenderer->renderTexture(glm::vec2(pos.x - size.x * 0.5f * (1.0f - percentage), pos.y), glm::vec2(size.x * percentage, size.y),
				glm::uvec4(0, 0, 1, 1), glm::vec4(1.0f, 1.0f, 1.0f, alpha));
			counter++;
		};
		swarmView.each(swarmHealthBar);

		this->backgroundRects.clear();
		return false;
	}
};
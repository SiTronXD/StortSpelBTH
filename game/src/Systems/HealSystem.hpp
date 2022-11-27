#pragma once

#include <vengine.h>
#include "../Components/Combat.h"
#include "../Components/HealArea.h"

class HealSystem : public System
{
private:
	int playerID;
	Scene* scene;
	std::vector<Entity> toRemove;
public:
	HealSystem(int playerID, Scene* scene)
		: playerID(playerID), scene(scene)
	{ }

	virtual ~HealSystem()
	{ }

	bool update(entt::registry& reg, float deltaTime) final
	{
		Transform& playerTransform = scene->getComponent<Transform>(playerID);
		Combat& combat = scene->getComponent<Combat>(playerID);

		auto view = reg.view<Transform, HealArea>();
		auto func = [&](const auto entity, Transform& transform, HealArea& area)
		{
			area.lifeTime -= deltaTime;
			area.accumulator += area.regen * deltaTime;
			glm::vec3 diff = playerTransform.position - transform.position;

			if (glm::dot(diff, diff) <= area.radius * area.radius) { combat.health = std::min(combat.health + (int)area.accumulator, combat.maxHealth); }
			if (area.accumulator >= 1.0f) { area.accumulator -= (int)area.accumulator; }
			if (area.lifeTime < 0.0f) { toRemove.push_back((Entity)entity); }
		};
		view.each(func);

		for (const auto& e : toRemove)
		{
			this->scene->removeEntity(e);
		}
		toRemove.clear();

		return false;
	}
};
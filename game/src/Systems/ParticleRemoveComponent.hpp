#pragma once

#include <vengine.h>
#include <unordered_map>

class ParticleRemoveComponent : public System
{
private:
	std::vector<Entity> toRemove;
	std::unordered_map<Entity, float> entityTimers;

	Scene* scene;

public:
	ParticleRemoveComponent(Scene* scene)
		: scene(scene)
	{ }

	virtual ~ParticleRemoveComponent()
	{ }

	bool update(entt::registry& reg, float deltaTime) final
	{
		// Loop through all particle systems
		auto view = reg.view<Transform, ParticleSystem>();
		auto func = [&](const auto entity, Transform& transform, ParticleSystem& particleSystem)
		{
			// Particle system components to remove
			if (strcmp(particleSystem.name, "SwarmPS") == 0 ||
				strcmp(particleSystem.name, "RmvComponent") == 0)
			{
				// Entity was recently created
				if (this->entityTimers.count((Entity)entity) <= 0)
				{
					this->entityTimers[(Entity)entity] = particleSystem.maxlifeTime;
				}

				// Decrease timer
				this->entityTimers[(Entity)entity] -= deltaTime;

				// Remove entity
				if (this->entityTimers[(Entity)entity] <= 0.0f)
				{
					this->entityTimers.erase((Entity)entity);
					this->toRemove.push_back((Entity)entity);
				}
			}
		};
		view.each(func);

		// Remove component from entities
		for (const auto& e : this->toRemove)
		{
			this->scene->removeComponent<ParticleSystem>(e);
		}
		this->toRemove.clear();

		return false;
	}
};
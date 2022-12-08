#pragma once

#include <vengine.h>
#include <vector>
#include <unordered_map>

class PerkAbilityOutsideRangeSystem : public System
{
private:
	std::vector<Entity> toActivate;
	std::vector<Entity> toDeactivate;

	const float MAX_DIST = 350.0f;

	Scene* scene;
	Transform* playerTransform;

	bool isInsideRange(const glm::vec3& otherPosition)
	{
		glm::vec3 deltaVec = playerTransform->position - otherPosition;

		return glm::dot(deltaVec, deltaVec) <= MAX_DIST * MAX_DIST;
	}

public:
	PerkAbilityOutsideRangeSystem(Scene* scene, Transform* playerTransform)
		: scene(scene), playerTransform(playerTransform)
	{ }

	virtual ~PerkAbilityOutsideRangeSystem()
	{ }

	bool update(entt::registry& reg, float deltaTime) final
	{
		// Loop through all active point lights
		auto deactivateView = reg.view<Transform, MeshComponent, PointLight>(entt::exclude<Inactive>);
		auto deactivateFunc = [&](const auto entity, Transform& transform, const MeshComponent& meshComp, PointLight& pointLight)
		{
			// Outside range
			if (!this->isInsideRange(transform.position))
			{
				this->toDeactivate.push_back((Entity) entity);
			}
		};
		deactivateView.each(deactivateFunc);

		// Loop through all deactivated point lights
		auto activateView = reg.view<Transform, MeshComponent, PointLight, Inactive>();
		auto activateFunc = [&](const auto entity, Transform& transform, const MeshComponent& meshComp, PointLight& pointLight, const Inactive& inactive)
		{
			// Inside range
			if (this->isInsideRange(transform.position))
			{
				this->toActivate.push_back((Entity)entity);
			}
		};
		activateView.each(activateFunc);

		// Deactivate
		for (const auto& e : this->toDeactivate)
		{
			this->scene->setInactive(e);
		}
		this->toDeactivate.clear();

		// Activate
		for (const auto& e : this->toActivate)
		{
			this->scene->setActive(e);
		}
		this->toActivate.clear();

		return false;
	}
};
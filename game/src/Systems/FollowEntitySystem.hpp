#pragma once

#include <vengine.h>

struct FollowEntity
{
	Entity entityToFollow = -1;
};

class FollowEntitySystem : public System
{
private:
	Scene* scene;

public:
	FollowEntitySystem(Scene* scene)
		: scene(scene)
	{ }

	virtual ~FollowEntitySystem()
	{ }

	bool update(entt::registry& reg, float deltaTime) final
	{
		// Follow entities
		auto followView = reg.view<Transform, FollowEntity>();
		auto followFunc = [&](
			const auto entity,
			Transform& transform,
			const FollowEntity& followEntityComponent)
		{
			// Make sure entity still is valid
			if (this->scene->entityValid(followEntityComponent.entityToFollow))
			{
				// Follow
				transform.position =
					this->scene->getComponent<Transform>(
						followEntityComponent.entityToFollow
						).position;
			}
		};
		followView.each(followFunc);

		return false;
	}
};
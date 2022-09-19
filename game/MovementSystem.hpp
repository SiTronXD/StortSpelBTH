#pragma once

#include <vengine.h>
#include "Movement.h"

class MovementSystem : public System
{
private:

	Scene* scene;
	float slowDown = 0.07f;

public:

	MovementSystem(Scene* scene) 
		:scene(scene)
	{
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Transform, Movement>();
		auto foo = [&](Transform& transform, Movement& movement)
		{
			move(movement, transform, deltaTime);
			rotate(movement, transform, deltaTime);
		};
		view.each(foo);

		return false;
	}

	void move(Movement& movement, Transform& transform, float& deltaTime)
	{
		movement.speed = 0.05f;
		movement.maxSpeed = 0.1f;
		movement.moveDir.y = (float)(Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
		movement.moveDir.x = (float)(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D));

		if (movement.currentSpeed.y > movement.maxSpeed)
		{
			movement.currentSpeed.y = movement.maxSpeed;
		}
		if (!Input::isKeyDown(Keys::W) && !Input::isKeyDown(Keys::S) && movement.currentSpeed.y != 0.f)
		{
			if (movement.currentSpeed.y > 0.f)
			{
				movement.currentSpeed.y -= slowDown * deltaTime;
			}
			else if (movement.currentSpeed.y < 0.f)
			{
				movement.currentSpeed.y += slowDown * deltaTime;
			}
		}
		else
		{
			movement.currentSpeed.y += movement.speed * movement.moveDir.y * deltaTime;
		}

		transform.position += (movement.currentSpeed.y * transform.up());
	};

	void rotate(Movement& movement, Transform& transform, float& deltaTime)
	{
		movement.turnSpeed = 200.f;

		if (transform.rotation.z > 359.5f && transform.rotation.z != 0.f)
		{
			transform.rotation.z = 0.f;
		}
		if (movement.moveDir.x > 0)
		{
			transform.rotation.z += movement.turnSpeed * deltaTime;
		}
		else if (movement.moveDir.x < 0)
		{
			transform.rotation.z -= movement.turnSpeed * deltaTime;
		}
	}
};
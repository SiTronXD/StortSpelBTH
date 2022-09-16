#pragma once

#include <vengine.h>
#include "Movement.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class MovementSystem : public System
{
private:

	Scene* scene;
	float slowDown = 0.07f;

public:

	MovementSystem(Scene* scene) :
		scene(scene)
	{
	}

	bool update(entt::registry& reg, float deltaTime) final
	{
		auto view = reg.view<Transform, Movement>();
		view.each([&](Transform& transform, Movement& movement)
			{
				move(movement, transform);
				rotate(movement, transform);
			});

		return false;
	}

	void move(Movement& movement, Transform& transform)
	{
		auto dT = Time::getDT();
		movement.speed = 0.05f;
		movement.maxSpeed = 0.1f;
		movement.moveDir.y = Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S);
		movement.moveDir.x = Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D);

		if (movement.currentSpeed.y > movement.maxSpeed)
		{
			movement.currentSpeed.y = movement.maxSpeed;
		}
		if (!Input::isKeyDown(Keys::W) && !Input::isKeyDown(Keys::S) && movement.currentSpeed.y != 0.f)
		{
			if (movement.currentSpeed.y > 0.f)
			{
				movement.currentSpeed.y -= slowDown * dT;
			}
			else if (movement.currentSpeed.y < 0.f)
			{
				movement.currentSpeed.y += slowDown * dT;
			}
		}
		else
		{
			movement.currentSpeed.y += movement.speed * movement.moveDir.y * dT;
		}

		transform.position += (movement.currentSpeed.y * transform.up());
	};

	void rotate(Movement& movement, Transform& transform)
	{
		auto dT = Time::getDT();
		movement.turnSpeed = 200.f;

		if (transform.rotation.z > 359.5f && transform.rotation.z != 0.f)
		{
			transform.rotation.z = 0.f;
		}
		if (movement.moveDir.x > 0)
		{
			transform.rotation.z += movement.turnSpeed * dT;
		}
		else if (movement.moveDir.x < 0)
		{
			transform.rotation.z -= movement.turnSpeed * dT;
		}
	}
};
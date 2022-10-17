#pragma once

#include <vengine.h>
#include "../Components/AiMovement.h"

class AiMovementSystem : public System {
private:
	Scene* scene;
    int ID;
public:
    AiMovementSystem(Scene* scene, int ID): scene(scene), ID(ID)
    {
        if (!scene->hasComponents<AiMovement>(ID)) {
            scene->setComponent<AiMovement>(ID);
        }
        AiMovement& movement = scene->getComponent<AiMovement>(ID);
        movement.maxSpeed = 50.f;
        movement.speedIncrease = 200.f;
        movement.slowDown = 180.f;
        movement.currentSpeed = glm::vec2(0.f);
        movement.turnSpeed = 200.f;
    }

    bool update(entt::registry& reg, float dt) final
    {
        auto view = reg.view<Transform, AiMovement>();
        auto moveLambda = [&](Transform& transform, AiMovement& movement) {
            move(movement, transform, dt);
            rotate(movement, transform, dt);
        };
        view.each(moveLambda);
        return false;
    }

    void move(AiMovement& movement, Transform& transform, float dt)
    {
        movement.currentSpeed.x = 0.f;
        //movement.moveDir.y = (float) MovementInput, how does AI move?

        if (movement.currentSpeed.y > movement.maxSpeed)
        {
            movement.currentSpeed.y = movement.maxSpeed;
        }
        if (!movement.moveDir.y && movement.currentSpeed.y != 0.f)
        {
            if (movement.currentSpeed.y < 0.001f && movement.currentSpeed.y > -0.001f)
            {
                movement.currentSpeed.y = 0.f;
            }
            else if (movement.currentSpeed.y > 0.f)
            {
                movement.currentSpeed.y -= movement.slowDown * dt;
            }
            else if (movement.currentSpeed.y < 0.f)
            {
                movement.currentSpeed.y += movement.slowDown * dt;
            }
        }
        else
        {
            movement.currentSpeed.y += movement.speedIncrease * movement.moveDir.y * dt;
        }

        if (movement.currentSpeed.y > movement.maxSpeed)
        {
            movement.currentSpeed.y = movement.maxSpeed;
        }
        else if (movement.currentSpeed.y < -movement.maxSpeed)
        {
            movement.currentSpeed.y = -movement.maxSpeed;
        }

        transform.position += (movement.currentSpeed.y * transform.up()) * dt;
    }

    void rotate(AiMovement& movement, Transform& transform, float dt)
    {
        //movement.moveDir.x = (float) Input? How does AI move?

        if (transform.rotation.y > 359.5f && transform.rotation.y != 0.f)
        {
            transform.rotation.y = 0.f;
        }
        if (movement.moveDir.x > 0)
        {
            transform.rotation.y += movement.turnSpeed * dt;
        }
        else if (movement.moveDir.x < 0)
        {
            transform.rotation.y -= movement.turnSpeed * dt;
        }
    }
};
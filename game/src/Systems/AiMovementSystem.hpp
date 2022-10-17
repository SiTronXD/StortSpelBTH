#pragma once

#include <vengine.h>
#include "../Components/AiMovement.h"

#include <chrono>

class AiMovementSystem : public System {
private:
	Scene* scene;
    int ID;
    float distance;
    std::chrono::time_point<std::chrono::system_clock> timer;
    std::chrono::duration<float> durTimer;
public:
    AiMovementSystem(Scene* scene, int ID): scene(scene), ID(ID)
    {
        if (!scene->hasComponents<AiMovement>(ID)) {
            scene->setComponent<AiMovement>(ID);
        }
        AiMovement& movement = scene->getComponent<AiMovement>(ID);
        movement.maxSpeed = 100.f;
        movement.speedIncrease = 50.f;
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
        Transform playerTrans = scene->getComponent<Transform>(4);
        distance = glm::length(transform.position - playerTrans.position);
        std::cout << distance << std::endl;

        if (distance <= 10.f)
        {
            movement.currentSpeed.y = 0.f;
        }

        movement.currentSpeed.y += movement.speedIncrease * dt;

        if (movement.currentSpeed.y > movement.maxSpeed)
        {
            movement.currentSpeed.y = movement.maxSpeed;
        }
        else if (movement.currentSpeed.y < -movement.maxSpeed)
        {
            movement.currentSpeed.y = -movement.maxSpeed;
        }
        std::cout << movement.currentSpeed.y << std::endl;
        glm::vec3 hej = glm::normalize(playerTrans.position - transform.position);

        transform.position += (hej * movement.currentSpeed.y) * dt;
    }

    void rotate(AiMovement& movement, Transform& transform, float dt)
    {
        //movement.moveDir.x = (float) Input? How does AI move?
        Transform playerTrans = scene->getComponent<Transform>(4);
        distance = glm::abs(glm::length(transform.position - playerTrans.position));

        if (distance > 50.f)
        {
            transform.rotation.x = playerTrans.rotation.x;
        }
    }
};
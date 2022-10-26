#pragma once

#include <vengine.h>
#include "../Components/AiMovement.h"

#include <chrono>

class AiMovementSystem : public System {
private:
	SceneHandler* sceneHandler;
    Scene* scene;
    int playerID = -1;
public:
    AiMovementSystem(SceneHandler* sceneHandler, int playerID): sceneHandler(sceneHandler), playerID(playerID)
    {
        scene = sceneHandler->getScene();
        auto view = scene->getSceneReg().view<AiMovement>();
        auto movementLambda = [&](AiMovement& movement) {
            movement.maxSpeed = 50.f;
            movement.speedIncrease = 10.f;
            movement.slowDown = 180.f;
            movement.currentSpeed = glm::vec2(0.f);
            movement.turnSpeed = 200.f;
        };
        view.each(movementLambda);
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
        Transform playerTrans = scene->getComponent<Transform>(playerID);
        if (transform.position != playerTrans.position)
        {
            movement.distance = glm::length(transform.position - playerTrans.position);
            if (movement.distance <= 100.f)
            {
                if (movement.distance <= 8.f)
                {
                    movement.currentSpeed.y = 0.f;
                }
                else
                {
                    movement.currentSpeed.y += movement.speedIncrease * dt;

                    if (movement.currentSpeed.y > movement.maxSpeed)
                    {
                        movement.currentSpeed.y = movement.maxSpeed;
                    }
                    else if (movement.currentSpeed.y < -movement.maxSpeed)
                    {
                        movement.currentSpeed.y = -movement.maxSpeed;
                    }
                }
                movement.moveDir = glm::normalize(playerTrans.position - transform.position);
                transform.position += (movement.moveDir * movement.currentSpeed.y) * dt;
            }
        }
    }

    void rotate(AiMovement& movement, Transform& transform, float dt)
    {
        if (movement.distance < 100.f)
        {
            Transform playerTrans = scene->getComponent<Transform>(playerID);
            if (transform.position != playerTrans.position)
            {
                float posX = transform.position.x - playerTrans.position.x;
                float posZ = transform.position.z - playerTrans.position.z;
                float angle = atan2(posX, posZ);
                angle = glm::degrees(angle);
                transform.rotation.y = angle;
            }
        }
    }
};
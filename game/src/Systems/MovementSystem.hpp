#pragma once

#include <vengine.h>
#include "../Components/Movement.h"

class MovementSystem: public System 
{
private:

    Scene* scene;
    int playerID;

public:

    MovementSystem(Scene* scene, int playerID): scene(scene), playerID(playerID)
    {
        if (!scene->hasComponents<Movement>(playerID)) 
        {
            scene->setComponent<Movement>(playerID);
        }

        Movement& movement     = scene->getComponent<Movement>(playerID);
        movement.maxSpeed      = 70.f;//250.f;
        movement.speedIncrease = 200.f;
        movement.slowDown      = 180.f;
        movement.currentSpeed  = glm::vec2(0.f);
        movement.turnSpeed     = 200.f;
    }

    bool update(entt::registry& reg, float deltaTime) final
    {
        // Reference currently only used for ImGui
        Movement& movement = scene->getComponent<Movement>(playerID);

        static bool moveSystem0 = false;

        if (ImGui::Begin("Movement")) {
            ImGui::PushItemWidth(-100.f);
            ImGui::Text("Player");

            ImGui::DragFloat("Speed", &movement.maxSpeed, 0.05f, 0.f, 500.f);

            if (!ImGui::Checkbox("Move system switch", &moveSystem0))
            {
                ImGui::DragFloat("Speed inc", &movement.speedIncrease, 0.05f);
                ImGui::DragFloat("Slow down", &movement.slowDown, 0.05f, 0.0001f);
            }

            ImGui::Text("Z,X Speed: (%f, %f)", movement.currentSpeed.y, movement.currentSpeed.x);

            glm::vec3 playerPos = scene->getComponent<Transform>(playerID).position;
            ImGui::Text("Pos: (%f, %f, %f)", playerPos.x, playerPos.y, playerPos.z);

            ImGui::Separator();
            ImGui::PopItemWidth();
        }
        ImGui::End();


        Transform& camTra = scene->getComponent<Transform>(scene->getMainCameraID());

        if (moveSystem0) 
        {
            auto playerView = reg.view<Transform, Movement>();
            auto moveLambda = [&](Transform& transform, Movement& movement) {
                move(movement, transform, deltaTime);
                rotate(movement, transform, deltaTime);
            };
            playerView.each(moveLambda);
        }
        else 
        {
            auto playerView = reg.view<Transform, Movement>();
            auto moveLambda = [&](Transform& transform, Movement& movement) {
                move2(movement, transform, camTra, deltaTime);
                rotate2(movement, transform, camTra, deltaTime);
            };
            playerView.each(moveLambda);
        }
        
        return false;
    }

    void move(Movement& movement, Transform& transform, float deltaTime)
    {
        movement.currentSpeed.x = 0.f;
        movement.moveDir.y = (float)(Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));

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
                movement.currentSpeed.y -= movement.slowDown * deltaTime;
            }
            else if (movement.currentSpeed.y < 0.f) 
            {
                movement.currentSpeed.y += movement.slowDown * deltaTime;
            }
        }
        else 
        {
            movement.currentSpeed.y += movement.speedIncrease * movement.moveDir.y * deltaTime;
        }

        if (movement.currentSpeed.y > movement.maxSpeed) 
        {
            movement.currentSpeed.y = movement.maxSpeed;
        }
        else if (movement.currentSpeed.y < -movement.maxSpeed)
        {
            movement.currentSpeed.y = -movement.maxSpeed;
        }

        transform.position += (movement.currentSpeed.y * transform.up()) * deltaTime;
    };

    void rotate(Movement& movement, Transform& transform, float deltaTime)
    {
        movement.moveDir.x = (float)(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D));

        if (transform.rotation.y > 359.5f && transform.rotation.y != 0.f)
        {
            transform.rotation.y = 0.f;
        }
        if (movement.moveDir.x > 0)
        {
            transform.rotation.y += movement.turnSpeed * deltaTime;
        }
        else if (movement.moveDir.x < 0)
        {
            transform.rotation.y -= movement.turnSpeed * deltaTime;
        }
    }


    void move2(Movement& movement, Transform& transform, Transform& camTransform, float deltaTime)
    {
        movement.moveDir.y = (float)(Input::isKeyDown(Keys::W) - Input::isKeyDown(Keys::S));
        movement.moveDir.x = (float)(Input::isKeyDown(Keys::A) - Input::isKeyDown(Keys::D));

        movement.currentSpeed.x = movement.moveDir.x * movement.maxSpeed;
        movement.currentSpeed.y = movement.moveDir.y * movement.maxSpeed;

        if (movement.moveDir.x && movement.moveDir.y)
        {
            movement.currentSpeed = glm::normalize(movement.currentSpeed) * movement.maxSpeed;
        }

        Transform& camTra = scene->getComponent<Transform>(scene->getMainCameraID());
        const glm::vec3 camFwd = camTra.forward();

        glm::vec3 moveFwd = camFwd;
        moveFwd.y = 0.f;

        // += forward + side
        transform.position +=
            glm::normalize(moveFwd) * (movement.currentSpeed.y * deltaTime) +
            glm::normalize(glm::cross(camTra.up(), camFwd)) * (movement.currentSpeed.x * deltaTime); 
    }

    void rotate2(Movement& movement, Transform& transform, Transform& camTransform, float deltaTime)
    {
        // +180.f current cuz model is flipped wrong way ):<

        if (movement.moveDir.y > 0.f)
        {
            transform.rotation.y = (camTransform.rotation.y + 180.f) + 45.f * movement.moveDir.x;
        }

        else if (movement.moveDir.y < 0.f)
        {
            transform.rotation.y = (camTransform.rotation.y) - 45.f * movement.moveDir.x;
        }

        else if (movement.moveDir.x > 0.f) 
        {
            transform.rotation.y = camTransform.rotation.y + 90.f;
        }
        
        else if (movement.moveDir.x < 0.f)
        {
            transform.rotation.y = camTransform.rotation.y - 90.f;
        }
    }
};
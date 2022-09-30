#pragma once

#include <vengine.h>
#include "../Components/CameraMovement.h"
#include "glm/gtc/quaternion.hpp";

class CameraMovementSystem: public System {
  private:

    Scene* scene;

    static glm::vec3 GetRandVec(float scalar)
    {
        return glm::vec3((rand() % 200) * 0.01f - 1.f * scalar,
                         ((rand() % 200) * 0.01f - 1.f) * scalar,
                         ((rand() % 200) * 0.01f - 1.f) * scalar);
    }

    glm::vec3 qrot(glm::vec4 q, glm::vec3 v)
    {
        return v + (2.f * glm::cross(glm::vec3(q.x, q.y, q.z), glm::cross(glm::vec3(q.x, q.y, q.z), v) + q.w * v));
    }

  public:

      CameraMovementSystem(Scene* scene, int playerID) : scene(scene) {
          if (scene->hasComponents<CameraMovement>(playerID)) {
              CameraMovement& cameraMovement = scene->getComponent<CameraMovement>(playerID);
          }
      }

      bool update(entt::registry& reg, float deltaTime) final {
          auto view = reg.view<Transform, CameraMovement>();
          auto foo  = [&](Transform& camTransform, Transform& playerTransform, CameraMovement& cameraMovement) {
              basicMovement(playerTransform, deltaTime);
          };
          view.each(foo);
          return false;
      }

      void basicMovement(Transform& playerTransform, float deltaTime)
      {
          static float speed = 20.f;

          if (ImGui::Begin("yas")) {
              ImGui::PushItemWidth(-100.f);

              ImGui::DragFloat("Speed", &speed, 0.01f, 0.f, 20.f);

              ImGui::PopItemWidth();
          }
          ImGui::End();

          const float ZInput     = Input::isKeyDown(Keys::W)   ? 1.f
                                   : Input::isKeyDown(Keys::S) ? -1.f
                                                               : 0.f;
          const float XInput     = Input::isKeyDown(Keys::D)   ? 1.f
                                   : Input::isKeyDown(Keys::A) ? -1.f
                                                               : 0.f;
          const float frameSpeed = speed * deltaTime;

          glm::vec3 yasFwd = playerTransform.up();
          yasFwd.y         = 0.f;
          yasFwd           = glm::normalize(yasFwd) * (frameSpeed * ZInput);

          glm::vec3 yasSide = glm::normalize(glm::cross(playerTransform.forward(), playerTransform.up())) *
                              (frameSpeed * XInput);

          playerTransform.position += yasFwd + yasSide;
      }

      void camMovement(CameraMovement& camMovement, Transform& transform, float deltaTime) {
          if (ImGui::Begin("yas")) {
              ImGui::PushItemWidth(-100.f);

              ImGui::DragFloat("Sens", &camMovement.sens, 0.01f, 0.f, 50.f);
              ImGui::DragFloat("Cam Distance", &camMovement.camDist, 0.01f);
              ImGui::DragFloat("Cam Height", &camMovement.camHeight, 0.01f);

              ImGui::DragFloat("Max Rotation", &camMovement.maxXRot, 0.001f);
              ImGui::DragFloat("Min Rotation", &camMovement.minXRot, 0.001f);

              ImGui::DragFloat("Shake Scalar", &camMovement.shakeScalar, 0.0005f, 0.f, 20.f);
              ImGui::DragFloat("Shake Duration", &camMovement.shakeDuration, 0.001f);


              ImGui::PopItemWidth();
          }
          ImGui::End();

          if (Input::isKeyDown(Keys::Q) && !camMovement.shaking) camMovement.shaking = true;

          const float XInput = Input::getMouseDeltaX();
          const float YInput = Input::getMouseDeltaY();

          glm::vec2 camRot;
          camRot.x += camMovement.sens * YInput * deltaTime;
          camRot.y += camMovement.sens * XInput * deltaTime;

          printf("DT: %f\nX Input: %f\nY Input: %f\n", deltaTime, XInput, YInput);

          if (camRot.x >= camMovement.maxXRot) camRot.x = camMovement.maxXRot;
          else if (camRot.x <= camMovement.minXRot)
              camRot.x = camMovement.minXRot;

          //printf("X Rot: %f\nY Rot: %f\n\n", camRot.x, camRot.y);

          glm::vec3 targetPos = transform.position;
          targetPos.y += camMovement.camHeight;

          if (camMovement.shaking) {
              camMovement.shakeTimer += deltaTime;
              if (camMovement.shakeTimer >= camMovement.shakeDuration) {
                  camMovement.shakeTimer = 0.f;
                  camMovement.shaking    = false;
              }
              else {
                  targetPos = targetPos + GetRandVec(camMovement.shakeScalar);
              }
          }

          glm::quat vector = glm::vec3(camRot.x, camRot.y, 0.f);
          glm::vec4 hej              = { vector.x, vector.y, vector.z, vector.w };
          glm::vec3 fwd    = glm::normalize(qrot(hej, transform.forward()));
          glm::vec3 up               = glm::normalize(qrot(hej, transform.up()));
          glm::vec3 pos = targetPos + (transform.forward() * -camMovement.camDist);

          /*printf("Tar: %f, %f, %f\nCam: %f, %f, %f\n\n",
        tra.position.x, tra.position.y, tra.position.z,
        pos.m128_f32[0], pos.m128_f32[1], pos.m128_f32[2]);*/
      }
};

#pragma once

#include <vengine.h>
#include "../Components/CameraMovement.h"
#include "glm/gtc/quaternion.hpp";

// temp
constexpr float RADIAN = 57.2957795f;

class CameraMovementSystem: public System 
{
private:

    //SceneHandler* hScene;
    Scene* scene;
    int playerID;

    static glm::vec3 GetRandVec(float scalar)
    {
        return glm::vec3((rand() % 200) * 0.01f - 1.f * scalar,
                         ((rand() % 200) * 0.01f - 1.f) * scalar,
                         ((rand() % 200) * 0.01f - 1.f) * scalar);
    }

    static glm::vec3 qrot(glm::vec4 q, glm::vec3 v)
    {
        return v + (2.f * glm::cross(glm::vec3(q.x, q.y, q.z), glm::cross(glm::vec3(q.x, q.y, q.z), v) + q.w * v));
    }

public:

      CameraMovementSystem(Scene* scene, int playerID): scene(scene), playerID(playerID)
      {
          auto q = scene->getComponent<Transform>(scene->getMainCameraID());
      }

      bool update(entt::registry& reg, float deltaTime) final {

          basicMovement(deltaTime);
          camMovement(deltaTime);

          return false;
      }

      void basicMovement(float deltaTime)
      {
          static float speed = 20.f;

          if (ImGui::Begin("yas")) {
              ImGui::PushItemWidth(-100.f);

              ImGui::DragFloat("Speed", &speed, 0.01f, 0.f, 20.f);

              ImGui::PopItemWidth();
          }
          ImGui::End();

          const float ZInput = Input::isKeyDown(Keys::W) ? 1.f : Input::isKeyDown(Keys::S) ? -1.f : 0.f;
          const float XInput = Input::isKeyDown(Keys::D) ? 1.f : Input::isKeyDown(Keys::A) ? -1.f : 0.f;
          const float frameSpeed = speed * deltaTime;

          Transform&      camTra = scene->getComponent<Transform>(scene->getMainCameraID());
          const glm::vec3 camFwd = camTra.forward();
          const glm::vec3 camUp  = camTra.up();

          glm::vec3 yasFwd  = camFwd;
          yasFwd.y = 0.f;
          yasFwd = glm::normalize(yasFwd) * (frameSpeed * ZInput);

          glm::vec3 finalVec = glm::normalize(glm::cross(camFwd, camUp)) * (frameSpeed * XInput) + yasFwd;

          scene->getComponent<Transform>(playerID).position += finalVec;
      }

      void camMovement(float deltaTime)
      {
          Transform& camTransform = scene->getComponent<Transform>(scene->getMainCameraID());
          CameraMovement& camMovement = scene->getComponent<CameraMovement>(scene->getMainCameraID()); 

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

          if (Input::isKeyDown(Keys::Q) && !camMovement.shaking) {
              camMovement.shaking = true;
          }

          //const float XInput = Input::getMouseDeltaX();
          //const float YInput = Input::getMouseDeltaY();
          
          // temp input
          const float XInput = Input::isKeyDown(Keys::LEFT) ? 1.f : Input::isKeyDown(Keys::RIGHT) ? -1.f : 0.f;
          const float YInput = Input::isKeyDown(Keys::UP)   ? 1.f : Input::isKeyDown(Keys::DOWN)  ? -1.f : 0.f;
          
          camMovement.camRot.x += camMovement.sens * YInput * deltaTime;
          camMovement.camRot.y += camMovement.sens * XInput * deltaTime;

          // Temp debug
          if (Input::isKeyDown(Keys::E)) {
              camMovement.camRot.x = 0.f; 
              camMovement.camRot.y = 0.f;
          }

          if (camMovement.camRot.x >= camMovement.maxXRot) {
              camMovement.camRot.x = camMovement.maxXRot;
          }
          else if (camMovement.camRot.x <= camMovement.minXRot) {
              camMovement.camRot.x = camMovement.minXRot;
          }


          glm::vec3 targetPos = scene->getComponent<Transform>(playerID).position;
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

          glm::quat quat = glm::quat(camMovement.camRot);
          glm::vec3 fwd = glm::normalize(qrot(glm::vec4(quat.x, quat.y, quat.z, quat.w), glm::vec3(0.f, 0.f, 1.f)));

          camTransform.position = targetPos + (fwd * -camMovement.camDist);
          camTransform.rotation = glm::eulerAngles(quat) * RADIAN;

          //printf("Fwd: (%f, %f, %f)\n", fwd.x, fwd.y, fwd.z);
          //printf("Vec rot: (%f, %f)\n", camMovement.camRot.x, camMovement.camRot.y);
          printf("Cam rot: (%f, %f, %f)\n", camTransform.rotation.x, camTransform.rotation.y, camTransform.rotation.z);
          //printf("Cam pos: (%f, %f, %f)\n", camTransform.position.x, camTransform.position.y, camTransform.position.z);
          //printf("Tar pos: (%f, %f, %f)\n", targetPos.x, targetPos.y, targetPos.z);
          //printf("Cam dist: %f\n", glm::distance(pos, targetPos));

          //printf("\n");
      }
};

#pragma once

#include <vengine.h>
#include "../Components/CameraMovement.h"
#include "glm/gtc/quaternion.hpp";
#include "glm/gtx/euler_angles.hpp"
#include "vengine/VengineMath.hpp"

class CameraMovementSystem: public System 
{
private:

    Scene* scene;
    int playerID;

public:

      CameraMovementSystem(Scene* scene, int playerID): scene(scene), playerID(playerID)
      {
      }

      bool update(entt::registry& reg, float deltaTime) final 
      {
          camMovement(deltaTime);

          return false;
      }

      void camMovement(float deltaTime)
      {
          Transform& camTransform = scene->getComponent<Transform>(scene->getMainCameraID());
          CameraMovement& camMovement = scene->getComponent<CameraMovement>(scene->getMainCameraID()); 

          if (ImGui::Begin("Movement")) 
          {
              ImGui::PushItemWidth(-100.f);
              ImGui::Text("Camera (Shake key: Q)");

              ImGui::DragFloat("Sens", &camMovement.sens, 0.01f, 0.f, 50.f);
              ImGui::DragFloat("Distance", &camMovement.camDist, 0.01f);
              ImGui::DragFloat("Height", &camMovement.camHeight, 0.01f);

              ImGui::DragFloat("Max Rotation", &camMovement.maxXRot, 0.001f);
              ImGui::DragFloat("Min Rotation", &camMovement.minXRot, 0.001f);

              ImGui::DragFloat("Shake Scalar", &camMovement.shakeScalar, 0.0005f, 0.f, 20.f);
              ImGui::DragFloat("Shake Duration", &camMovement.shakeDuration, 0.001f);

              ImGui::Separator();
              ImGui::PopItemWidth();
          }
          ImGui::End();


          if (Input::isKeyDown(Keys::Q))
          {
              camMovement.shaking = true;
          }

          const float XInput = Input::isKeyDown(Keys::LEFT) ? 1.f : Input::isKeyDown(Keys::RIGHT) ? -1.f : 0.f;
          const float YInput = Input::isKeyDown(Keys::DOWN) ? 1.f : Input::isKeyDown(Keys::UP)  ? -1.f : 0.f;

          camMovement.camRot.x += camMovement.sens * YInput * deltaTime;
          camMovement.camRot.y += camMovement.sens * XInput * deltaTime;

          if (camMovement.camRot.x >= camMovement.maxXRot)
          {
              camMovement.camRot.x = camMovement.maxXRot;
          }
          else if (camMovement.camRot.x <= camMovement.minXRot) 
          {
              camMovement.camRot.x = camMovement.minXRot;
          }

          glm::vec3 targetPos = scene->getComponent<Transform>(playerID).position;
          targetPos.y += camMovement.camHeight;

          if (camMovement.shaking) 
          {
              camMovement.shakeTimer += deltaTime;
              if (camMovement.shakeTimer >= camMovement.shakeDuration)
              {
                  camMovement.shakeTimer = 0.f;
                  camMovement.shaking = false;
              }
              else 
              {
                  targetPos = targetPos + SMath::getRandomVector(camMovement.shakeScalar);
              }
          }

          const glm::quat quat = glm::quat(camMovement.camRot);
          const glm::vec3 scaledFwd = glm::normalize(SMath::rotateVectorByQuaternion(quat, glm::vec3(0.f, 0.f, 1.f))) * -camMovement.camDist;

          camTransform.position = targetPos + scaledFwd;

          camTransform.rotation.y = camMovement.camRot.y * (180.f / (float)M_PI);
          camTransform.rotation.x = camMovement.camRot.x * (180.f / (float)M_PI);
      }
};
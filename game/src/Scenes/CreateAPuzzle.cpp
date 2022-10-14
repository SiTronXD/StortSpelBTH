#include "CreateAPuzzle.h"

#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"

// decreaseFps used for testing game with different framerates

PuzzleCreator::PuzzleCreator() : camEntity(-1) {}

PuzzleCreator::~PuzzleCreator() {}

void PuzzleCreator::init()
{
  camEntity = this->createEntity();
  this->setComponent<Camera>(this->camEntity, 1.0f);
  this->setComponent<CameraMovement>(this->camEntity);
  this->setMainCamera(this->camEntity);
  Transform& camTransform = this->getComponent<Transform>(this->camEntity);
  camTransform.position = glm::vec3(1.0f);

  //ground
  this->ground = this->createEntity();
  int groundMesh =
      this->getResourceManager()->addMesh("vengine_assets/models/Cube.fbx");

  this->setComponent<Transform>(this->ground);
  this->setComponent<MeshComponent>(this->ground, groundMesh);
  Transform& transform2 = this->getComponent<Transform>(this->ground);
  transform2.position = glm::vec3(0.0f, -10.0f, 0.0f);
  transform2.scale = glm::vec3(100.f, 0.1f, 100.f);

  this->puzzleLoader = this->createEntity();
  this->setScriptComponent(puzzleLoader, "src/Scripts/PuzzleLoader.lua");

  this->puzzleCreator = this->createEntity();
  this->setScriptComponent(puzzleCreator, "src/Scripts/PuzzleCreatorLua.lua");

  this->player = this->createEntity();
  this->setComponent<MeshComponent>(this->player);
  this->setComponent<Movement>(this->player);
  this->setComponent<Combat>(this->player);
  Transform& transform = this->getComponent<Transform>(this->player);
  transform.position = glm::vec3(0.0f, 0.0f, 20.0f);
  transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
  transform.scale = glm::vec3(5.0f);
  this->createSystem<CombatSystem>(this, player);
  this->createSystem<MovementSystem>(this, this->player);
  
  this->createSystem<CameraMovementSystem>(this, this->player);
}

static int a;  //change this later

void PuzzleCreator::update()
{
  /*
  if (ImGui::Begin("Add prefabs"))
    {
  
      if (a < objectNames.size())
        {
          ImGui::InputInt(("Add " + objectNames[a]).c_str(), &a);
        }
      else
        {
          ImGui::InputInt("Add ...", &a);
        }
  
      for (size_t i = 0; i < objectID.size(); i++)
        {
          const std::string treeID = "Prefab nr " + std::to_string(i);
          if (ImGui::TreeNode(treeID.c_str()))
            {
              if (ImGui::TreeNode("Position"))
                {
                  ImGui::InputFloat(
                      "X",
                      &this->getComponent<Transform>(objectID[i]).position.x
                  );
                  ImGui::InputFloat(
                      "Y",
                      &this->getComponent<Transform>(objectID[i]).position.y
                  );
                  ImGui::InputFloat(
                      "Z",
                      &this->getComponent<Transform>(objectID[i]).position.z
                  );
                  ImGui::TreePop();
                }
              if (ImGui::TreeNode("Rotation"))
                {
                  ImGui::SliderFloat(
                      "X",
                      &this->getComponent<Transform>(objectID[i]).rotation.x,
                      -360,
                      360
                  );
                  ImGui::SliderFloat(
                      "Y",
                      &this->getComponent<Transform>(objectID[i]).rotation.y,
                      -360,
                      360
                  );
                  ImGui::SliderFloat(
                      "Z",
                      &this->getComponent<Transform>(objectID[i]).rotation.z,
                      -360,
                      360
                  );
                  ImGui::TreePop();
                }
              if (ImGui::TreeNode("Size"))
                {
                  ImGui::InputFloat(
                      "X", &this->getComponent<Transform>(objectID[i]).scale.x
                  );
                  ImGui::InputFloat(
                      "Y", &this->getComponent<Transform>(objectID[i]).scale.y
                  );
                  ImGui::InputFloat(
                      "Z", &this->getComponent<Transform>(objectID[i]).scale.z
                  );
                  ImGui::TreePop();
                }
              if (ImGui::Button("Delete"))
                {
                  this->removeEntity(objectID[i]);
                  objectID.erase(objectID.begin() + i);
                }
              ImGui::TreePop();
            }
        }
      if (ImGui::Button("Save"))
        {
          std::cout << "Name of Puzzle:" << std::endl;
          std::cin >> puzzleName;
          if (puzzleName == "")
            {
              puzzleName = "puzzle_T";
            }
          createPuzzel(this, &objectID, &meshes, puzzleName);
        }
    }
  ImGui::End();*/
}
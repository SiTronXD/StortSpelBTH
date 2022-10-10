#include "CreateAPuzzle.h"

#include "../FileMaker.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"

// decreaseFps used for testing game with different framerates

PuzzleCreator::PuzzleCreator() : camEntity(-1) {}

PuzzleCreator::~PuzzleCreator() {}

void PuzzleCreator::init()
{
  //also need to add collision boxes and others
  PuzzleCreator::getObjectFileNames();

  //Filesystem
  //puzzleObject = readPuzzle("YesaTestPuzzle");
  //puzzleObject.offset = glm::vec2(20, 0);
  //puzzleObject.addToScene(this, objectID);
  
  //ground
  this->ground = this->createEntity();
  
  if (meshes.find("Cube.fbx") == meshes.end())
    {
      meshes.insert({
        "Cube.fbx", this->getResourceManager()->addMesh("vengine_assets/models/Cube.fbx")}
      );
    }
  this->setComponent<MeshComponent>(
      this->ground, meshes.find("Cube.fbx")->second
  );
  Transform& transform2 = this->getComponent<Transform>(this->ground);
  transform2.position = glm::vec3(0.0f, -10.0f, 0.0f);
  transform2.scale = glm::vec3(100.f, 0.1f, 100.f);
  
  //player
  this->mover = this->createEntity();
  this->setComponent<MeshComponent>(this->mover);
  this->setComponent<Movement>(this->mover);
  Transform& transform = this->getComponent<Transform>(this->mover);
  transform.position = glm::vec3(0.0f, 0.0f, 20.0f);
  transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
  transform.scale = glm::vec3(5.0f);
  this->createSystem<MovementSystem>(this, this->mover);
  
  //camera
  this->camEntity = this->createEntity();
  this->setComponent<Camera>(this->camEntity, 1.0f);
  this->setComponent<CameraMovement>(this->camEntity);
  this->setMainCamera(this->camEntity);
  Transform& camTransform = this->getComponent<Transform>(this->camEntity);
  camTransform.position = glm::vec3(1.0f);
  
  this->createSystem<CameraMovementSystem>(this, this->mover);
}

static int a;  //change this later

void PuzzleCreator::update()
{
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
  ImGui::End();
  //if (a > meshes.size() - 1)
  //  {
  //    return;
  //  }
  //if (Input::isKeyPressed(Keys::B))
  //  {
  //    int temp = this->createEntity();
  //    objectID.push_back(temp);
  //
  //    if (meshes.find(objectNames[a]) == meshes.end())
  //      {
  //        meshes.insert(
  //            {objectNames[a],
  //             this->getResourceManager()->addMesh(objectFilePath[a].c_str())}
  //        );
  //      }
  //    this->setComponent<MeshComponent>(
  //        temp, meshes.find(objectNames[a])->second
  //    );
  //
  //    Transform& transform = this->getComponent<Transform>(temp);
  //    transform.position = this->getComponent<Transform>(this->mover).position;
  //    transform.scale = glm::vec3(1.f, 1.f, 1.f);
  //  }
}

/*#include <filesystem>
void PuzzleCreator::getObjectFileNames()
{
  std::string path = "vengine_assets/models";
  for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      std::string input = entry.path().generic_string();

      if (input.substr(input.size() - 3) == "obj" ||
          input.substr(input.size() - 3) == "fbx")
        {
          objectFilePath.push_back(input);
          input = input.substr(input.find_last_of("/\\") + 1);
          objectNames.push_back(input);
        }
    }
}*/
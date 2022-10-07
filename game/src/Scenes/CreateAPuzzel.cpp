#include "CreateAPuzzel.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"

// decreaseFps used for testing game with different framerates

PuzzelCreator::PuzzelCreator(): camEntity(-1) {}

PuzzelCreator::~PuzzelCreator() {}

void PuzzelCreator::init()
{
    //ground
    this->ground = this->createEntity();
    this->setComponent<MeshComponent>(this->ground);
    Transform& transform2 = this->getComponent<Transform>(this->ground);
    transform2.position   = glm::vec3(0.0f, -10.0f, 0.0f);
    transform2.scale      = glm::vec3(100.f, 0.1f, 100.f);

    //player
    this->mover = this->createEntity();
    this->setComponent<MeshComponent>(this->mover);
    this->setComponent<Movement>(this->mover);
    Transform& transform = this->getComponent<Transform>(this->mover);
    transform.position   = glm::vec3(0.0f, 0.0f, 20.0f);
    transform.rotation   = glm::vec3(-90.0f, 0.0f, 0.0f);
    transform.scale      = glm::vec3(5.0f);
    this->createSystem<MovementSystem>(this, this->mover);

    //camera
    this->camEntity = this->createEntity();
    this->setComponent<Camera>(this->camEntity, 1.0f);
    this->setComponent<CameraMovement>(this->camEntity);
    this->setMainCamera(this->camEntity);
    Transform& camTransform = this->getComponent<Transform>(this->camEntity);
    camTransform.position   = glm::vec3(1.0f);

    this->createSystem<CameraMovementSystem>(this, this->mover);

    for (int i = 0; i < 3; i++) {
            this->puzzelObjects.push_back(this->createEntity());
            std::string penist = "Cube.fbx";
        
            this->setComponent<MeshComponent>(this->puzzelObjects[this->puzzelObjects.size() - 1]);
                this->getComponent<MeshComponent>(this->puzzelObjects[this->puzzelObjects.size() - 1]);
            memcpy(m.filePath, penist.c_str(), sizeof(m.filePath));
            std::cout << m.filePath << std::endl;
        
            Transform& transform = this->getComponent<Transform>(this->puzzelObjects[this->puzzelObjects.size() - 1]);
            transform.position   = this->getComponent<Transform>(this->mover).position;
            transform.position.z += i * 5;
            transform.scale      = glm::vec3(5.f, 1.f, 1.f);
    }
}

void PuzzelCreator::update() 
{
    static int a;//change this later
    if (ImGui::Begin("Add prefabs")) 
    {
        for (size_t i = 0; i < puzzelObjects.size(); i++) {
            const std::string treeID = "Prefab nr " + std::to_string(i);
            if (ImGui::TreeNode(treeID.c_str())) {
                ImGui::Text("Type");
                ImGui::InputInt("Loops", &a);

                if (ImGui::TreeNode("Position")) 
                {
                    ImGui::InputFloat("X",&this->getComponent<Transform>(puzzelObjects[i]).position.x);
                    ImGui::InputFloat("Y",&this->getComponent<Transform>(puzzelObjects[i]).position.y);
                    ImGui::InputFloat("Z",&this->getComponent<Transform>(puzzelObjects[i]).position.z);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Rotation")) {
                    ImGui::SliderFloat("X",&this->getComponent<Transform>(puzzelObjects[i]).rotation.x,-360,360);
                    ImGui::SliderFloat("Y",&this->getComponent<Transform>(puzzelObjects[i]).rotation.y,-360,360);
                    ImGui::SliderFloat("Z",&this->getComponent<Transform>(puzzelObjects[i]).rotation.z,-360,360);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Size")) {
                    ImGui::InputFloat("X",&this->getComponent<Transform>(puzzelObjects[i]).scale.x);
                    ImGui::InputFloat("Y",&this->getComponent<Transform>(puzzelObjects[i]).scale.y);
                    ImGui::InputFloat("Z",&this->getComponent<Transform>(puzzelObjects[i]).scale.z);
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
    //if (Input::isKeyPressed(Keys::B)) {
    //
    //    this->puzzelObjects.push_back(this->createEntity());
    //    std::string penist = "Cahhaj";
    //
    //    this->setComponent<MeshComponent>(this->puzzelObjects[this->puzzelObjects.size() - 1]);
    //    MeshComponent& m =
    //        this->getComponent<MeshComponent>(this->puzzelObjects[this->puzzelObjects.size() - 1]);
    //    memcpy(m.filePath, penist.c_str(), sizeof(m.filePath));
    //    std::cout << m.filePath << std::endl;
    //    
    //    Transform& transform = this->getComponent<Transform>(this->puzzelObjects[this->puzzelObjects.size() - 1]);
    //    transform.position   = this->getComponent<Transform>(this->mover).position;
    //    transform.scale      = glm::vec3(5.f, 1.f, 1.f);
    //}

}
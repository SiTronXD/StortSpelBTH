#include "../Systems/AiCombatSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "NetworkGameScene.h"
#include <iostream>
//SEVER SIDE!!!

NetworkGameScene::NetworkGameScene() 
{
  //create seed
#ifdef _CONSOLE
  srand(69);
#else
  srand(time(NULL));
#endif
  this->roomSeed = rand();
}

NetworkGameScene::~NetworkGameScene()
{
  //if (aiHandler != nullptr)
  //  {
  //    delete aiHandler;
  //  }
}

void NetworkGameScene::start()
{
  //TODO : just change to seed
  //send seed to players
  std::cout << "SERVER: seed is: " << roomSeed << std::endl;
  this->addEvent({(int)GameEvents::GetLevelSeed, this->roomSeed});
  srand(roomSeed);
  this->roomHandler.serverInit(
      this,
      this->getConfigValue<int>("room_size"),
      this->getConfigValue<int>("tile_types")
  );
  this->roomHandler.generate();

  //// Ai management
  //this->aiHandler = new AIHandler();
  //this->aiHandler->init(this->getSceneHandler());
  //aiExample();
  //int floor = this->createEntity();
  //this->setComponent<Collider>(
  //    floor, Collider::createBox(glm::vec3(100, 0.2, 100))
  //);
  //Transform& transform2 = this->getComponent<Transform>(floor);
  //transform2.position = glm::vec3(0.0f, -10.0f, 0.0f);
}

void NetworkGameScene::init() {}

void NetworkGameScene::update(float dt)
{


}

void NetworkGameScene::aiExample()
{
  auto a = [&](FSM* fsm, uint32_t entityId) -> void
  {
    SwarmFSM* swarmFSM = (SwarmFSM*)fsm;

    auto entityImguiWindow = [&](SwarmFSM* swarmFsm, uint32_t entityId) -> void
    {
      auto& entitySwarmComponent =
          this->getSceneHandler()->getScene()->getComponent<SwarmComponent>(
              entityId
          );
      auto& entityAiCombatComponent =
          this->getSceneHandler()->getScene()->getComponent<AiCombat>(entityId);
      auto& entiyFSMAgentComp =
          this->getSceneHandler()->getScene()->getComponent<FSMAgentComponent>(
              entityId
          );
      auto& entityRigidBody =
          this->getSceneHandler()->getScene()->getComponent<Rigidbody>(entityId
          );
      int& health = entitySwarmComponent.life;
      float& jumpForce = entitySwarmComponent.jumpForce;
      float& jumpForceY = entitySwarmComponent.jumpY;
      float& speed = entitySwarmComponent.speed;
      float& attackRange = entitySwarmComponent.attackRange;
      float& sightRange = entitySwarmComponent.sightRadius;
      bool& inCombat = entitySwarmComponent.inCombat;
      float& attackPerSec = entityAiCombatComponent.lightAttackTime;
      float& lightAttackDmg = entityAiCombatComponent.lightHit;
      float& gravity = entityRigidBody.gravityMult;
      std::string& status = entiyFSMAgentComp.currentNode->status;
      ImGui::Text(status.c_str());
      ImGui::SliderInt("health", &health, 0, 100);
      ImGui::SliderFloat("speed", &speed, 0, 100);
      ImGui::SliderFloat("jumpForce", &jumpForce, 0, 100);
      ImGui::SliderFloat("jumpForceY", &jumpForceY, 0, 100);
      ImGui::SliderFloat("gravity", &gravity, 0, 10);
      ImGui::SliderFloat("attackRange", &attackRange, 0, 100);
      ImGui::SliderFloat("sightRange", &sightRange, 0, 100);
      ImGui::InputFloat("attack/s", &attackPerSec);
      ImGui::InputFloat("lightattackDmg", &lightAttackDmg);
      ImGui::Checkbox("inCombat", &inCombat);
    };
    //TEMP

    static bool showEntityId = false;
    ImGui::Checkbox("Show Entity ID", &showEntityId);
    if (showEntityId)
      {

        // Show all entity ID over entitties
        glm::vec4 entityPos = glm::vec4(
            this->getSceneHandler()
                ->getScene()
                ->getComponent<Transform>(entityId)
                .position,
            1.f
        );

        auto screenPos = this->getMainCamera()->projection *
                         this->getMainCamera()->view * entityPos;
        glm::vec3 realScreenPos;
        realScreenPos.x = (screenPos.x / screenPos.w) * 1920 / 2;
        realScreenPos.y = (screenPos.y / screenPos.w) * 1080 / 2;
        realScreenPos.z = screenPos.z / screenPos.w;

        Scene::getUIRenderer()->setTexture(this->fontTextureIndex);
        Scene::getUIRenderer()->renderString(
            std::to_string(entityId),
            glm::vec2(realScreenPos.x, realScreenPos.y),
            glm::vec2(20, 20)
        );
      }

    std::string playerString = "playerID";
    int playerID;
    this->getSceneHandler()->getScriptHandler()->getGlobal(
        playerID, playerString
    );
    auto& playerCombat =
        this->getSceneHandler()->getScene()->getComponent<Combat>(playerID);
    if (ImGui::Button("Kill Player"))
      {
        playerCombat.health = 0;
      }
    if (ImGui::Button("INVINCIBLE Player"))
      {
        playerCombat.health = INT_MAX;
      }
    ImGui::Separator();
    ImGui::Separator();
    entityImguiWindow(swarmFSM, entityId);

    auto& entitySwarmComponent =
        this->getSceneHandler()->getScene()->getComponent<SwarmComponent>(
            entityId
        );
    auto& entityAiCombatComponent =
        this->getSceneHandler()->getScene()->getComponent<AiCombat>(entityId);
    auto& entiyFSMAgentComp =
        this->getSceneHandler()->getScene()->getComponent<FSMAgentComponent>(
            entityId
        );

    std::string groupName = "GroupMembers[" +
                            std::to_string(entitySwarmComponent.group->myId) +
                            "]";
    if (ImGui::TreeNode(groupName.c_str()))
      {
        if (ImGui::Button("Kill All"))
          {
            entitySwarmComponent.life = 0;
            for (auto& ent : entitySwarmComponent.group->members)
              {
                auto& entSwarmComp = this->getSceneHandler()
                                         ->getScene()
                                         ->getComponent<SwarmComponent>(ent);
                entSwarmComp.life = 0;
              }
          }

        static int selected_friend = -1;

        for (auto& ent : entitySwarmComponent.group->members)
          {
            std::string entityName = "entity[" + std::to_string(ent) + "]";
            if (ImGui::Button(entityName.c_str()))
              {
                selected_friend = ent;
              }
          }
        if (selected_friend != -1)
          {
            std::string entityName =
                "entity[" + std::to_string(selected_friend) + "]";
            ImGui::Begin((entityName + "_popup").c_str());
            entityImguiWindow(swarmFSM, selected_friend);
            ImGui::End();
          }
        ImGui::TreePop();
      }
  };
  // static SwarmFSM swarmFSM;

  // this->aiHandler->addFSM(&swarmFSM, "swarmFSM");
  this->aiHandler->addFSM(&this->swarmFSM, "swarmFSM");

//TODO: Cause crash on second run, therefore disabled in distribution...
#ifdef _CONSOLE
  //this->aiHandler->addImguiToFSM("swarmFSM", a);
#endif

  //int numOfGroups = 4;
  //int group_size = 3;
  int numOfGroups = 1;
  int group_size = 1;
  for (size_t j = 0; j < numOfGroups; j++)
    {
      this->swarmGroups.push_back(new SwarmGroup);
      for (size_t i = 0; i < group_size; i++)
        {
          this->enemyIDs.push_back(this->createEnemy(1));
          this->setComponent<AiCombat>(this->enemyIDs.back());
          this->setComponent<Collider>(
              this->enemyIDs.back(), Collider::createSphere(4.0f)
          );
          this->getComponent<Transform>(this->enemyIDs.back()).position =
              glm::vec3(0, 2, 0);
          this->setComponent<Rigidbody>(this->enemyIDs.back());
          Rigidbody& rb = this->getComponent<Rigidbody>(this->enemyIDs.back());
          rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
          rb.gravityMult = 5.0f;
          rb.friction = 1.5f;
          this->aiHandler->createAIEntity(this->enemyIDs.back(), "swarmFSM");
          this->swarmGroups.back()->members.push_back(this->enemyIDs.back());
          //this->setInactive(this->enemyIDs.back());
          this->getSceneHandler()
              ->getScene()
              ->getComponent<SwarmComponent>(this->enemyIDs.back())
              .group = this->swarmGroups.back();
          SwarmComponent& swarmComp =
              this->getComponent<SwarmComponent>(this->enemyIDs.back());
          swarmComp.life = 100.0f;
        }
    }
}
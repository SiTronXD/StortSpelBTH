#include "../Systems/AiCombatSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "NetworkGameScene.h"
#include <iostream>
//SEVER SIDE!!!

NetworkGameScene::NetworkGameScene() : newRoomFrame(false)
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
  if (aiHandler != nullptr)
  {
    delete aiHandler;
  }
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
  this->aiHandler = new AIHandler();
  this->aiHandler->init(this->getSceneHandler());
  aiExample();
  int floor = this->createEntity();
  this->setComponent<Collider>(
      floor, Collider::createBox(glm::vec3(100, 0.2, 100))
  );
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

  this->aiHandler->addFSM(&this->swarmFSM, "swarmFSM");

//TODO: Cause crash on second run, therefore disabled in distribution...
#ifdef _CONSOLE
  this->aiHandler->addImguiToFSM("swarmFSM", a);
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
          
          //this->enemyIDs.push_back(this->createEntity());
          this->enemyIDs.push_back(this->createEnemy(0));
          this->setComponent<AiCombat>(this->enemyIDs.back());
          this->setComponent<Collider>(
              this->enemyIDs.back(), Collider::createSphere(4.0f)
          );
          this->setComponent<Rigidbody>(this->enemyIDs.back());
          Rigidbody& rb = this->getComponent<Rigidbody>(this->enemyIDs.back());
          rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
          rb.gravityMult = 5.0f;
          rb.friction = 1.5f;
          this->aiHandler->createAIEntity(this->enemyIDs.back(), "swarmFSM");
          this->swarmGroups.back()->members.push_back(this->enemyIDs.back());
          this->setInactive(this->enemyIDs.back());
          this->getSceneHandler()
              ->getScene()
              ->getComponent<SwarmComponent>(this->enemyIDs.back())
              .group = this->swarmGroups.back();
          SwarmComponent& swarmComp =
              this->getComponent<SwarmComponent>(this->enemyIDs.back());
          swarmComp.life = 0;
        }
    }
}

bool NetworkGameScene::allDead()
{
  bool ret = true;
  for (auto p : enemyIDs)
    {
      if (this->isActive(p))
        {
          ret = false;
          break;
        }
    }
  return ret;
}

void NetworkGameScene::onTriggerStay(Entity e1, Entity e2)
{
    //TODO : player 0 is temp
  Entity player = e1 == this->getPlayer(0)   ? e1
                  : e2 == this->getPlayer(0) ? e2
                                             : -1;

  if (player == this->getPlayer(0))  // player triggered a trigger :]
    {
      Entity other = e1 == player ? e2 : e1;
      if (roomHandler.onPlayerTrigger(other))
        {
          this->newRoomFrame = true;

          int idx = 0;
          int randNumEnemies = rand() % 8 + 3;
          int counter = 0;
          const std::vector<Entity>& entites = roomHandler.getFreeTiles();
          for (Entity entity : entites)
            {
              if (idx != 10 && randNumEnemies - counter != 0)
                {
                  this->setActive(this->enemyIDs[idx]);
                  Transform& transform =
                      this->getComponent<Transform>(this->enemyIDs[idx]);
                  Transform& tileTrans = this->getComponent<Transform>(entity);
                  float tileWidth =
                      rand() % ((int)RoomHandler::TILE_WIDTH / 2) + 0.01f;
                  transform.position = tileTrans.position;
                  transform.position =
                      transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

                  //Temporary enemie reset
                  SwarmComponent& swarmComp =
                      this->getComponent<SwarmComponent>(this->enemyIDs[idx]);
                  transform.scale.y = 1.0f;
                  swarmComp.life = swarmComp.FULL_HEALTH;
                  swarmComp.group->inCombat = false;

                  swarmComp.group->aliveMembers.push(0
                  );  // TODO: This should be done somewhere else... Like in SwarmFSM/BT

                  idx++;
                  counter++;
                }
            }
          for (SwarmGroup* group : this->swarmGroups)
            {
              //Set idle mid pos
              group->idleMidBos = glm::vec3(0.0f, 0.0f, 0.0f);
              int numAlive = 0;
              for (auto b : group->members)
                {
                  if (isActive(b) &&
                      this->getComponent<SwarmComponent>(b).life > 0)
                    {
                      group->idleMidBos +=
                          this->getComponent<Transform>(b).position;
                      numAlive++;
                    }
                }
              group->idleMidBos /= numAlive;
              //Set ilde radius
              for (auto b : group->members)
                {
                  if (isActive(b) &&
                      this->getComponent<SwarmComponent>(b).life > 0)
                    {
                      float len = glm::length(
                          group->idleMidBos -
                          this->getComponent<Transform>(b).position
                      );
                      if (len > group->idleRadius)
                        {
                          group->idleRadius = len;
                        }
                    }
                }
              //Set move to
              for (auto b : group->members)
                {
                  SwarmComponent& swarmComp =
                      this->getComponent<SwarmComponent>(b);
                  swarmComp.idleMoveTo = group->idleMidBos;
                  glm::vec3 dir = glm::normalize(glm::vec3(
                      rand() * (rand() % 2 == 0 ? -1 : 1),
                      0.0f,
                      rand() * (rand() % 2 == 0 ? -1 : 1)
                  ));
                  swarmComp.idleMoveTo = swarmComp.group->idleMidBos +
                                         dir * swarmComp.group->idleRadius;
                }
            }
        }

      //TODO : ?
      //if (other == portal &&
      //    numRoomsCleared >= this->roomHandler.getNumRooms() -
      //                           1)  // -1 not counting start room
      //  {
      //    this->switchScene(new GameSceneNetwork(), "scripts/gamescene.lua");
      //  }
    }
}

void NetworkGameScene::onTriggerEnter(Entity e1, Entity e2)
{
  Entity ground = e1 == this->roomHandler.getFloor()   ? e1
                  : e2 == this->roomHandler.getFloor() ? e2
                                                       : -1;
  //Entity perk = this->hasComponents<Perks>(e1)   ? e1
  //              : this->hasComponents<Perks>(e2) ? e2
  //                                               : -1;
  //Entity ability = this->hasComponents<Abilities>(e1)   ? e1
  //                 : this->hasComponents<Abilities>(e2) ? e2
  //                                                      : -1;
  //
  //if (this->entityValid(ground))
  //  {
  //    if (this->entityValid(perk))
  //      {
  //        this->removeComponent<Rigidbody>(perk);
  //        Transform& perkTrans = this->getComponent<Transform>(perk);
  //        perkTrans.position.y = 2.f;
  //      }
  //    else if (this->entityValid(ability))
  //      {
  //        this->removeComponent<Rigidbody>(ability);
  //        Transform& abilityTrans = this->getComponent<Transform>(ability);
  //        abilityTrans.position.y = 4.f;
  //      }
  //  }
}

void NetworkGameScene::onCollisionEnter(Entity e1, Entity e2)
{
  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
    {
      this->getComponent<SwarmComponent>(e1).touchedFriend = true;
      this->getComponent<SwarmComponent>(e2).touchedFriend = true;
    }
}

void NetworkGameScene::onCollisionStay(Entity e1, Entity e2)
{
  Entity player = e1 == this->getPlayer(0)   ? e1
                  : e2 == this->getPlayer(0) ? e2
                                             : -1;

  if (player == this->getPlayer(0))  // player triggered a trigger :]
    {
      Entity other = e1 == player ? e2 : e1;
      if (this->hasComponents<SwarmComponent>(other))
        {
          auto& swarmComp = this->getComponent<SwarmComponent>(other);
          if (swarmComp.inAttack)
            {
              auto& aiCombat = this->getComponent<AiCombat>(other);
              swarmComp.inAttack = false;
              swarmComp.touchedPlayer = true;
              //aiCombat.timer = aiCombat.lightAttackTime;
              this->getComponent<Combat>(player).health -=
                  (int)aiCombat.lightHit;
              std::cout << "WAS HIT\n";
            }
        }
    }

  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
    {
      this->getComponent<SwarmComponent>(e1).touchedFriend = true;
      this->getComponent<SwarmComponent>(e2).touchedFriend = true;
    }
}

void NetworkGameScene::onCollisionExit(Entity e1, Entity e2)
{

  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
    {
      this->getComponent<SwarmComponent>(e1).touchedFriend = false;
      this->getComponent<SwarmComponent>(e2).touchedFriend = false;
    }
}

void NetworkGameScene::createPortal()
{
  //glm::vec3 portalTriggerDims(6.f, 18.f, 1.f);
  //glm::vec3 portalBlockDims(3.f, 18.f, 3.f);
  //
  //portalOffMesh =
  //    this->getResourceManager()->addMesh("assets/models/PortalOff.obj");
  //portalOnMesh =
  //    this->getResourceManager()->addMesh("assets/models/PortalOn.obj");
  //
  //portal = this->createEntity();
  //this->getComponent<Transform>(portal).position =
  //    this->roomHandler.getExitRoom().position;
  //this->setComponent<Collider>(
  //    portal, Collider::createBox(portalTriggerDims, glm::vec3(0, 0, 0), true)
  //);
  //
  //this->setComponent<MeshComponent>(portal);
  //this->getComponent<MeshComponent>(portal).meshID = portalOffMesh;
  //
  //Entity collider1 = this->createEntity();
  //this->getComponent<Transform>(collider1).position =
  //    this->getComponent<Transform>(portal).position;
  //this->getComponent<Transform>(collider1).position.x += 9.f;
  //this->getComponent<Transform>(collider1).position.y += 9.f;
  //this->setComponent<Collider>(collider1, Collider::createBox(portalBlockDims));
  //
  //Entity collider2 = this->createEntity();
  //this->getComponent<Transform>(collider2).position =
  //    this->getComponent<Transform>(portal).position;
  //this->getComponent<Transform>(collider2).position.x -= 9.f;
  //this->getComponent<Transform>(collider2).position.y += 9.f;
  //this->setComponent<Collider>(collider2, Collider::createBox(portalBlockDims));
}
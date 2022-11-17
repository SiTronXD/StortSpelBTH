#include "NetworkGameScene.h"
#include "../Systems/AiCombatSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"
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
      aiHandler->clean();
      delete aiHandler;
    }
  for (int i = 0; i < swarmGroups.size(); i++)
  {
      delete swarmGroups[i]; 
  }
}

void NetworkGameScene::start()
{
  //send seed to players
  std::cout << "SERVER: seed is: " << roomSeed << std::endl;
  this->addEvent({(int)GameEvents::GetLevelSeed, this->roomSeed});
  srand(roomSeed);
  this->roomHandler.serverInit(this, 15, 15);
  this->roomHandler.generate();
  
  // Ai management
  this->aiHandler = new AIHandler();
  this->aiHandler->init(this->getSceneHandler());
  aiExample();
}

void NetworkGameScene::init() {}

void NetworkGameScene::update(float dt)
{
  aiHandler->update();
  
  if (allDead() && this->newRoomFrame)
    {
      this->newRoomFrame = false;
      this->addEvent({GameEvents::ROOM_CLEAR});
      // Call when a room is cleared
      roomHandler.roomCompleted();
      this->numRoomsCleared++;
  
      if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
      {
          //send event to spawn portal
      }
    }
}

void NetworkGameScene::aiExample()
{

  this->aiHandler->addFSM(&this->swarmFSM, "swarmFSM");

  int numOfGroups = 3;
  int group_size = 4;
  for (size_t j = 0; j < numOfGroups; j++)
    {
      this->swarmGroups.push_back(new SwarmGroup);
      for (size_t i = 0; i < group_size; i++)
        {

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

  //TODO : this is going to be sent by player to make this
  Entity player = isAPlayer(e1) ? e1 : isAPlayer(e2) ? e2 : -1;

  if (isAPlayer(player))  // player triggered a trigger :]
    {
      Entity other = e1 == player ? e2 : e1;
      if (roomHandler.onPlayerTrigger(other))
        {
          this->newRoomFrame = true;

          std::cout << "walked in new room" << std::endl;

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

                  std::cout << "spawn enemy at: " << transform.position.x
                            << ", " << transform.position.y << ", "
                            << transform.position.z << std::endl;

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
  Entity player = this->isAPlayer(e1) ? e1 : this->isAPlayer(e2) ? e2 : -1;

  if (this->isAPlayer(player))  // player triggered a trigger :]
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
              aiCombat.timer = aiCombat.lightAttackTime;
              this->addEvent(
                  {(int)GameEvents::MONSTER_HIT,
                   other,
                   (int)aiCombat.lightHit,
                   player}
              );
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
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
  spawnHandler.init(
      &this->roomHandler,
      this,
      this->getSceneHandler(),
      this->aiHandler,
      this->getResourceManager(),
      this->getUIRenderer()
  );
}

void NetworkGameScene::init() {}

void NetworkGameScene::update(float dt)
{
    //aiHandler->update(dt);
  
    if (this->spawnHandler.allDead() && this->newRoomFrame)
    {
          this->newRoomFrame = false;
          std::cout << "all dead" << std::endl;
          this->addEvent({GameEvents::ROOM_CLEAR});
          // Call when a room is cleared
          roomHandler.roomCompleted();
          this->numRoomsCleared++;
    
          if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
          {
              //send event to spawn portal
          }
    }

    for (int i = 0; i < roomHandler.rooms.size(); i++)
    {
        for (int d = 0; d < 4; d++)
        {
            if (roomHandler.rooms[i].doors[d] != -1)
            {
                glm::vec3 dp = this->getComponent<Transform>(roomHandler.rooms[i].doors[d]).position;
                 addEvent({(int)GameEvents::Draw_Debug_BoxCollider}, {dp.x, dp.y, dp.z, 10.f, 10.f, 10.f});
            }    
        }
    }

    sf::Packet &packet = ((NetworkSceneHandler*)this->getSceneHandler())->getCallFromClient();
    while (!packet.endOfPacket())
    {
        int gameEvent;
        packet >> gameEvent;
        if (gameEvent == GameEvents::HitMonster)
          {
            int monsterID, playerEnt, damage;
            float knockBack;
            packet >> monsterID >> playerEnt >> damage >> knockBack;

            SwarmComponent& enemy = this->getComponent<SwarmComponent>(monsterID);
            enemy.life -= damage;
            std::cout << enemy.life << std::endl;
            Rigidbody& enemyRB = this->getComponent<Rigidbody>(monsterID);
            Transform& enemyTrans = this->getComponent<Transform>(monsterID);
            Transform& playerTrans = this->getComponent<Transform>(playerEnt);
	    	glm::vec3 newDir = glm::normalize(playerTrans.position - enemyTrans.position);
            enemyRB.velocity = glm::vec3(-newDir.x, 0.f, -newDir.z) * knockBack;
          }
    }
}

void NetworkGameScene::onTriggerStay(Entity e1, Entity e2)
{
    Entity player = isAPlayer(e1) ? e1 : isAPlayer(e2) ? e2 : -1;

	if (player != -1) // player triggered a trigger :]
	{
		Entity other = e1 == player ? e2 : e1;
		if (roomHandler.onPlayerTrigger(other))
		{
			this->newRoomFrame = true;

            this->spawnHandler.spawnEnemiesIntoRoom();
		}
	}        

            //if (other == portal &&
            //    numRoomsCleared >= this->roomHandler.getNumRooms() -
            //                           1)  // -1 not counting start room
            //    {
            //        this->switchScene(new GameScene(), "scripts/gamescene.lua");
            //    }
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
              auto& aiCombat = this->getComponent<AiCombatSwarm>(other);
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
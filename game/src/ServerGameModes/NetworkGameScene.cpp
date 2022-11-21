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
    aiHandler->update(dt);
  
    if (allDead() && this->newRoomFrame)
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
          this->setComponent<AiCombatSwarm>(this->enemyIDs.back());
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
  static TankFSM tankFSM;
  this->aiHandler->addFSM(&tankFSM, "tankFSM");
  for (int i = 0; i < 1; i++)
      {
          this->tankIDs.push_back(this->createEntity());
          this->setComponent<AiCombatTank>(this->tankIDs.back());
          this->setComponent<Rigidbody>(this->tankIDs.back());
          Rigidbody& rb = this->getComponent<Rigidbody>(this->tankIDs.back());
          rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
          rb.gravityMult = 5.0f;
          rb.friction = 3.0f;
          rb.mass = 10.0f;
          Transform& transform =
              this->getComponent<Transform>(this->tankIDs.back());
          transform.scale = glm::vec3(3.0f, 3.0f, 3.0f);
          this->setComponent<Collider>(
              this->tankIDs.back(),
              Collider::createSphere(4.0f * transform.scale.x)
          );
          this->aiHandler->createAIEntity(this->tankIDs.back(), "tankFSM");
          TankComponent& tankComp =
              this->getComponent<TankComponent>(this->tankIDs.back());
          tankComp.origScaleY = transform.scale.y;
          this->setInactive(this->tankIDs.back());
      }
  //stnky LICH
  static LichFSM lichFSM;
  this->aiHandler->addFSM(&lichFSM, "lichFSM");
  for (int i = 0; i < 1; i++)
      {
          this->lichIDs.push_back(this->createEntity());
          this->setComponent<AiCombatLich>(this->lichIDs.back());
          this->setComponent<Rigidbody>(this->lichIDs.back());
          Rigidbody& rb = this->getComponent<Rigidbody>(this->lichIDs.back());
          rb.rotFactor = glm::vec3(0.0f, 0.0f, 0.0f);
          rb.gravityMult = 5.0f;
          rb.friction = 3.0f;
          rb.mass = 10.0f;
          Transform& transform =
              this->getComponent<Transform>(this->lichIDs.back());
          transform.scale = glm::vec3(1.0f, 3.0f, 1.0f);
          this->setComponent<Collider>(
              this->lichIDs.back(),
              Collider::createCapsule(4.0f, 4.0f * transform.scale.y)
          );
          this->aiHandler->createAIEntity(this->lichIDs.back(), "lichFSM");
          this->setInactive(this->lichIDs.back());
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
    Entity player = isAPlayer(e1) ? e1 : isAPlayer(e2) ? e2 : -1;

	if (player != -1) // player triggered a trigger :]
	{
		Entity other = e1 == player ? e2 : e1;
		if (roomHandler.onPlayerTrigger(other))
		{
			this->newRoomFrame = true;

            //Num to spawn
            int numTanks        = 1;
            int numLich         = 0;
            int numSwarm        = 3;

			int swarmIdx        = 0;
			int lichIdx         = 0;
			int tankIdx         = 0;
			int randNumEnemies  = 10;
			int counter         = 0;
			const std::vector<Entity>& tiles = roomHandler.getFreeTiles();
			for (Entity tile : tiles)
			{
				if (randNumEnemies - counter != 0)
				{
					
					if(tankIdx < numTanks)
					{
						this->setActive(this->tankIDs[tankIdx]);
						Transform& transform = this->getComponent<Transform>(this->tankIDs[tankIdx]);
						Transform& tileTrans = this->getComponent<Transform>(tile);
						float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
						transform.position = tileTrans.position;
						transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

                        //Reset
                        TankComponent& tankComp = this->getComponent<TankComponent>(this->tankIDs[tankIdx]);
                        tankComp.life = tankComp.FULL_HEALTH;
                        transform.scale.y = tankComp.origScaleY;

						tankIdx++;
					}
					else if(lichIdx < numLich)
					{
						this->setActive(this->lichIDs[lichIdx]);
						Transform& transform = this->getComponent<Transform>(this->lichIDs[lichIdx]);
						Transform& tileTrans = this->getComponent<Transform>(tile);
						float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
						transform.position = tileTrans.position;
						transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

                        //Reset
                        LichComponent& lichComp = this->getComponent<LichComponent>(this->lichIDs[tankIdx]);
                        lichComp.life = lichComp.FULL_HEALTH;

						lichIdx++;
					}
					else if(swarmIdx < numSwarm)
					{
						this->setActive(this->swarmIDs[swarmIdx]);
						Transform& transform = this->getComponent<Transform>(this->swarmIDs[swarmIdx]);
						Transform& tileTrans = this->getComponent<Transform>(tile);
						float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
						transform.position = tileTrans.position;
						transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);

						//Temporary enemie reset
						SwarmComponent& swarmComp = this->getComponent<SwarmComponent>(this->swarmIDs[swarmIdx]);
						transform.scale.y = 1.0f;
						swarmComp.life = swarmComp.FULL_HEALTH;
						swarmComp.group->inCombat = false;
					
						swarmComp.group->aliveMembers.push(0); 

						swarmIdx++;
					}
					
					counter++;
				
				}
			}

			for(SwarmGroup* group: this->swarmGroups)
			{
					//Set idle mid pos
					group->idleMidPos = glm::vec3(0.0f, 0.0f, 0.0f);
					int numAlive = 0;
					for(auto b: group->members)
					{
						if(isActive(b) && this->getComponent<SwarmComponent>(b).life > 0)
						{
							group->idleMidPos += this->getComponent<Transform>(b).position;
							numAlive++;
						}
					}
					group->idleMidPos /= numAlive;
					//Set ilde radius
					for(auto b: group->members)
					{
						if(isActive(b) && this->getComponent<SwarmComponent>(b).life > 0)
						{
							float len = glm::length(group->idleMidPos - this->getComponent<Transform>(b).position);
							if(len > group->idleRadius)
							{
								group->idleRadius = len;
							}
						}
					}
					//Set move to
					for(auto b: group->members)
					{
						SwarmComponent& swarmComp = this->getComponent<SwarmComponent>(b);
						swarmComp.idleMoveTo = group->idleMidPos;
						glm::vec3 dir = glm::normalize(glm::vec3(rand() * (rand() % 2 == 0 ? - 1 : 1), 0.0f, rand() * (rand() % 2 == 0 ? - 1 : 1)));
						swarmComp.idleMoveTo = swarmComp.group->idleMidPos + dir * swarmComp.group->idleRadius;
					}
					for(auto t: tankIDs)
			        {
			        	TankComponent& tankComp = this->getComponent<TankComponent>(t);
			        	tankComp.setFriends(this, t);
			        }
			}
		}        

            //if (other == portal &&
            //    numRoomsCleared >= this->roomHandler.getNumRooms() -
            //                           1)  // -1 not counting start room
            //    {
            //        this->switchScene(new GameScene(), "scripts/gamescene.lua");
            //    }
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
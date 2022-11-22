#include "GameSceneNetwork.h"

#include "../Systems/AiCombatSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/HealthBarSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "GameOverScene.h"

#ifdef _CONSOLE
// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);
#endif

GameSceneNetwork::GameSceneNetwork() :
    playerID(-1), portal(-1), numRoomsCleared(0), newRoomFrame(false), perk(-1),
    perk1(-1), perk2(-1), ability(-1)
{
}

GameSceneNetwork::~GameSceneNetwork()
{
  for (auto& p : swarmGroups)
    {
      delete p;
    }
}

void GameSceneNetwork::init()
{
  int swarm =
      this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");

  roomHandler.init(
      this,
      this->getResourceManager(),
      this->getConfigValue<int>("room_size"),
      this->getConfigValue<int>("tile_types")
  );
  roomHandler.generate();
  createPortal();

  ResourceManager* resourceMng = this->getResourceManager();
  abilityTextures[0] =
      resourceMng->addTexture("assets/textures/UI/knockbackAbility.png");
  abilityTextures[1] =
      resourceMng->addTexture("assets/textures/UI/knockbackAbility.png");
  abilityTextures[2] = resourceMng->addTexture("assets/textures/UI/empty.png");
  perkTextures[0] = resourceMng->addTexture("assets/textures/UI/hpUp.png");
  perkTextures[1] = resourceMng->addTexture("assets/textures/UI/dmgUp.png");
  perkTextures[2] =
      resourceMng->addTexture("assets/textures/UI/atkSpeedUp.png");
  perkTextures[3] = resourceMng->addTexture("assets/textures/UI/empty.png");
  this->hpBarBackgroundTextureID =
      resourceMng->addTexture("assets/textures/UI/hpBarBackground.png");
  this->hpBarTextureID =
      resourceMng->addTexture("assets/textures/UI/hpBar.png");

  // Temporary light
  Entity directionalLightEntity = this->createEntity();
  this->setComponent<DirectionalLight>(
      directionalLightEntity, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f)
  );

  Entity sun = this->createEntity();
  this->setComponent<DirectionalLight>(sun);
  DirectionalLight& light = this->getComponent<DirectionalLight>(sun);
  light.color = glm::vec3(1.f, 0.8f, 0.5f);
  light.direction = glm::normalize(glm::vec3(-1.f));

  this->createSystem<HealthBarSystem>(
      this->hpBarBackgroundTextureID,
      this->hpBarTextureID,
      this,
      this->getUIRenderer()
  );
}

void GameSceneNetwork::start()
{
  int swarm =
      this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
  int playerModel = this->getResourceManager()->addAnimations(
      std::vector<std::string>({"assets/models/Character/CharRun.fbx"}),
      "assets/textures/playerMesh"
  );

  this->getNetworkHandler()->setMeshes("blob", swarm);
  this->getNetworkHandler()->setMeshes("range", swarm);
  this->getNetworkHandler()->setMeshes("tank", swarm);
  this->getNetworkHandler()->setMeshes("PlayerMesh", playerModel);

  this->getNetworkHandler()->createPlayers();
  std::string playerName = "playerID";
  this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);

  bool paused = false;
  this->setComponent<Combat>(playerID);
  this->createSystem<CombatSystem>(
      this,
      this->getResourceManager(),
      this->playerID,
      paused,
      this->getPhysicsEngine(),
      this->getUIRenderer(),
      this->getScriptHandler(),
      nullptr
  );

  this->ability = this->createEntity();
  int knockback =
      this->getResourceManager()->addMesh("assets/models/KnockbackAbility.obj");
  this->setComponent<MeshComponent>(this->ability, knockback);
  Transform& abilityTrans = this->getComponent<Transform>(this->ability);
  abilityTrans.position = glm::vec3(50.f, 10.f, 0.f);
  abilityTrans.scale = glm::vec3(4.f, 4.f, 4.f);
  this->setComponent<Collider>(
      this->ability, Collider::createSphere(4.f, glm::vec3(0), true)
  );
  this->setComponent<Abilities>(this->ability, healAbility);

  this->perk = this->createEntity();
  int perkHp = this->getResourceManager()->addMesh("assets/models/Perk_Hp.obj");
  this->setComponent<MeshComponent>(this->perk, perkHp);
  Transform& perkTrans = this->getComponent<Transform>(this->perk);
  perkTrans.position = glm::vec3(30.f, 5.f, 20.f);
  perkTrans.scale = glm::vec3(2.f, 2.f, 2.f);
  this->setComponent<Collider>(
      this->perk, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true)
  );
  this->setComponent<Perks>(this->perk);
  Perks& perkSetting = this->getComponent<Perks>(this->perk);
  perkSetting.multiplier = 1.f;
  perkSetting.perkType = hpUpPerk;
  this->setScriptComponent(this->perk, "scripts/spin.lua");

  this->perk1 = this->createEntity();
  int perkDmg =
      this->getResourceManager()->addMesh("assets/models/Perk_Hp.obj");
  this->setComponent<MeshComponent>(this->perk1, perkDmg);
  Transform& perkTrans1 = this->getComponent<Transform>(this->perk1);
  perkTrans1.position = glm::vec3(30.f, 5.f, -20.f);
  perkTrans1.scale = glm::vec3(2.f, 2.f, 2.f);
  this->setComponent<Collider>(
      this->perk1, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true)
  );
  this->setComponent<Perks>(this->perk1);
  Perks& perkSetting1 = this->getComponent<Perks>(this->perk1);
  perkSetting1.multiplier = 0.2f;
  perkSetting1.perkType = hpUpPerk;
  this->setScriptComponent(this->perk1, "scripts/spin.lua");

  this->perk2 = this->createEntity();
  int perkAtkSpeed =
      this->getResourceManager()->addMesh("assets/models/Perk_AtkSpeed.obj");
  this->setComponent<MeshComponent>(this->perk2, perkAtkSpeed);
  Transform& perkTrans2 = this->getComponent<Transform>(this->perk2);
  perkTrans2.position = glm::vec3(30.f, 5.f, 0.f);
  perkTrans2.scale = glm::vec3(2.f, 2.f, 2.f);
  this->setComponent<Collider>(
      this->perk2, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true)
  );
  this->setComponent<Perks>(this->perk2);
  Perks& perkSetting2 = this->getComponent<Perks>(this->perk2);
  perkSetting2.multiplier = 1.f;
  perkSetting2.perkType = attackSpeedUpPerk;
  this->setScriptComponent(this->perk2, "scripts/spin.lua");

  // Ai management
  this->aiHandler = this->getAIHandler();
  this->aiHandler->init(this->getSceneHandler());

  aiExample();
}

void GameSceneNetwork::update()
{
  if (allDead() && this->newRoomFrame)
    {
      this->newRoomFrame = false;

      // Call when a room is cleared
      roomHandler.roomCompleted();
      this->numRoomsCleared++;

      if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
        {
          this->getComponent<MeshComponent>(portal).meshID = portalOnMesh;
        }
    }

  /*if (this->hasComponents<Collider, Rigidbody>(this->playerID))
	{
		Rigidbody& rb = this->getComponent<Rigidbody>(this->playerID);
		glm::vec3 vec = glm::vec3(Input::isKeyDown(Keys::LEFT) - Input::isKeyDown(Keys::RIGHT), 0.0f, Input::isKeyDown(Keys::UP) - Input::isKeyDown(Keys::DOWN));
		float y = rb.velocity.y;
		rb.velocity = vec * 10.0f;
		rb.velocity.y = y + Input::isKeyPressed(Keys::SPACE) * 5.0f;
	}*/

  // Switch scene if the player is dead
  if (this->hasComponents<Combat>(this->playerID))
    {
      if (this->getComponent<Combat>(this->playerID).health <= 0.0f)
        {
          this->switchScene(new GameOverScene(), "scripts/GameOverScene.lua");
        }
    }

  Combat& playerCombat = this->getComponent<Combat>(this->playerID);
  /*switch (playerCombat.ability.abilityType)
	{
	case knockbackAbility:
		this->getUIRenderer()->setTexture(abilityTextures[knockbackAbility]);
		break;
	case healAbility:
		this->getUIRenderer()->setTexture(abilityTextures[healAbility]);
		break;
	case emptyAbility:
		this->getUIRenderer()->setTexture(abilityTextures[emptyAbility]);
		break;
	}*/
  this->getUIRenderer()->setTexture(
      abilityTextures[playerCombat.ability.abilityType]
  );
  this->getUIRenderer()->renderTexture(
      glm::vec2(890.f, -390.f), glm::vec2(100.0f)
  );

  float perkXPos = -720.f;
  float perkYPos = -500.f;
  for (size_t i = 0; i < 4; i++)
    {
      /*switch (playerCombat.perks[i].perkType)
		{
		case hpUpPerk:
			this->getUIRenderer()->setTexture(perkTextures[hpUpPerk]);
			this->getUIRenderer()->renderTexture(glm::vec2(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f), glm::vec2(70.0f));
			break;
		case dmgUpPerk:
			this->getUIRenderer()->setTexture(perkTextures[dmgUpPerk]);
			this->getUIRenderer()->renderTexture(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f, 70.f, 70.f);
			break;
		case attackSpeedUpPerk:
			this->getUIRenderer()->setTexture(perkTextures[attackSpeedUpPerk]);
			this->getUIRenderer()->renderTexture(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f, 70.f, 70.f);
			break;
		case emptyPerk:
			this->getUIRenderer()->setTexture(perkTextures[emptyPerk]);
			this->getUIRenderer()->renderTexture(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f, 70.f, 70.f);
			break;
		}*/
      this->getUIRenderer()->setTexture(
          perkTextures[playerCombat.perks[i].perkType]
      );
      this->getUIRenderer()->renderTexture(
          glm::vec2(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f),
          glm::vec2(70.0f)
      );
    }

  // Render HP bar UI
  float hpPercent = 1.0f;
  float maxHpPercent = 1.0f;
  if (this->hasComponents<Combat>(this->playerID))
    {
      hpPercent = playerCombat.health * 0.01f;
      maxHpPercent = playerCombat.maxHealth * 0.01f;
    }
  float xPos = -720.f;
  float yPos = -500.f;
  float xSize = 1024.f * 0.35f;
  float ySize = 64.f * 0.35f;

  this->getUIRenderer()->setTexture(this->hpBarBackgroundTextureID);
  this->getUIRenderer()->renderTexture(
      glm::vec2(xPos - (1.0f - maxHpPercent) * xSize * 0.5f, yPos),
      glm::vec2((xSize * maxHpPercent) + 10, ySize + 10)
  );
  this->getUIRenderer()->setTexture(this->hpBarTextureID);
  this->getUIRenderer()->renderTexture(
      glm::vec2(xPos - (1.0f - hpPercent) * xSize * 0.5f, yPos),
      glm::vec2(xSize * hpPercent, ySize)
  );

#ifdef _CONSOLE

  static bool renderDebug = false;
  if (ImGui::Begin("Debug"))
    {
      if (ImGui::Checkbox("Render debug shapes", &renderDebug))
        {
          this->getPhysicsEngine()->renderDebugShapes(renderDebug);
        }
      const glm::vec3& playerPos =
          this->getComponent<Transform>(playerID).position;
      ImGui::Text(
          "Player pos: (%d, %d, %d)",
          (int)playerPos.x,
          (int)playerPos.y,
          (int)playerPos.z
      );
      ImGui::Separator();
    }
  ImGui::End();

  roomHandler.imgui();

  decreaseFps();
#endif
}

void GameSceneNetwork::aiExample()
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

    if (ImGui::Button("SWITCHsCENE"))
      {
        this->switchScene(new GameSceneNetwork(), "scripts/gamescene.lua");
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
  static SwarmFSM swarmFSM;

  this->aiHandler->addFSM(&swarmFSM, "swarmFSM");

//TODO: Cause crash on second run, therefore disabled in distribution...
#ifdef _CONSOLE
  this->aiHandler->addImguiToFSM("swarmFSM", a);
#endif

  int swarm =
      this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");

  int numOfGroups = 4;
  int group_size = 3;
  for (size_t j = 0; j < numOfGroups; j++)
    {
      this->swarmGroups.push_back(new SwarmGroup);
      for (size_t i = 0; i < group_size; i++)
        {
          this->enemyIDs.push_back(this->createEntity());
          this->setComponent<MeshComponent>(this->enemyIDs.back(), swarm);
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

bool GameSceneNetwork::allDead()
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

void GameSceneNetwork::onTriggerStay(Entity e1, Entity e2)
{
  Entity player = e1 == playerID ? e1 : e2 == playerID ? e2 : -1;

  if (player == playerID)  // player triggered a trigger :]
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

      if (other == portal &&
          numRoomsCleared >= this->roomHandler.getNumRooms() -
                                 1)  // -1 not counting start room
        {
          this->switchScene(new GameSceneNetwork(), "scripts/gamescene.lua");
        }
    }
}

void GameSceneNetwork::onTriggerEnter(Entity e1, Entity e2)
{
  Entity ground = e1 == this->roomHandler.getFloor()   ? e1
                  : e2 == this->roomHandler.getFloor() ? e2
                                                       : -1;
  Entity perk = this->hasComponents<Perks>(e1)   ? e1
                : this->hasComponents<Perks>(e2) ? e2
                                                 : -1;
  Entity ability = this->hasComponents<Abilities>(e1)   ? e1
                   : this->hasComponents<Abilities>(e2) ? e2
                                                        : -1;

  if (this->entityValid(ground))
    {
      if (this->entityValid(perk))
        {
          this->removeComponent<Rigidbody>(perk);
          Transform& perkTrans = this->getComponent<Transform>(perk);
          perkTrans.position.y = 2.f;
        }
      else if (this->entityValid(ability))
        {
          this->removeComponent<Rigidbody>(ability);
          Transform& abilityTrans = this->getComponent<Transform>(ability);
          abilityTrans.position.y = 4.f;
        }
    }
}

void GameSceneNetwork::onCollisionEnter(Entity e1, Entity e2)
{
  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
    {
      this->getComponent<SwarmComponent>(e1).touchedFriend = true;
      this->getComponent<SwarmComponent>(e2).touchedFriend = true;
    }
}

void GameSceneNetwork::onCollisionStay(Entity e1, Entity e2)
{
  Entity player = e1 == playerID ? e1 : e2 == playerID ? e2 : -1;

  if (player == playerID)  // player triggered a trigger :]
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

void GameSceneNetwork::onCollisionExit(Entity e1, Entity e2)
{

  if (this->hasComponents<SwarmComponent>(e1) &&
      this->hasComponents<SwarmComponent>(e2))
    {
      this->getComponent<SwarmComponent>(e1).touchedFriend = false;
      this->getComponent<SwarmComponent>(e2).touchedFriend = false;
    }
}

void GameSceneNetwork::createPortal()
{
  glm::vec3 portalTriggerDims(6.f, 18.f, 1.f);
  glm::vec3 portalBlockDims(3.f, 18.f, 3.f);

  portalOffMesh =
      this->getResourceManager()->addMesh("assets/models/PortalOff.obj");
  portalOnMesh =
      this->getResourceManager()->addMesh("assets/models/PortalOn.obj");

  portal = this->createEntity();
  this->getComponent<Transform>(portal).position =
      this->roomHandler.getExitRoom().position;
  this->setComponent<Collider>(
      portal, Collider::createBox(portalTriggerDims, glm::vec3(0, 0, 0), true)
  );

  this->setComponent<MeshComponent>(portal);
  this->getComponent<MeshComponent>(portal).meshID = portalOffMesh;

  Entity collider1 = this->createEntity();
  this->getComponent<Transform>(collider1).position =
      this->getComponent<Transform>(portal).position;
  this->getComponent<Transform>(collider1).position.x += 9.f;
  this->getComponent<Transform>(collider1).position.y += 9.f;
  this->setComponent<Collider>(collider1, Collider::createBox(portalBlockDims));

  Entity collider2 = this->createEntity();
  this->getComponent<Transform>(collider2).position =
      this->getComponent<Transform>(portal).position;
  this->getComponent<Transform>(collider2).position.x -= 9.f;
  this->getComponent<Transform>(collider2).position.y += 9.f;
  this->setComponent<Collider>(collider2, Collider::createBox(portalBlockDims));
}

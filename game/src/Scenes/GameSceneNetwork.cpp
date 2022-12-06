#include "GameSceneNetwork.h"

#include "../Systems/AiCombatSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/HealthBarSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "GameOverScene.h"

//THIS IS ON THE CLIENT SIDE

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
  TextureSamplerSettings samplerSettings{};
  samplerSettings.filterMode = vk::Filter::eNearest;
  samplerSettings.unnormalizedCoordinates = VK_TRUE;

  int fontTextureId = Scene::getResourceManager()->addTexture(
      "assets/textures/UI/font.png", {samplerSettings, true}
  );
  Scene::getUIRenderer()->setBitmapFont(
      {"abcdefghij",
       "klmnopqrst",
       "uvwxyz+-.'",
       "0123456789",
       "!?,<>:()#^",
       "@%        "},
      fontTextureId,
      glm::uvec2(50, 50)
  );

  int swarm =
      this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
  int playerModel = this->getResourceManager()->addAnimations(
      std::vector<std::string>(
          {"assets/models/Character/CharIdle.fbx",
           "assets/models/Character/CharRun.fbx",
           "assets/models/Character/CharDodge.fbx",
           "assets/models/Character/CharOutwardAttack.fbx",
           "assets/models/Character/CharHeavyAttack.fbx",
           "assets/models/Character/CharSpinAttack.fbx",
           "assets/models/Character/CharKnockbackAttack.fbx",
           "assets/models/Character/CharInwardAttack.fbx",
           "assets/models/Character/CharSlashAttack.fbx"}
      ),
      "assets/textures/playerMesh"
  );
  this->getResourceManager()->mapAnimations(
      playerModel,
      std::vector<std::string>(
          {"idle",
           "run",
           "dodge",
           "lightAttack",
           "heavyAttack",
           "spinAttack",
           "knockback",
           "mixAttack",
           "slashAttack"}
      )
  );

  this->getNetworkHandler()->setMeshes("blob", swarm);
  this->getNetworkHandler()->setMeshes("range", swarm);
  this->getNetworkHandler()->setMeshes("tank", swarm);
  this->getNetworkHandler()->setMeshes("PlayerMesh", playerModel);

  //int seed = this->getNetworkHandler()->getServerSeed();
  int seed = 123;
  std::cout << "Client: got seed " << seed << std::endl;

  roomHandler.init(
      this,
      this->getResourceManager(), this->getPhysicsEngine(), true
  );
  roomHandler.generate(123);
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
      directionalLightEntity, glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(0.6f)
  );
  DirectionalLight& dirLight =
      this->getComponent<DirectionalLight>(directionalLightEntity);
  dirLight.shadowMapMinBias = 0.0001f;
  dirLight.shadowMapAngleBias = 0.001f;

  this->createSystem<HealthBarSystem>(
      this->hpBarBackgroundTextureID,
      this->hpBarTextureID,
      this,
      this->getUIRenderer()
  );
}

void GameSceneNetwork::start()
{
  
  std::string playerName = "playerID";
  this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);

  bool paused = false;
  this->setComponent<Combat>(playerID);
  this->createSystem<CombatSystem>(
      this,
      this->getResourceManager(),
      this->playerID,
      &paused,
      this->getPhysicsEngine(),
      this->getUIRenderer(),
      this->getScriptHandler(),
      nullptr
  );

  this->ability = this->createEntity();
  int knockback = this->getResourceManager()->addMesh("assets/models/KnockbackAbility.obj");
  this->setComponent<MeshComponent>(this->ability, knockback);
  Transform& abilityTrans = this->getComponent<Transform>(this->ability);
  abilityTrans.position = glm::vec3(50.f, 10.f, 0.f);
  abilityTrans.scale = glm::vec3(4.f, 4.f, 4.f);
  this->setComponent<Collider>(this->ability, Collider::createSphere(4.f, glm::vec3(0), true));
  this->setComponent<Abilities>(this->ability, healAbility);

  this->perk = this->createEntity();
  int perkHp = this->getResourceManager()->addMesh("assets/models/Perk_Hp.obj");
  this->setComponent<MeshComponent>(this->perk, perkHp);
  Transform& perkTrans = this->getComponent<Transform>(this->perk);
  perkTrans.position = glm::vec3(30.f, 5.f, 20.f);
  perkTrans.scale = glm::vec3(2.f, 2.f, 2.f);
  this->setComponent<Collider>(this->perk, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
  this->setComponent<Perks>(this->perk);
  Perks& perkSetting = this->getComponent<Perks>(this->perk);
  perkSetting.multiplier = 1.f;
  perkSetting.perkType = hpUpPerk;
  this->setScriptComponent(this->perk, "scripts/spin.lua");

  this->perk1 = this->createEntity();
  int perkDmg = this->getResourceManager()->addMesh("assets/models/Perk_Hp.obj");
  this->setComponent<MeshComponent>(this->perk1, perkDmg);
  Transform& perkTrans1 = this->getComponent<Transform>(this->perk1);
  perkTrans1.position = glm::vec3(30.f, 5.f, -20.f);
  perkTrans1.scale = glm::vec3(2.f, 2.f, 2.f);
  this->setComponent<Collider>(this->perk1, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true));
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

}

void GameSceneNetwork::update()
{
    
            // TODO: Move to SpawnHandler ---- 
    if (this->roomHandler.playerNewRoom(this->playerID))
    {
        this->newRoomFrame = true;
    }
    if (this->hasComponents<HealthComp>(this->playerID))
    {
        if (this->getComponent<HealthComp>(this->playerID).health <= 0.0f)
        {
            this->switchScene(new GameOverScene(), "scripts/GameOverScene.lua");
        }
    }
    Combat& playerCombat = this->getComponent<Combat>(this->playerID);
    this->getUIRenderer()->setTexture(abilityTextures[playerCombat.ability.abilityType]);
    this->getUIRenderer()->renderTexture(glm::vec2(890.f, -390.f), glm::vec2(100.0f));

    float perkXPos = -720.f;
    float perkYPos = -500.f;
    for (size_t i = 0; i < 4; i++)
      {

        this->getUIRenderer()->setTexture(
            perkTextures[playerCombat.perks[i].perkType]
        );
        this->getUIRenderer()->renderTexture(
            glm::vec2(-perkXPos - 70.f + i * 80.f, perkYPos + 10.f),
            glm::vec2(70.0f)
        );
      }

    // Render HP bar UI
    HealthComp& playerHealth = this->getComponent<HealthComp>(this->playerID);
    float hpPercent = playerHealth.health * 0.01f;
    float maxHpPercent = playerHealth.maxHealth * 0.01f;
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

  this->roomHandler.imgui(this->getDebugRenderer());

#endif
}

bool GameSceneNetwork::allDead()
{
 bool ret = true;
	for(auto p: swarmIDs)
	{
		if(this->isActive(p))
		{
			ret = false;
			break;
		}
	}
	if(ret)
	{
		for(auto p: tankIDs)
		{
			if(this->isActive(p))
			{
				ret = false;
				break;
			}
		}
		if(ret)
		{
			for(auto p: lichIDs)
			{
				if(this->isActive(p))
				{
					ret = false;
					break;
				}
			}
		}
		
	}
	
	return ret;
}

void GameSceneNetwork::onTriggerStay(Entity e1, Entity e2)
{
Entity player = e1 == this->playerID ? e1 : e2 == this->playerID ? e2 : -1;
	
	if (player == this->playerID) // player triggered a trigger :]
	{
		Entity other = e1 == player ? e2 : e1;
	
		if (other == this->portal && this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1) // -1 not counting start room
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

//TODO : don't know if this should be in server or not?
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

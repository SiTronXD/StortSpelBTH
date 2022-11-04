#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/AiCombatSystem.hpp"
#include "GameOverScene.h"

#ifdef _CONSOLE
// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);
#endif

GameScene::GameScene() 
	:playerID(-1), portal(-1), numRoomsCleared(0), newRoomFrame(false), perk(-1)
{
}

GameScene::~GameScene()
{
	for (auto& p : swarmGroups)
	{
		delete p;
	}
}

void GameScene::init()
{
	int swarm = this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
	
	roomHandler.init(this, this->getResourceManager(), this->getConfigValue<int>("room_size"), this->getConfigValue<int>("tile_types"));
	roomHandler.generate();
	createPortal();

	this->hpBarBackgroundTextureID =
		this->getResourceManager()->addTexture("assets/textures/UI/hpBarBackground.png");
	this->hpBarTextureID = 
		this->getResourceManager()->addTexture("assets/textures/UI/hpBar.png");

    // Add textures for ui renderer
	TextureSamplerSettings samplerSettings{};
	samplerSettings.filterMode = vk::Filter::eNearest;	
	samplerSettings.unnormalizedCoordinates = VK_TRUE;
    this->fontTextureIndex = Scene::getResourceManager()->addTexture("assets/textures/testBitmapFont.png", { samplerSettings, true });

	Scene::getUIRenderer()->setBitmapFont(
		{
			"abcdefghij",
			"klmnopqrst",
			"uvwxyz+-.'",
			"0123456789",
			"!?,<>:()#^",
			"@         "
		},
		fontTextureIndex,
		16, 16
	);
}

void GameScene::start()
{
	std::string playerName = "playerID";
	this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);
	
	this->setComponent<Combat>(playerID);
	this->createSystem<CombatSystem>(this, this->playerID, this->getPhysicsEngine(), this->getDebugRenderer());

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
	perkSetting.perkType = hpUp;

	this->perk1 = this->createEntity();
	int perkDmg = this->getResourceManager()->addMesh("assets/models/Perk_Dmg.obj");
	this->setComponent<MeshComponent>(this->perk1, perkDmg);
	Transform& perkTrans1 = this->getComponent<Transform>(this->perk1);
	perkTrans1.position = glm::vec3(30.f, 5.f, -20.f);
	perkTrans1.scale = glm::vec3(2.f, 2.f, 2.f);
    this->setComponent<Collider>(
        this->perk1, Collider::createSphere(2.f, glm::vec3(0, 0, 0), true)
    );
	this->setComponent<Perks>(this->perk1);
	Perks& perkSetting1 = this->getComponent<Perks>(this->perk1);
	perkSetting1.multiplier = 1.f;
	perkSetting1.perkType = dmgUp;

	this->perk2 = this->createEntity();
	int perkAtkSpeed = this->getResourceManager()->addMesh("assets/models/Perk_AtkSpeed.obj");
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
	perkSetting2.perkType = attackSpeedUp;

}

void GameScene::update()
{
	//if (allDead() && this->newRoomFrame) 
	//{
	//	this->newRoomFrame = false;
	//
	//	// Call when a room is cleared
	//	roomHandler.roomCompleted();
	//	this->numRoomsCleared++;
	//
	//	if (this->numRoomsCleared >= this->roomHandler.getNumRooms() - 1)
	//	{
	//		this->getComponent<MeshComponent>(portal).meshID = portalOnMesh;
	//	}
	//}

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

	// Render HP bar UI
	float hpPercent = 1.0f;
	float maxHpPercent = 1.0f;
	if (this->hasComponents<Combat>(this->playerID))
	{
		hpPercent = 
			this->getComponent<Combat>(this->playerID).health * 0.01f;
		maxHpPercent =
			this->getComponent<Combat>(this->playerID).maxHealth * 0.01f;
	}
	float xPos = -720.f;
	float yPos = -500.f;
	float xSize = 1024.f * 0.35f;
	float ySize = 64.f * 0.35f;

	Scene::getUIRenderer()->setTexture(this->hpBarBackgroundTextureID);
	Scene::getUIRenderer()->renderTexture(xPos - (1.0f - maxHpPercent) * xSize * 0.5f, yPos, (xSize * maxHpPercent) + 10, ySize + 10);
	Scene::getUIRenderer()->setTexture(this->hpBarTextureID);
	Scene::getUIRenderer()->renderTexture(xPos - (1.0f - hpPercent) * xSize * 0.5f, yPos, xSize * hpPercent, ySize);
	
#ifdef _CONSOLE

	static bool renderDebug = false;
	if (ImGui::Begin("Debug"))
	{
		if (ImGui::Checkbox("Render debug shapes", &renderDebug))
		{
			this->getPhysicsEngine()->renderDebugShapes(renderDebug);
		}
		const glm::vec3& playerPos = this->getComponent<Transform>(playerID).position;
		ImGui::Text("Player pos: (%d, %d, %d)", (int)playerPos.x, (int)playerPos.y, (int)playerPos.z);
		ImGui::Separator();
	}
	ImGui::End();

	roomHandler.imgui();

	decreaseFps();
#endif

}


void GameScene::onTriggerStay(Entity e1, Entity e2)
{
	Entity player = e1 == playerID ? e1 : e2 == playerID ? e2 : -1;
	
	if (player == playerID) // player triggered a trigger :]
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
					Transform& transform = this->getComponent<Transform>(this->enemyIDs[idx]);
					Transform& tileTrans = this->getComponent<Transform>(entity);
					float tileWidth = rand() % ((int)RoomHandler::TILE_WIDTH/2) + 0.01f;
					transform.position = tileTrans.position;
					transform.position = transform.position + glm::vec3(tileWidth, 0.f, tileWidth);
                

					//Temporary enemie reset
					SwarmComponent& swarmComp = this->getComponent<SwarmComponent>(this->enemyIDs[idx]);
					transform.scale.y = 1.0f;
					swarmComp.life = swarmComp.FULL_HEALTH;
					swarmComp.group->inCombat = false;

					idx++;
					counter++;
				
				}
			}
		}        

		if (other == portal && numRoomsCleared >= this->roomHandler.getNumRooms() - 1) // -1 not counting start room
		{
			this->switchScene(new GameScene(), "scripts/gamescene.lua");
		}
	}
}

void GameScene::onCollisionStay(Entity e1, Entity e2)
{
    Entity player = e1 == playerID ? e1 : e2 == playerID ? e2 : -1;
	
	if (player == playerID) // player triggered a trigger :]
	{
		Entity other = e1 == player ? e2 : e1;
        if(this->hasComponents<SwarmComponent>(other))
        {
            auto& swarmComp = this->getComponent<SwarmComponent>(other);
            if(swarmComp.inAttack)
            {
                auto& aiCombat = this->getComponent<AiCombat>(other);
                swarmComp.inAttack = false; 
                this->getComponent<Combat>(player).health -= aiCombat.lightHit;
                std::cout << "WAS HIT\n";
                
            }            
        }
    }
}

void GameScene::createPortal()
{
	glm::vec3 portalTriggerDims(6.f, 18.f, 1.f);
	glm::vec3 portalBlockDims(3.f, 18.f, 3.f);

	portalOffMesh = this->getResourceManager()->addMesh("assets/models/PortalOff.obj");
	portalOnMesh = this->getResourceManager()->addMesh("assets/models/PortalOn.obj");

	portal = this->createEntity();
	this->getComponent<Transform>(portal).position = this->roomHandler.getExitRoom().position;
    this->setComponent<Collider>(
        portal, Collider::createBox(portalTriggerDims, glm::vec3(0, 0, 0), true)
    );

	this->setComponent<MeshComponent>(portal);
	this->getComponent<MeshComponent>(portal).meshID = portalOffMesh;

	Entity collider1 = this->createEntity();
	this->getComponent<Transform>(collider1).position = this->getComponent<Transform>(portal).position;
	this->getComponent<Transform>(collider1).position.x += 9.f;
	this->getComponent<Transform>(collider1).position.y += 9.f;
	this->setComponent<Collider>(collider1, Collider::createBox(portalBlockDims));

	Entity collider2 = this->createEntity();
	this->getComponent<Transform>(collider2).position = this->getComponent<Transform>(portal).position;
	this->getComponent<Transform>(collider2).position.x -= 9.f;
	this->getComponent<Transform>(collider2).position.y += 9.f;
	this->setComponent<Collider>(collider2, Collider::createBox(portalBlockDims));
}

#ifdef _CONSOLE
void decreaseFps()
{
	static double result = 1234567890.0;

	static int num = 0;
	if (ImGui::Begin("Debug"))
	{
		ImGui::Text("Fps %f", 1.f / Time::getDT());
		ImGui::InputInt("Loops", &num);
	}
	ImGui::End();

	for (int i = 0; i < num; i++)
	{
		result /= std::sqrt(heavyFunction(result));
		result /= std::sqrt(heavyFunction(result));
		result /= std::sqrt(heavyFunction(result));
		result /= std::sqrt(heavyFunction(result));
		result /= std::sqrt(heavyFunction(result));
	}
}

double heavyFunction(double value)
{
	double result = 1234567890.0;
	for (size_t i = 0; i < 3000; i++)
	{
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 0.892375892))));
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 1.476352734))));
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 2.248923885))));
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 3.691284908))));
		result /= std::sqrt(std::sqrt(std::sqrt(std::sqrt(value * 3.376598278))));
	}

	return result;
}
#endif

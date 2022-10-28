#include "GameScene.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/AiMovementSystem.hpp"
#include "../Systems/AiCombatSystem.hpp"

// decreaseFps used for testing game with different framerates
void decreaseFps();
double heavyFunction(double value);

GameScene::GameScene() : playerID(-1)
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

	this->hpBarBackgroundTextureID =
		this->getResourceManager()->addTexture("assets/textures/UI/hpBarBackground.png");
	this->hpBarTextureID = 
		this->getResourceManager()->addTexture("assets/textures/UI/hpBar.png");
}

void GameScene::start()
{
	std::string playerName = "playerID";
	this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);
	//int spinningMesh = this->getResourceManager()->addMesh("assets/models/Hurricane Kick.fbx");
	//int runningMesh = this->getResourceManager()->addMesh("assets/models/run_forward.fbx");
	this->setComponent<Combat>(playerID);
	//this->setComponent<MeshComponent>(this->playerID, runningMesh);
	this->createSystem<CombatSystem>(this, this->playerID);

	uint32_t swordId = this->getResourceManager()->addMesh("assets/models/Sword.obj");
	
	Entity sword = this->createEntity();
	this->setComponent<MeshComponent>(sword);
	this->getComponent<MeshComponent>(sword).meshID = swordId;
    // Ai management 
    this->aiHandler = this->getAIHandler();
	this->aiHandler->init(this->getSceneHandler());
    
	aiExample();

}

void GameScene::update()
{
	if (Input::isKeyPressed(Keys::E)) 
	{
		// Call when a room is cleared
		roomHandler.roomCompleted();		
	}

	// Player entered a new room
	if (roomHandler.checkPlayer(this->getComponent<Transform>(playerID).position))
	{
		const std::vector<Entity>& entites = roomHandler.getFreeTiles();
		for (Entity entity : entites)
		{
			// Hi
		}
	}
	

	decreaseFps();

	// Render HP bar UI
	float hpPercent = 1.0f;
	if (this->hasComponents<Combat>(this->playerID))
	{
		hpPercent =
			this->getComponent<Combat>(this->playerID).health * 0.01f;
	}
	float xPos = -720;
	float yPos = -500;
	float xSize = 1024 * 0.35;
	float ySize = 64 * 0.35;

	Scene::getUIRenderer()->setTexture(this->hpBarBackgroundTextureID);
	Scene::getUIRenderer()->renderTexture(xPos, yPos, xSize + 10, ySize + 10);
	Scene::getUIRenderer()->setTexture(this->hpBarTextureID);
	Scene::getUIRenderer()->renderTexture(xPos - (1.0 - hpPercent) * xSize * 0.5, yPos, xSize * hpPercent, ySize);
}

void GameScene::aiExample() 
{
	auto a = [&](FSM* fsm, uint32_t entityId) -> void {
		SwarmFSM* swarmFSM = (SwarmFSM*)fsm;
        auto& entitySwarmComponent = this->getSceneHandler()->getScene()->getComponent<SwarmComponent>(entityId);
        auto& entityAiCombatComponent = this->getSceneHandler()->getScene()->getComponent<AiCombat>(entityId);
        auto& entiyFSMAgentComp = this->getSceneHandler()->getScene()->getComponent<FSMAgentComponent>(entityId);
		int& health            = entitySwarmComponent.life;
        float& speed           = entitySwarmComponent.speed;
        float& attackRange     = entitySwarmComponent.attackRange;
        float& sightRange      = entitySwarmComponent.sightRadius;
        bool& inCombat         = entitySwarmComponent.inCombat;
        float& attackPerSec    = entityAiCombatComponent.lightAttackTime;
		float& lightAttackDmg  = entityAiCombatComponent.lightHit;
		std::string& status    = entiyFSMAgentComp.currentNode->status;
		
		ImGui::SliderInt("health", &health, 0, 100);
		ImGui::SliderFloat("speed", &speed, 0, 100);
		ImGui::SliderFloat("attackRange", &attackRange, 0, 100);
		ImGui::SliderFloat("sightRange", &sightRange, 0, 100);		
		ImGui::InputFloat("attack/s", &attackPerSec);		
		ImGui::InputFloat("lightattackDmg", &lightAttackDmg);		
		ImGui::Checkbox("inCombat", &inCombat);		
        ImGui::Text(status.c_str());
	};
	static SwarmFSM swarmFSM;
	this->aiHandler->addFSM(&swarmFSM, "swarmFSM");
	this->aiHandler->addImguiToFSM("swarmFSM", a);

	int swarmModel = this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");

	int num_blobs = 6;
	int group_size = 3;
	for (int i = 0; i < num_blobs; i++)
    {
		this->swarmEnemies.push_back(this->createEntity());   
		Transform& trans = this->getSceneHandler()->getScene()->getComponent<Transform>(this->swarmEnemies.back());
        trans.position.x += rand()%50 - 25;
        trans.position.z += rand()%50 - 25;
		this->setComponent<MeshComponent>(this->swarmEnemies.back(), swarmModel);
		this->aiHandler->createAIEntity(this->swarmEnemies.back(), "swarmFSM");
        if ((i)%group_size == 0)
        {
			this->swarmGroups.push_back(new SwarmGroup);
        }
        this->swarmGroups.back()->members.push_back(this->swarmEnemies.back());
        this->getSceneHandler()->getScene()->getComponent<SwarmComponent>(this->swarmEnemies.back()).group = this->swarmGroups.back();
	}
    
}

void decreaseFps()
{
	static double result = 1234567890.0;

	static int num = 0;
	if (ImGui::Begin("FPS decrease"))
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
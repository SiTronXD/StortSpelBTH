#include "GameOverScene.h"
#include "MainMenu.h"

GameOverScene::GameOverScene() : graveStone(-1), cam(-1), dirLight(-1), ground(-1), groundPillar(-1)
{
	for (size_t i = 0; i < 4; i++)
	{
		this->lightPillars[i] = -1;
		this->crystals[i] = -1;
	}
	for (size_t j = 0; j < 40; j++)
	{
		this->trees[j] = -1;
	}
}

GameOverScene::~GameOverScene()
{
}

void GameOverScene::init() 
{
	this->cam = this->createEntity();
	Transform& camTrans = this->getComponent<Transform>(this->cam);
	camTrans.rotation.x = (30.f);
	this->setComponent<Camera>(this->cam);
	this->setMainCamera(this->cam);

	int graveStoneMesh = this->getResourceManager()->addMesh("assets/models/GameOverScene/GraveStone.obj");
	int lightPillarMesh = this->getResourceManager()->addMesh("assets/models/GameOverScene/LightPillar.obj");
	int ground = this->getResourceManager()->addMesh("assets/models/Tiles/Floor.obj");
	int trees = this->getResourceManager()->addMesh("assets/models/Tiles/Border/1.obj");
	int groundTopMesh = this->getResourceManager()->addMesh("assets/models/GameOverScene/GroundTop.obj");
	int crystalMesh = this->getResourceManager()->addMesh("assets/models/GameOverScene/Crystal.obj");

	this->graveStone = this->createEntity();
	this->setComponent<MeshComponent>(this->graveStone, graveStoneMesh);
	Transform& graveStoneTrans = this->getComponent<Transform>(this->graveStone);
	graveStoneTrans.position.y = 20.f;
	this->setComponent<Spotlight>(this->graveStone, glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(2.f, 0.5f, 0.5f), 90.f);

	// SPAWN IN LIGHT HOLDERS
	float offsetVal = 4.f;
	std::vector<glm::vec3> offsetVec =
	{
		glm::vec3(offsetVal, 0.f, offsetVal),
		glm::vec3(-offsetVal, 0.f, offsetVal),
		glm::vec3(offsetVal, 0.f, -offsetVal),
		glm::vec3(-offsetVal, 0.f, -offsetVal)
	};
	for (size_t i = 0; i < 4; i++)
	{
		this->lightPillars[i] = this->createEntity();
		this->setComponent<MeshComponent>(this->lightPillars[i], lightPillarMesh);
		this->crystals[i] = this->createEntity();
		this->setComponent<MeshComponent>(this->crystals[i], crystalMesh);
		this->setComponent<Spotlight>(this->crystals[i], glm::vec3(0.f, 2.f, 0.f), glm::vec3(0.f, -1.f, 0.f), glm::vec3(2.f, 0.5f, 0.5f), 90.f);

		Transform& lightPillarTrans = this->getComponent<Transform>(this->lightPillars[i]);
		lightPillarTrans.position = offsetVec[i];
		Transform& crystalTrans = this->getComponent<Transform>(this->crystals[i]);
		crystalTrans.position = offsetVec[i];
		this->setScriptComponent(this->crystals[i], "scripts/spin.lua");
		this->getScriptHandler()->setScriptComponentValue(this->getComponent<Script>(this->crystals[i]), 0.f, "floatValue");
		this->getScriptHandler()->setScriptComponentValue(this->getComponent<Script>(this->crystals[i]), 0.1f, "sinMulti");
	}

	// GENERATE TREES
	for (size_t j = 0; j < 7; j++)
	{
		this->trees[j] = this->createEntity();
		this->setComponent<MeshComponent>(this->trees[j], trees);
		Transform& treeTrans = this->getComponent<Transform>(this->trees[j]);
		treeTrans.position = glm::vec3(15.f, -0.3f, j * 2.5f);
		treeTrans.scale = glm::vec3(0.1f);
	}	
	for (size_t k = 0; k < 6; k++)
	{
		this->trees[k + 7] = this->createEntity();
		this->setComponent<MeshComponent>(this->trees[k+7], trees);
		Transform& treeTrans = this->getComponent<Transform>(this->trees[k+7]);
		treeTrans.position = glm::vec3(15.f, -0.3f, -2.5f - (k * 2.5));
		treeTrans.scale = glm::vec3(0.1f);
	}
	for (size_t l = 0; l < 7; l++)
	{
		this->trees[l + 13] = this->createEntity();
		this->setComponent<MeshComponent>(this->trees[l + 13], trees);
		Transform& treeTrans = this->getComponent<Transform>(this->trees[l + 13]);
		treeTrans.position = glm::vec3(-15.f, -0.3f, l * 2.5f);
		treeTrans.scale = glm::vec3(0.1f);
	}
	for (size_t m = 0; m < 6; m++)
	{
		this->trees[m + 20] = this->createEntity();
		this->setComponent<MeshComponent>(this->trees[m + 20], trees);
		Transform& treeTrans = this->getComponent<Transform>(this->trees[m + 20]);
		treeTrans.position = glm::vec3(-15.f, -0.3f, -2.5f - (m * 2.5f));
		treeTrans.scale = glm::vec3(0.1f);
	}

	// TOP FLOOR AND GROUND
	this->groundPillar = this->createEntity();
	this->setComponent<MeshComponent>(this->groundPillar, groundTopMesh);
	this->ground = this->createEntity();
	this->setComponent<MeshComponent>(this->ground, ground);
	Transform& groundTrans = this->getComponent<Transform>(this->ground);
	groundTrans.position.y = -0.3;
	groundTrans.scale = glm::vec3(30.f);

	// DIRLIGHT FOR SCENE
	this->dirLight = this->createEntity();
	this->setComponent<DirectionalLight>(
		this->dirLight,
		glm::vec3(-0.5f, -1.0f, -0.5f),
		glm::vec3(0.5f)
		);
	DirectionalLight& dirLight = this->getComponent<DirectionalLight>(this->dirLight);
	dirLight.cascadeSizes[0] = 0.044f;
	dirLight.cascadeSizes[1] = 0.149f;
	dirLight.cascadeSizes[2] = 1.0f;
	dirLight.cascadeDepthScale = 36.952f;
	dirLight.shadowMapMinBias = 0.00001f;
	dirLight.shadowMapAngleBias = 0.0004f;
}

void GameOverScene::start() {}

void GameOverScene::update()
{
	// Switch scene
	if (Input::isKeyPressed(Keys::SPACE))
	{
		this->switchScene(new MainMenu(), "scripts/MainMenu.lua");
	}
	Transform& graveTrans = this->getComponent<Transform>(this->graveStone);
	Transform& camTrans = this->getComponent<Transform>(this->cam);
	camTrans.rotation.y += this->camRotSpeed * Time::getDT();
	camTrans.updateMatrix();
	camTrans.position = graveTrans.position - (camTrans.forward() * this->camDist);

	if (graveTrans.position.y < 0.f)
	{
		graveTrans.position.y = 0.f;
	}
	else if (graveTrans.position.y > 0.001f)
	{
		graveTrans.position.y -= 10.f * Time::getDT();
	}

	////Apply rotation
	//self.transform.rotation = camRot
	//local forward = self.transform:forward()

	////Apply position
	//local scaledFwd = forward * (actualDist - self.distMargin)
	//self.transform.position = targetPos - scaledFwd
}

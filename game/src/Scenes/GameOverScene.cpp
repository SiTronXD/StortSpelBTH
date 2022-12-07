#include "GameOverScene.h"
#include "MainMenu.h"

GameOverScene::GameOverScene() : graveStone(-1), cam(-1), dirLight(-1), ground(-1), groundPillar(-1)
{
	for (size_t i = 0; i < 4; i++)
	{
		this->lightPillars[i] = -1;
		this->crystals[i] = -1;
	}
	for (size_t j = 0; j < 148; j++)
	{
		this->trees[j] = -1;
	}
	for (size_t k = 0; k < 60; k++)
	{
		this->stones[k] = -1;
	}
}

GameOverScene::~GameOverScene()
{
}

void GameOverScene::init() 
{
	this->getNetworkHandler()->disconnectClient();
	this->getNetworkHandler()->deleteServer();

	TextureSamplerSettings samplerSettings{};
	samplerSettings.filterMode = vk::Filter::eNearest;
	samplerSettings.unnormalizedCoordinates = VK_TRUE;

	uint32_t fontTextureId = Scene::getResourceManager()->addTexture("assets/textures/UI/font.png", { samplerSettings, true });
	Scene::getUIRenderer()->setBitmapFont(
		{
			"abcdefghij",
			"klmnopqrst",
			"uvwxyz+-.'",
			"0123456789",
			"!?,<>:()#^",
			"@%        "
		},
		fontTextureId,
		glm::uvec2(50, 50)
	);

	this->setBloomBufferLerpAlpha(0.340);
	this->setBloomNumMipLevels(7);

	this->cam = this->createEntity();
	Transform& camTrans = this->getComponent<Transform>(this->cam);
	camTrans.rotation.x = (40.f);
	this->setComponent<Camera>(this->cam);
	this->setMainCamera(this->cam);

	int graveStoneMesh = this->getResourceManager()->addMesh("assets/models/GameOverScene/GraveStone.obj");
	int lightPillarMesh = this->getResourceManager()->addMesh("assets/models/GameOverScene/LightPillar.obj");
	int ground = this->getResourceManager()->addMesh("assets/models/Tiles/Floor.obj");
	int trees = this->getResourceManager()->addMesh("assets/models/Tiles/Border/1.obj");
	int stones = this->getResourceManager()->addMesh("assets/models/Tiles/OneXOne/1.obj");
	int groundTopMesh = this->getResourceManager()->addMesh("assets/models/GameOverScene/GroundTop.obj");
	int crystalMesh = this->getResourceManager()->addMesh("assets/models/GameOverScene/Crystal.obj");

	this->graveStone = this->createEntity();
	this->setComponent<MeshComponent>(this->graveStone, graveStoneMesh);
	Transform& graveStoneTrans = this->getComponent<Transform>(this->graveStone);
	graveStoneTrans.position.y = 80.f;
	graveStoneTrans.rotation.y = 180.f;
	graveStoneTrans.scale *= 5.f;
	this->setComponent<PointLight>(this->graveStone, glm::vec3(0.f, 6.f, 4.f), 
		glm::vec3(2.f, 0.5f, 0.5f) * 3.f);

	// SPAWN IN LIGHT HOLDERS
	float offsetVal = 18.f;
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
		this->setComponent<PointLight>(this->crystals[i], glm::vec3(0.f, 10.f, 0.f), 
			glm::vec3(0.431f, 1.624f, 0.130f) * 10.f);

		Transform& lightPillarTrans = this->getComponent<Transform>(this->lightPillars[i]);
		lightPillarTrans.position = offsetVec[i];
		lightPillarTrans.scale *= 5.f;
		Transform& crystalTrans = this->getComponent<Transform>(this->crystals[i]);
		crystalTrans.position = offsetVec[i];
		crystalTrans.scale = glm::vec3(7.f);
		this->setScriptComponent(this->crystals[i], "scripts/spin.lua");
		this->getScriptHandler()->setScriptComponentValue(this->getComponent<Script>(
			this->crystals[i]), -5.f, "floatValue");
		this->getScriptHandler()->setScriptComponentValue(this->getComponent<Script>(
			this->crystals[i]), 0.5f, "sinMulti");
	}

	Material& crystalMat = this->getResourceManager()->getMaterial(this->getComponent<MeshComponent>(this->crystals[0]), 0);
	crystalMat.emissionColor = glm::vec3(0.431f, 1.624f, 0.130f);
	crystalMat.glowMapTextureIndex = this->getResourceManager()->addTexture("vengine_assets/textures/DefaultEmission.png");

	// GENERATE TREES
	float treeOffset = 12.f;
	float treePos = 200.f;
	int treeLineAmount = 37;
	int treeLineIdx = 37;
	for (size_t j = 0; j < treeLineAmount; j++)
	{
		this->trees[j] = this->createEntity();
		this->setComponent<MeshComponent>(this->trees[j], trees);
		Transform& treeTrans = this->getComponent<Transform>(this->trees[j]);
		treeTrans.position = glm::vec3(treePos, -1.f, -treePos + j * treeOffset);
		treeTrans.rotation.y = float(rand() % 360);
		treeTrans.scale = glm::vec3(0.5f);
	}
	for (size_t k = 0; k < treeLineAmount; k++)
	{
		this->trees[k + treeLineIdx] = this->createEntity();
		this->setComponent<MeshComponent>(this->trees[k + treeLineIdx], trees);
		Transform& treeTrans = this->getComponent<Transform>(this->trees[k + treeLineIdx]);
		treeTrans.position = glm::vec3(-treePos + k * treeOffset, -1.f, treePos);
		treeTrans.rotation.y = float(rand() % 360);
		treeTrans.scale = glm::vec3(0.5f);
	}
	treeLineIdx = 37 * 2;
	for (size_t l = 0; l < treeLineAmount; l++)
	{
		this->trees[l + treeLineIdx] = this->createEntity();
		this->setComponent<MeshComponent>(this->trees[l + treeLineIdx], trees);
		Transform& treeTrans = this->getComponent<Transform>(this->trees[l + treeLineIdx]);
		treeTrans.position = glm::vec3(-treePos, -1.f, -treePos + l * treeOffset);
		treeTrans.rotation.y = float(rand() % 360);
		treeTrans.scale = glm::vec3(0.5f);
	}
	treeLineIdx = 37 * 3;
	for (size_t m = 0; m < treeLineAmount; m++)
	{
		this->trees[m + treeLineIdx] = this->createEntity();
		this->setComponent<MeshComponent>(this->trees[m + treeLineIdx], trees);
		Transform& treeTrans = this->getComponent<Transform>(this->trees[m + treeLineIdx]);
		treeTrans.position = glm::vec3(-treePos + m * treeOffset, -1.f, -treePos);
		treeTrans.rotation.y = float(rand() % 360);
		treeTrans.scale = glm::vec3(0.5f);
	}

	// GENERATE STONES
	for (size_t n = 0; n < 60; n++)
	{
		this->stones[n] = this->createEntity();
		this->setComponent<MeshComponent>(this->stones[n], stones);
		Transform& stoneTrans = this->getComponent<Transform>(this->stones[n]);
		stoneTrans.position = glm::vec3(float(rand() % 30 + 50) * 
			float(rand() % 2 ? 1.f : -1.f), 0.f, float(rand() & 30 + 50) * float(rand() % 2 ? 1.f : -1.f));
		stoneTrans.rotation.y = float(rand() % 360);
		stoneTrans.scale = glm::vec3(float(rand() % 40 + 20) * 0.01f) * 5.f;
	}

	// TOP FLOOR AND GROUND
	this->groundPillar = this->createEntity();
	this->setComponent<MeshComponent>(this->groundPillar, groundTopMesh);
	Transform& groundPillarTrans = this->getComponent<Transform>(this->groundPillar);
	groundPillarTrans.scale *= 5.f;
	this->ground = this->createEntity();
	this->setComponent<MeshComponent>(this->ground, ground);
	Transform& groundTrans = this->getComponent<Transform>(this->ground);
	groundTrans.position.y = -1.;
	groundTrans.scale = glm::vec3(400.f);

	// DIRLIGHT FOR SCENE
	this->dirLight = this->createEntity();
	this->setComponent<DirectionalLight>(
		this->dirLight,
		glm::vec3(0.5f, -1.0f, 0.5f),
		glm::vec3(0.1f)
		);
	DirectionalLight& dirLight = this->getComponent<DirectionalLight>(this->dirLight);
	dirLight.cascadeSizes[0] = 47.0f;
	dirLight.cascadeSizes[1] = 144.0f;
	dirLight.cascadeSizes[2] = 500.0f;
	dirLight.cascadeDepthScale = 36.952f;
	dirLight.shadowMapMinBias = 0.00001f;
	dirLight.shadowMapAngleBias = 0.0004f;
}

void GameOverScene::start() {}

void GameOverScene::update()
{
	float dt = Time::getDT();
	// Switch scene
	if (Input::isKeyPressed(Keys::SPACE))
	{
		this->switchScene(new MainMenu(), "scripts/MainMenu.lua");
	}

	// Gravestone moving downwards and camera spinning
	Transform& graveTrans = this->getComponent<Transform>(this->graveStone);
	Transform& camTrans = this->getComponent<Transform>(this->cam);
	if (graveTrans.position.y == 0.f)
	{
		camTrans.rotation.y += this->camRotSpeed * dt;
		camTrans.updateMatrix();
	}
	else if (graveTrans.position.y < 0.f)
	{
		graveTrans.position.y = 0.f;
		this->shakeTimer = this->shakeDuration;
		this->isShaking = true;
	}
	else if (graveTrans.position.y > 0.001f)
	{
		graveTrans.position.y -= 40.f * dt;
	}

	// CAMERA SHAKE
	if (!this->isShaking)
	{
		camTrans.position = graveTrans.position - (camTrans.forward() * this->camDist);
	}
	else if (this->shakeTimer > 0.f)
	{
		camTrans.position = graveTrans.position - (camTrans.forward() * this->camDist) + 
			glm::vec3(float(rand() % 3 + 1) * 0.1f, float(rand() % 3 + 1) * 0.1f, float(rand() % 3 + 1) * 0.1f);
		this->shakeTimer -= dt;
	}
	else
	{
		this->isShaking = false;
	}

#ifdef _CONSOLE
	float bloomAlpha = this->getBloomSettings().bloomBufferLerpAlpha;
	int bloomMip = this->getBloomSettings().numBloomMipLevels;
	// Bloom Effect
	ImGui::Begin("Bloom");
	ImGui::SliderFloat("Bloom Alpha", &bloomAlpha, 0.f, 10.f);
	ImGui::SliderInt("Bloom Mip Level", &bloomMip, 0, 10);
	ImGui::End();
	this->setBloomBufferLerpAlpha(bloomAlpha);
	this->setBloomNumMipLevels(bloomMip);

	glm::vec3 emissionCrystal = this->getResourceManager()->getMaterial(
		this->getComponent<MeshComponent>(this->crystals[0]), 0).emissionColor;
	ImGui::Begin("CrystalBloom");
	ImGui::SliderFloat("Color R", &emissionCrystal[0], 0.f, 10.f);
	ImGui::SliderFloat("Color G", &emissionCrystal[1], 0.f, 10.f);
	ImGui::SliderFloat("Color B", &emissionCrystal[2], 0.f, 10.f);
	ImGui::End();
	this->getResourceManager()->getMaterial(this->getComponent<MeshComponent>(
		this->crystals[0]), 0).emissionColor = emissionCrystal;
#endif
}

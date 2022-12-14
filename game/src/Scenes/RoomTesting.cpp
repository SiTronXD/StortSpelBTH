#include "RoomTesting.h"

RoomTesting::RoomTesting() 
{
}

RoomTesting::~RoomTesting()
{

}

void RoomTesting::init()
{
	this->roomHandler.init(this, this->getResourceManager(), this->getPhysicsEngine(), true);
	this->roomHandler.generate(rand(), 0u);

	Entity cam = this->createEntity();
	this->setComponent<Camera>(cam);
	this->setMainCamera(cam);
	this->getComponent<Transform>(cam).rotation.x = 90.f;
	this->getComponent<Transform>(cam).position.y = 120.f;
	this->setComponent<DirectionalLight>(cam,
		{glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f)});
	DirectionalLight& light = this->getComponent<DirectionalLight>(cam);
	light.cascadeSizes[0] = 3000.f;
	light.cascadeSizes[1] = 3.f;
	light.cascadeSizes[2] = 3.f;
	light.cascadeDepthScale = 36.952f;
	light.shadowMapMinBias = 0.00001f;
	light.shadowMapAngleBias = 0.0004f;

	this->setFogStartDistance(9999999999999999999999.f);

	int playerMesh = this->getResourceManager()->addMesh("assets/models/Character/CharRun.fbx");
	player = this->createEntity();
	this->setComponent<MeshComponent>(player);
	this->getComponent<MeshComponent>(player).meshID = playerMesh;
}

void RoomTesting::start()
{
}

void RoomTesting::update()
{
#ifdef _CONSOLE
	roomHandler.imgui(getDebugRenderer());
#endif // _CONSOLE

	const float speed = 100.f;
	const float frameSpeed = speed * Time::getDT() * Input::isKeyDown(Keys::CTRL) ? 0.1f : Input::isKeyDown(Keys::SHIFT) ? 10.f : 1.f;

	Entity cam = this->getMainCameraID();
	Transform& camTra = this->getComponent<Transform>(cam);
	if (Input::isKeyDown(Keys::A))
		camTra.position.x += frameSpeed;
	if (Input::isKeyDown(Keys::D))
		camTra.position.x -= frameSpeed;
	if (Input::isKeyDown(Keys::W))
		camTra.position.z += frameSpeed;
	if (Input::isKeyDown(Keys::S))
		camTra.position.z -= frameSpeed;
	if (Input::isKeyDown(Keys::E))
		camTra.position.y += frameSpeed;
	if (Input::isKeyDown(Keys::Q))
		camTra.position.y -= frameSpeed;

	if (ImGui::Begin("aa"))
	{
		ImGui::Text("Cam pos: (%d, %d, %d)", (int)camTra.position.x,(int)camTra.position.y,(int)camTra.position.z);
		static bool colls = false;
		if (ImGui::Checkbox("Show colliders", &colls))
		{
			this->getPhysicsEngine()->renderDebugShapes(colls);
		}

	}
	ImGui::End();

	if (ImGui::Begin("Directional light"))
	{
		ImGui::PushItemWidth(-100.f);
		DirectionalLight& dirLight = this->getComponent<DirectionalLight>(cam);
		ImGui::SliderFloat("Cascade size 0", &dirLight.cascadeSizes[0], 0.0f, 1000.0f);
		ImGui::SliderFloat("Cascade size 1", &dirLight.cascadeSizes[1], 0.0f, 1000.0f);
		ImGui::SliderFloat("Cascade size 2", &dirLight.cascadeSizes[2], 0.0f, 1000.0f);
		ImGui::SliderFloat("Cascade depth", &dirLight.cascadeDepthScale, 1.0f, 50.0f);
		ImGui::Checkbox("Visualize cascades", &dirLight.cascadeVisualization);
		ImGui::SliderFloat("Shadow map angle bias", &dirLight.shadowMapAngleBias, 0.0f, 0.005f);
		ImGui::PopItemWidth();
	}
	ImGui::End();

	Transform& plaTra = this->getComponent<Transform>(player);
	if (Input::isKeyDown(Keys::LEFT))
		plaTra.position.x += frameSpeed;
	if (Input::isKeyDown(Keys::RIGHT))
		plaTra.position.x -= frameSpeed;
	if (Input::isKeyDown(Keys::UP))
		plaTra.position.z += frameSpeed;
	if (Input::isKeyDown(Keys::DOWN))
		plaTra.position.z -= frameSpeed;
}

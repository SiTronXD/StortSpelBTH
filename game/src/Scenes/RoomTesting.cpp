#include "RoomTesting.h"

RoomTesting::RoomTesting() 
{
}

RoomTesting::~RoomTesting()
{

}

void RoomTesting::init()
{
	this->roomHandler.init(this, this->getResourceManager(), this->getPhysicsEngine(), false);
	this->roomHandler.generate(123);

	Entity cam = this->createEntity();
	this->setComponent<Camera>(cam);
	this->setMainCamera(cam);
	this->getComponent<Transform>(cam).rotation.x = 90.f;
	this->getComponent<Transform>(cam).position.y = 120.f;
	this->setComponent<DirectionalLight>(cam,
		{glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f)});
	DirectionalLight& light = this->getComponent<DirectionalLight>(cam);
	light.cascadeSizes[0] = 400.f;
	light.cascadeSizes[1] = 3.f;
	light.cascadeSizes[2] = 3.f;
	light.cascadeDepthScale = 36.952f;
	light.shadowMapMinBias = 0.00001f;
	light.shadowMapAngleBias = 0.0004f;


	int playerMesh = this->getResourceManager()->addMesh("assets/models/Character/CharRun.fbx");

	player1 = this->createEntity();
	this->setComponent<MeshComponent>(player1);
	this->getComponent<MeshComponent>(player1).meshID = playerMesh;
	this->setComponent<Collider>(player1, Collider::createCapsule(2.f, 10.f, glm::vec3(0.f, 7.3f, 0.f)));

	player2 = this->createEntity();
	this->setComponent<MeshComponent>(player2);
	this->getComponent<MeshComponent>(player2).meshID = playerMesh;
	this->setComponent<Collider>(player2, Collider::createCapsule(2.f, 10.f, glm::vec3(0.f, 7.3f, 0.f)));

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
	const float frameSpeed = speed * Time::getDT() * Input::isKeyDown(Keys::CTRL) ? 0.1f : Input::isKeyDown(Keys::SHIFT) ? 3.f : 1.f;

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

	if (roomHandler.playerNewRoom(player1))
	{
		printf("New room Player 1\n");
	}
	//if (roomHandler.playersInPathway(player1, player2))
	//{
	//	printf("Both players in pathway\n");
	//}
	
	if (ImGui::Begin("aa"))
	{
		ImGui::Text("Cam pos: (%.1f, %.1f, %.1f)", camTra.position.x, camTra.position.y, camTra.position.z);
		static bool colls = false;
		static bool follow = false;
		static bool follow2 = false;
		
		ImGui::Checkbox("Show colliders", &colls);
		ImGui::Checkbox("player1 follow", &follow);
		ImGui::Checkbox("player2 follow", &follow2);
		
		this->getPhysicsEngine()->renderDebugShapes(colls);
		
		if (follow)
		{
			Transform& plaTra = this->getComponent<Transform>(player1);
			plaTra.position.x = camTra.position.x;
			plaTra.position.y = 0.f;
			plaTra.position.z = camTra.position.z;
		}
		if (follow2)
		{
			Transform& plaTra = this->getComponent<Transform>(player2);
			plaTra.position.x = camTra.position.x;
			plaTra.position.y = 0.f;
			plaTra.position.z = camTra.position.z;
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
}

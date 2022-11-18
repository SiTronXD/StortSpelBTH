#include "RoomTesting.h"

RoomTesting::RoomTesting() 
{
}

RoomTesting::~RoomTesting()
{

}

void RoomTesting::init()
{
	srand(123);

	this->roomHandler.init(this, this->getResourceManager(), 15, this->getConfigValue<int>("tile_types"));
	this->roomHandler.generate2();

	Entity cam = this->createEntity();
	this->setComponent<Camera>(cam);
	this->setMainCamera(cam);
	this->getComponent<Transform>(cam).rotation.x = 90.f;
	this->getComponent<Transform>(cam).position.y = 400.f;
	this->setComponent<DirectionalLight>(cam,
		{glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f)});
	DirectionalLight& light = this->getComponent<DirectionalLight>(cam);
	light.shadowMapFrustumHalfWidth = 1000.0f;
	light.shadowMapFrustumHalfHeight = 1000.0f;
	light.shadowMapFrustumDepth = 2000.0f;
	light.shadowMapMinBias = 0.0001f;
	light.shadowMapAngleBias = 0.001f;


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
	roomHandler.imgui(this->getPhysicsEngine());
#endif // _CONSOLE


	static float speed = 10.f;
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

	if (ImGui::Begin("aa"))
	{
		ImGui::Text("Cam pos: (%d, %d, %d)", (int)camTra.position.x,(int)camTra.position.y,(int)camTra.position.z);
		static bool colls = false;
		ImGui::Checkbox("Show colliders", &colls);
		this->getPhysicsEngine()->renderDebugShapes(colls);

		ImGui::DragFloat("Speed", &speed, 0.1f, 0.f, 100.f);
	}ImGui::End();

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

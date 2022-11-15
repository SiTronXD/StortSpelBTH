#include "RoomTesting.h"

RoomTesting::RoomTesting() 
{
}

RoomTesting::~RoomTesting()
{

}

void RoomTesting::init()
{
	srand(89734);

	this->roomHandler.init(this, this->getResourceManager(), 15, this->getConfigValue<int>("tile_types"));
	this->roomHandler.generate2();

	Entity cam = this->createEntity();
	this->setComponent<Camera>(cam);
	this->setMainCamera(cam);
	this->getComponent<Transform>(cam).rotation.x = 90.f;
	this->getComponent<Transform>(cam).position.y = 400.f;
	this->setComponent<DirectionalLight>(cam,
		{glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f)});


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
	roomHandler.imgui();
#endif // _CONSOLE


	const float speed = 50.f * Time::getDT() * Input::isKeyDown(Keys::SHIFT) ? 0.1f : 1.f;

	Entity cam = this->getMainCameraID();
	Transform& camTra = this->getComponent<Transform>(cam);
	if (Input::isKeyDown(Keys::A))
		camTra.position.x += speed * 10.f;
	if (Input::isKeyDown(Keys::D))
		camTra.position.x -= speed * 10.f;
	if (Input::isKeyDown(Keys::W))
		camTra.position.z += speed * 10.f;
	if (Input::isKeyDown(Keys::S))
		camTra.position.z -= speed * 10.f;
	if (Input::isKeyDown(Keys::E))
		camTra.position.y += speed * 10.f;
	if (Input::isKeyDown(Keys::Q))
		camTra.position.y -= speed * 10.f;

	Transform& plaTra = this->getComponent<Transform>(player);
	if (Input::isKeyDown(Keys::LEFT))
		plaTra.position.x += speed;
	if (Input::isKeyDown(Keys::RIGHT))
		plaTra.position.x -= speed;
	if (Input::isKeyDown(Keys::UP))
		plaTra.position.z += speed;
	if (Input::isKeyDown(Keys::DOWN))
		plaTra.position.z -= speed;
}

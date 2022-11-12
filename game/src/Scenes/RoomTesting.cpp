#include "RoomTesting.h"

RoomTesting::RoomTesting() 
{
}

RoomTesting::~RoomTesting()
{

}

void RoomTesting::init()
{
	this->roomHandler.init(this, this->getResourceManager(), 15, this->getConfigValue<int>("tile_types"));
	this->roomHandler.genTilesOnly();

	Entity cam = this->createEntity();
	this->setComponent<Camera>(cam);
	this->setMainCamera(cam);
	this->getComponent<Transform>(cam).rotation.x = 90.f;
	this->getComponent<Transform>(cam).position.y = 400.f;
	this->setComponent<DirectionalLight>(cam,
		{glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f)});

	int tileFloorMesh = this->getResourceManager()->addMesh("assets/models/Tiles/Floor.obj");
	int meshId =  this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
	Entity swarm = this->createEntity();
	this->setComponent<MeshComponent>(swarm, tileFloorMesh);
	//this->getComponent<Transform>(swarm).scale *= RoomHandler::TILE_WIDTH;

	//int playerMesh = this->getResourceManager()->addMesh("assets/models/Character/CharRun.fbx");
	//player = this->createEntity();
	//this->setComponent<MeshComponent>(player);
	//this->getComponent<MeshComponent>(player).meshID = playerMesh;
}

void RoomTesting::start()
{
}

void RoomTesting::update()
{
#ifdef _CONSOLE
	roomHandler.imgui();
#endif // _CONSOLE


	const float speed = 50.f * Time::getDT();

	Entity cam = this->getMainCameraID();
	Transform& camTra = this->getComponent<Transform>(cam);
	if (Input::isKeyDown(Keys::A))
		camTra.position.x += speed * 5.f;
	if (Input::isKeyDown(Keys::D))
		camTra.position.x -= speed * 5.f;
	if (Input::isKeyDown(Keys::W))
		camTra.position.z += speed * 5.f;
	if (Input::isKeyDown(Keys::S))
		camTra.position.z -= speed * 5.f;
	if (Input::isKeyDown(Keys::E))
		camTra.position.y += speed * 5.f;
	if (Input::isKeyDown(Keys::Q))
		camTra.position.y -= speed * 5.f;

	return;
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

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
	this->getComponent<Transform>(cam).position.y = 200.f;
	this->setComponent<DirectionalLight>(cam,
		{glm::vec3(0.f, -1.f, 0.f), glm::vec3(1.f)});

	int tileFloorMesh = this->getResourceManager()->addMesh("assets/models/Tiles/Floor.obj");
	int meshId =  this->getResourceManager()->addMesh("assets/models/Swarm_Model.obj");
	Entity swarm = this->createEntity();
	this->setComponent<MeshComponent>(swarm, tileFloorMesh);
	//this->getComponent<Transform>(swarm).scale *= RoomHandler::TILE_WIDTH;

}

void RoomTesting::start()
{
}

void RoomTesting::update()
{

}

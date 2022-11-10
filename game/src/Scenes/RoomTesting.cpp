#include "RoomTesting.h"

RoomTesting::RoomTesting() 
{
}

RoomTesting::~RoomTesting()
{

}

void RoomTesting::init()
{
	this->roomHandler.init(this, this->getResourceManager(), 0, 0);
	this->roomHandler.genTilesOnly();

	Entity cam = this->createEntity();
	this->setMainCamera(cam);
	this->getComponent<Transform>(cam).rotation.x = -90.f;
	this->setComponent<DirectionalLight>(cam,
		{glm::vec3(-1.f), glm::vec3(1.f)});

}

void RoomTesting::start()
{
}

void RoomTesting::update()
{

}

#pragma once
#include "vengine.h"

class logInScene : public Scene
{
private:
	uint32_t fontTextureId;
	uint32_t backgroundId;
	std::string title;
	std::string name;
	std::string ipAddress;
	std::string* nameOrIp;
	bool incorrectIP = false;

	UIArea nameButton;
	UIArea ipButton;
	UIArea startButton;
	UIArea backButton;

	void write();

public:
	logInScene();
	virtual ~logInScene();

	// Inherited via Scene
	virtual void start() override;
	virtual void update() override;
	//virtual void onTriggerStay(Entity e1, Entity e2) override;
	//virtual void onCollisionStay(Entity e1, Entity e2) override;

private:
};

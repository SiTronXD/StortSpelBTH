#pragma once

#include "../Ai/Behaviors/Swarm/SwarmFSM.hpp"
#include "../World Handling/Room Handler.h"
#include "../Network/NetworkHandlerGame.h"
#include "vengine.h"
#include "vengine/network/NetworkEnumAndDefines.h"

class LobbyScene : public Scene
{
private:
	uint32_t fontTextureId;
	uint32_t backgroundId;

	uint32_t activePlayers;
	std::vector<Entity> players;
	std::vector<std::string> playersNames;

	static const int MAX_PLAYER_COUNT = 4;
	inline static const std::vector<glm::vec3> POSITIONS = { 
		glm::vec3(0, -10, 15), 
		glm::vec3(10, -10, 18), 
		glm::vec3(-10, -10, 18),
		glm::vec3(-5, -10, 21) 
	};

	int startButton;
	int disconnectButton;
	int light;

	sf::Packet helpPacket;
	NetworkHandlerGame* networkHandler;

public:
	LobbyScene();
	virtual ~LobbyScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void start() override;
	virtual void update() override;
};

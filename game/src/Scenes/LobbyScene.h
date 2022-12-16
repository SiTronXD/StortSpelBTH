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
	uint32_t fogGradientVerticalTextureId;
	uint32_t fogGradientHorizontalTextureId;
	uint32_t fogTextureId;
	uint32_t buttonId;
	uint32_t buttonSound;
	std::string serverIP;

	uint32_t activePlayers = 0;
	std::vector<Entity> players;
	std::vector<std::string> playersNames;
    int playerModel;

	static const int MAX_PLAYER_COUNT = 4;
	inline static const std::vector<glm::vec3> POSITIONS = { 
		glm::vec3(0, -10, 15), 
		glm::vec3(10, -10, 22), 
		glm::vec3(-10, -10, 22),
		glm::vec3(-5, -10, 28) 
	};

	Entity scene;

	UIArea startButton;
	UIArea disconnectButton;

	sf::Packet helpPacket;
	NetworkHandlerGame* networkHandler;

	void addCandle(glm::vec3 position);
	void renderUiFogs(
		const float& gradientHorizontalPos,
		const float& gradientVerticalPos);

	void preloadAssets();

public:
	LobbyScene(const std::string& serverIP = "");
	virtual ~LobbyScene();

	// Inherited via Scene
	virtual void init() override;
	virtual void start() override;
	virtual void update() override;
};

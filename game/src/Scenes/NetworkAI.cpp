#include "NetworkAI.h"

#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"

NetworkAI::NetworkAI(): camEntity(-1), player(-1) {}

NetworkAI::~NetworkAI() {}

void NetworkAI::init()
{
    this->camEntity = this->createEntity();
    this->setComponent<Camera>(this->camEntity, 1.0f);
    this->setMainCamera(this->camEntity);
    Transform& camTransform = this->getComponent<Transform>(this->camEntity);
    camTransform.position   = glm::vec3(1.0f);

    this->player = this->createEntity();
    this->setComponent<MeshComponent>(this->player);
    this->setComponent<Movement>(this->player);
    this->setComponent<Combat>(this->player);
    Transform& transform = this->getComponent<Transform>(this->player);
    transform.position = glm::vec3(0.0f, 0.0f, 20.0f);
    transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
    transform.scale = glm::vec3(5.0f);
    this->createSystem<CombatSystem>(this, player);
    this->createSystem<MovementSystem>(this, this->player);

    this->createSystem<CameraMovementSystem>(this, this->player);

    //ground
    int ground = this->createEntity();
    int groundMesh =
        this->getResourceManager()->addMesh("vengine_assets/models/Cube.fbx");

    this->setComponent<Transform>(ground);
    this->setComponent<MeshComponent>(ground, groundMesh);
    Transform& transform2 = this->getComponent<Transform>(ground);
    transform2.position = glm::vec3(0.0f, -10.0f, 0.0f);
    transform2.scale = glm::vec3(100.f, 0.1f, 100.f);

    int puzzleCreator = this->createEntity();
    this->setScriptComponent(puzzleCreator, "src/Scripts/PuzzleCreatorLua.lua");
}

#include "../ServerGameModes/TheServerGame.h"
void NetworkAI::update()
{
    Transform& transform = this->getComponent<Transform>(this->player);
    this->getNetworkHandler()->sendUDPDataToClient(transform.position, transform.rotation);

    if (Input::isKeyPressed(Keys::H)) {
        this->getNetworkHandler()->createServer(new TheServerGame());
        this->getNetworkHandler()->createClient();
        if (this->getNetworkHandler()->connectClientToThis())
          {
            std::cout << "connect" << std::endl;
          }
        else
          {
            std::cout << "no Connect" << std::endl;
        }
        //no visulation that we connected
    }
    if (Input::isKeyPressed(Keys::K)) {
        this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent { GameEvents::START });
    }
    if (Input::isKeyPressed(Keys::J)) {
        this->getNetworkHandler()->createClient("Cli");
        std::cout << "ip : ";
        std::string ip;
        std::cin >> ip;
        if (ip == "a") {
            ip = "192.168.1.104";
        }
        this->getNetworkHandler()->connectClient(ip);
    }
}

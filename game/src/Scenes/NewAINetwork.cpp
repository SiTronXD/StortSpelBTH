#include "NewAINetwork.hpp"

#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "../ServerGameModes/TheServerGame.h"

NewAINetwork::NewAINetwork(): camEntity(-1), player(-1) {}

NewAINetwork::~NewAINetwork() {}

void NewAINetwork::init()
{

    this->player = this->createEntity();
    this->setComponent<Combat>(this->player);
    Transform& transform = this->getComponent<Transform>(this->player);
    transform.position = glm::vec3(0.66f, 0.f, 7.9f);
    transform.rotation = glm::vec3(-90.0f, 0.0f, 0.0f);
    transform.scale = glm::vec3(5.0f);


    //ground
    int ground = this->createEntity();
    int groundMesh =
        this->getResourceManager()->addMesh("vengine_assets/models/Cube.fbx");
    this->setComponent<Collider>(ground, Collider::createBox(glm::vec3(100,1,100)));
    this->setComponent<Transform>(ground);
    this->setComponent<MeshComponent>(ground, groundMesh);
    Transform& transform2 = this->getComponent<Transform>(ground);
    transform2.position = glm::vec3(0.0f, -10.0f, 0.0f);
    transform2.scale = glm::vec3(100.f, 0.1f, 100.f);

    int puzzleCreator = this->createEntity();
    this->setScriptComponent(puzzleCreator, "src/Scripts/PuzzleCreatorLua.lua");
}

void NewAINetwork::start()
{
    
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


void NewAINetwork::update()
{

    if (Input::isKeyPressed(Keys::N)) {
        this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent { GameEvents::START });
    }
    if (Input::isKeyPressed(Keys::M)) {
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

#include "NewAINetwork.hpp"

#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "../ServerGameModes/TheServerGame.h"

NewAINetwork::NewAINetwork(): camEntity(-1), player(-1) {}

NewAINetwork::~NewAINetwork() {}

void NewAINetwork::init()
{

    //ground
    int ground = this->createEntity();
    int groundMesh =
        this->getResourceManager()->addMesh("vengine_assets/models/Cube.fbx");
    this->setComponent<Collider>(ground, Collider::createBox(glm::vec3(100,1,100)));
    this->setComponent<MeshComponent>(ground, groundMesh);
    Transform& transform2 = this->getComponent<Transform>(ground);
    transform2.position = glm::vec3(0.0f, -10.0f, 0.0f);
    transform2.scale = glm::vec3(100.f, 0.1f, 100.f);

}

void NewAINetwork::start()
{
  std::string playerIDStr = "playerID";
  this->getScriptHandler()->getGlobal(this->player, playerIDStr);

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

    int cube = this->createEntity();
    int a = this->getResourceManager()->addMesh("vengine_assets/models/Cube.fbx");
    this->setComponent<MeshComponent>(cube, a);
    this->setComponent<Collider>(cube, Collider::createBox(glm::vec3(8,16,1)));
    auto& c = this->getComponent<Transform>(cube);
    c.scale.x*=8;
    c.scale.y*=16;
    
    c.position.z += 10.f;

    this->getNetworkHandler()->sendAIPolygons({glm::vec2{c.position.x+4,c.position.z+1.f},glm::vec2{c.position.x-4,c.position.z-1.f},glm::vec2{c.position.x+4,c.position.z-1.f},glm::vec2{ c.position.x-4,c.position.z+1.f},});
}


void NewAINetwork::update()
{

    this->getPhysicsEngine()->renderDebugShapes(true);
    
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
    Transform& t =  this->getComponent<Transform>(player);

    // std::cout << t.position.x << ", " << t.position.x << ", " << t.position.z << std::endl;
    this->getNetworkHandler()->sendUDPDataToClient(
        this->getComponent<Transform>(player).position,
        this->getComponent<Transform>(player).rotation
    );
}

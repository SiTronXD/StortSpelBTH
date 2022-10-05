#include "NetworkAI.h"

#include "../Systems/MovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"

NetworkAI::NetworkAI(): camEntity(-1), entity(-1) {}

NetworkAI::~NetworkAI() {}

void NetworkAI::init()
{
    this->camEntity = this->createEntity();
    this->setComponent<Camera>(this->camEntity, 1.0f);
    this->setMainCamera(this->camEntity);
    Transform& camTransform = this->getComponent<Transform>(this->camEntity);
    camTransform.position   = glm::vec3(1.0f);

    this->entity = this->createEntity();
    this->setComponent<MeshComponent>(this->entity);
    this->setComponent<Movement>(this->entity);
    this->createSystem<MovementSystem>(this, entity);
    this->setComponent<Combat>(this->entity);
    this->createSystem<CombatSystem>(this, entity);
    Transform& transform = this->getComponent<Transform>(this->entity);
    transform.position   = glm::vec3(0.0f, 0.0f, 20.0f);
    transform.rotation   = glm::vec3(-90.0f, 0.0f, 0.0f);
    transform.scale      = glm::vec3(5.0f);
}

#include "../ServerGameModes/TheServerGame.h"
void NetworkAI::update()
{
    Transform& transform = this->getComponent<Transform>(this->entity);
    this->getNetworkHandler()->sendUDPDataToClient(transform.position, transform.rotation);

    if (Input::isKeyPressed(Keys::H)) {
        this->getNetworkHandler()->createServer(new TheServerGame());
        this->getNetworkHandler()->createClient();
        this->getNetworkHandler()->connectClientToThis();
        Transform& transform = this->getComponent<Transform>(this->entity);
        transform.rotation   += glm::vec3(15, 0, 0);
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

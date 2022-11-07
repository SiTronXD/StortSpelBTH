#include "NetworkAI.h"

#include "../ServerGameModes/TheServerGame.h"
#include "../Systems/CameraMovementSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/MovementSystem.hpp"

NetworkAI::NetworkAI() : camEntity(-1), player(-1) {}

NetworkAI::~NetworkAI() {}

void NetworkAI::init()
{
  int groundMesh =
      this->getResourceManager()->addMesh("vengine_assets/models/cube.obj");

  //ground
  int ground = this->createEntity();
  this->setComponent<Collider>(
      ground, Collider::createBox(glm::vec3(100, 1, 100))
  );
  this->setComponent<MeshComponent>(ground, groundMesh);
  Transform& transform2 = this->getComponent<Transform>(ground);
  transform2.position = glm::vec3(0.0f, -10.0f, 0.0f);
  transform2.scale = glm::vec3(100.f, 0.1f, 100.f);
}
void NetworkAI::start()
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

  int cube = this->createEntity();
  int a = this->getResourceManager()->addMesh("vengine_assets/models/cube.obj");

  this->setComponent<MeshComponent>(cube, a);
  this->setComponent<Collider>(cube, Collider::createBox(glm::vec3(8, 16, 1)));
  auto& c = this->getComponent<Transform>(cube);
  c.scale.x *= 8;
  c.scale.y *= 16;

  c.position.z += 10.f;

  this->getNetworkHandler()->sendAIPolygons({
      glm::vec2{c.position.x + 4, c.position.z + 1.f},
      glm::vec2{c.position.x - 4, c.position.z - 1.f},
      glm::vec2{c.position.x + 4, c.position.z - 1.f},
      glm::vec2{c.position.x - 4, c.position.z + 1.f},
  });
}

void NetworkAI::update()
{
  if (Input::isKeyPressed(Keys::N))
    {
      this->getNetworkHandler()->sendTCPDataToClient(TCPPacketEvent{
          GameEvents::START});
    }
  if (Input::isKeyPressed(Keys::M))
    {
      this->getNetworkHandler()->createClient("Cli");
      std::cout << "ip : ";
      std::string ip;
      std::cin >> ip;
      if (ip == "a")
        {
          ip = "192.168.1.104";
        }
      this->getNetworkHandler()->connectClient(ip);
    }
}

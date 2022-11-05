#include "LevelEditor.h"

LevelEditor::LevelEditor() 
{
}

LevelEditor::~LevelEditor() 
{
  if (comlib)
    delete comlib;
  if (data)
    delete data;
  if (header)
    delete header;
}

void LevelEditor::init() 
{
  comlib = new Comlib(L"MayaBuffer", 150 * (1 << 20), Consumer);
  Entity floor = this->createEntity();
  this->setComponent<MeshComponent>(floor, 0);
  this->getComponent<Transform>(floor).scale = glm::vec3(100.0f, 1.0f, 100.0f);
  this->getComponent<Transform>(floor).position = glm::vec3(0.0f, -1.0f, 0.0f);
  this->setComponent<Collider>(floor, Collider::createBox((glm::vec3(100.0f, 1.0f, 100.0f))));
}

void LevelEditor::start()
{
  std::string playerName = "playerID";
  this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);
}

void LevelEditor::update() 
{
  while (comlib->Recieve(data, header))
  {
      if (header->messageType == MESH_NEW)
      {
          objectHandler.addNewMesh(this, this->getResourceManager(), data);
      }
      else if (header->messageType == MESH_REMOVE)
      {
          objectHandler.removeMesh(this, data);
      }
      else if (header->messageType == MESH_TRANSFORM)
      {
          objectHandler.updateMeshTransform(this, data); 
      }
      else if (header->messageType == TOPOLOGY_CHANGED)
      {
          objectHandler.updateMeshTopology(this, this->getResourceManager(), data); 
      }

  }
}
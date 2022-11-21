#include "LevelEditor.h"

LevelEditor::LevelEditor() : mayaCamera(-1), playerCamera(-1) {}

LevelEditor::~LevelEditor() 
{
  if (comlib)
    delete comlib;
  //if (data)
  //  delete[] data;
}

void LevelEditor::init() 
{
  comlib = new Comlib(L"MayaBuffer", 150 * (1 << 20), Consumer);

  roomHandler.init(
      this,
      this->getResourceManager(),
      this->getConfigValue<int>("room_size"),
      this->getConfigValue<int>("tile_types")
  );
  roomHandler.generate();

  Entity sun = this->createEntity();
  this->setComponent<DirectionalLight>(sun);
  DirectionalLight& light = this->getComponent<DirectionalLight>(sun);
  light.color = glm::vec3(1.f, 0.8f, 0.5f);
  light.direction = glm::normalize(glm::vec3(-1.f));

}

void LevelEditor::start()
{
  std::string playerName = "playerID";
  this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);
  playerCamera = this->getMainCameraID();
  mayaCamera = this->createEntity();
  this->setComponent<Camera>(this->mayaCamera);
  //this->setMainCamera(mayaCamera);
}

void LevelEditor::update() 
{
  readBuffer();
  if(Input::isKeyPressed(Keys::C))
  {
      if (this->getMainCameraID() == mayaCamera)
          this->setMainCamera(playerCamera);
      else
          this->setMainCamera(mayaCamera);
  }
}

void LevelEditor::readBuffer() 
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
          objectHandler.updateMeshTopology(
              this, this->getResourceManager(), data
          );
        }
     else if (header->messageType == MATERIAL_DATA)
       {
         objectHandler.addOrUpdateMaterial(
             this, this->getResourceManager(), data
         );
       }
     else if (header->messageType == MESH_MATERIAL_CONNECTION)
       {
         objectHandler.setMeshMaterial(this, this->getResourceManager(), data);
       }
     else if (header->messageType == CAMERA_UPDATE)
       {
         CameraUpdateHeader camera;
         memcpy(&camera, data, sizeof(CameraUpdateHeader));
     
         Transform& t = this->getComponent<Transform>(mayaCamera);
         t.position = glm::vec3(camera.position[0], camera.position[1], camera.position[2]);
         t.rotation = glm::vec3(camera.rotation[0], camera.rotation[1], camera.rotation[2]);
         t.updateMatrix();
         //Camera& c = this->getComponent<Camera>(mayaCamera);
         //c.updateMatrices(t);
     }
      if (data)
          delete[] data;
    }
}
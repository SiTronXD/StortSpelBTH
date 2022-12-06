#include "LevelEditor.h"

LevelEditor::LevelEditor() : mayaCamera(-1), playerCamera(-1) {}

LevelEditor::~LevelEditor() 
{
  if (comlib)
    delete comlib;
}

void LevelEditor::init() 
{
  comlib = new Comlib(L"MayaBuffer", 150 * (1 << 20), Consumer);

  roomHandler.init(
      this,
      this->getResourceManager(),
      this->getPhysicsEngine(),
      true
  );
  roomHandler.generate(rand());

  sun = this->createEntity();
  this->setComponent<DirectionalLight>(sun);
  DirectionalLight& light = this->getComponent<DirectionalLight>(sun);
  light.color = glm::vec3(1.f, 0.8f, 0.5f);
  light.direction = glm::normalize(glm::vec3(-1.f));
  light.shadowMapAngleBias = 0.04f;
}

void LevelEditor::start()
{
  std::string playerName = "playerID";
  this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);
  playerCamera = this->getMainCameraID();
  mayaCamera = this->createEntity();
  this->setComponent<Camera>(this->mayaCamera);
}

void LevelEditor::update() 
{
#ifdef _CONSOLE
    DirectionalLight& light = this->getComponent<DirectionalLight>(sun);
    ImGui::Begin("Shadows");
    ImGui::SliderFloat("Size 0", &light.cascadeSizes[0], 0.0f, 1.0f);
    ImGui::SliderFloat("Size 1", &light.cascadeSizes[1], 0.0f, 1.0f);
    ImGui::SliderFloat("Size 2", &light.cascadeSizes[2], 0.0f, 1.0f);
    ImGui::SliderFloat("Depth", &light.cascadeDepthScale, 0.0f, 50.0f);
    ImGui::Checkbox("Visualize cascades", &light.cascadeVisualization);
    ImGui::SliderFloat("Angle bias", &light.shadowMapAngleBias, 0.0f, 0.1f);
    ImGui::End();
#endif

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
     }
      if (data)
          delete[] data;
    }
}
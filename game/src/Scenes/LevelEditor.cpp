#include "LevelEditor.h"
#include "MainMenu.h"
#include "../Settings/Settings.h"

LevelEditor::LevelEditor() : mayaCamera(-1), playerCamera(-1) 
{
    Input::setHideCursor(true);
}

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
      this->getResourceManager(), this->getPhysicsEngine(),
      true
  );
  roomHandler.generate(rand(), 2u);

  sun = this->createEntity();
  this->setComponent<DirectionalLight>(sun);
  DirectionalLight& light = this->getComponent<DirectionalLight>(sun);
  light.color = glm::vec3(1.f, 0.8f, 0.5f);
  light.direction = glm::normalize(glm::vec3(-1.f));
  light.shadowMapAngleBias = 0.04f;

  TextureSamplerSettings samplerSettings{};
  samplerSettings.filterMode = vk::Filter::eNearest;
  samplerSettings.unnormalizedCoordinates = VK_TRUE;

  Settings::sceneHandler = this->getSceneHandler();
  this->settingsEntity = this->createEntity();
  this->setScriptComponent(this->settingsEntity, "scripts/settings.lua");
  uint32_t texID = this->getResourceManager()->addTexture("assets/textures/UI/howToUse.png");
  this->getScriptHandler()->setScriptComponentValue(this->getComponent<Script>(settingsEntity), (int)texID, "howToPlayTex");
  Settings::setEntity(this->settingsEntity);
  this->setInactive(this->settingsEntity);
  
	this->pauseBackgroundId = this->getResourceManager()->addTexture("assets/textures/UI/frame.png");
	this->howToUseBackgroundId = this->getResourceManager()->addTexture("assets/textures/UI/howToUse.png");
    this->settingsBackgroundId = this->getResourceManager()->addTexture("assets/textures/UI/settings.png");
	this->buttonTexture = this->getResourceManager()->addTexture("assets/textures/UI/button.jpg");
   
    this->fontTextureId = Scene::getResourceManager()->addTexture("assets/textures/UI/font.png", { samplerSettings, true });
	Scene::getUIRenderer()->setBitmapFont(
		{
			"abcdefghij",
			"klmnopqrst",
			"uvwxyz+-.'",
			"0123456789",
			"!?,<>:()#^",
			"@%        "
		},
		fontTextureId,
		glm::uvec2(50, 50)
	);
}

void LevelEditor::start()
{
  std::string playerName = "playerID";
  this->getSceneHandler()->getScriptHandler()->getGlobal(playerID, playerName);
  playerCamera = this->getMainCameraID();
  mayaCamera = this->createEntity();
  this->setComponent<Camera>(this->mayaCamera);

  Input::setHideCursor(false);

  // Pause menu
  this->resumeButton.position = glm::vec2(0.0f, 225.0f);
  this->settingsButton.position = glm::vec2(0.0f, 75.0f);
  this->howToPlayButton.position = glm::vec2(0.0f, -75.0f);
  this->exitButton.position = glm::vec2(0.0f, -255.0f);
  this->backButton.position = glm::vec2(-745.0f, -360.0f);

  this->resumeButton.dimension = glm::vec2(500.0f, 100.0f);
  this->settingsButton.dimension = glm::vec2(500.0f, 100.0f);
  this->howToPlayButton.dimension = glm::vec2(500.0f, 100.0f);
  this->exitButton.dimension = glm::vec2(500.0f, 100.0f);
  this->backButton.dimension = glm::vec2(190.0f, 65.0f);
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
    
    // Paused
    if (Input::isKeyPressed(Keys::ESC))
    {
        this->paused = !this->paused;
        this->getScriptHandler()->setGlobal(this->paused, "paused");
        this->getScriptHandler()->setGlobal(false, "settings");
        this->getComponent<Rigidbody>(this->playerID).velocity = glm::vec3(0.0f);
        this->setInactive(this->settingsEntity);
    }
    if (this->paused)
    {
        if (this->isActive(this->settingsEntity))
        {
            Settings::updateValues();
            if (this->backButton.isClicking())
            {
                this->getScriptHandler()->setGlobal(false, "settings");
                this->getScriptHandler()->setGlobal(false, "howToPlay");
                this->setInactive(this->settingsEntity);
            }
        }
        else
        {
            if (this->resumeButton.isClicking())
            {
                this->paused = false;
                this->getScriptHandler()->setGlobal(this->paused, "paused");
                this->getScriptHandler()->setGlobal(false, "settings");
                this->getScriptHandler()->setGlobal(false, "howToPlay");
                Input::setHideCursor(!this->paused);
            }
            else if (this->settingsButton.isClicking())
            {
                this->getScriptHandler()->setGlobal(true, "settings");
                this->getScriptHandler()->setGlobal(false, "howToPlay");
                this->setActive(this->settingsEntity);
            }
            if (this->howToPlayButton.isClicking())
            {
                this->getScriptHandler()->setGlobal(true, "settings");
                this->getScriptHandler()->setGlobal(true, "howToPlay");
                this->setActive(this->settingsEntity);
            }
            else if (this->exitButton.isClicking())
            {
                this->switchScene(new MainMenu(), "scripts/MainMenu.lua");
            }
        }
    }

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
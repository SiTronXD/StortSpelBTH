
-- Camera
local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
scene.setMainCamera(cam)

-- Tutorial logic
local tutorialID = scene.createEntity()
scene.setComponent(tutorialID, CompType.Script, "scripts/Tutorial.lua")
scene.getComponent(tutorialID, CompType.Script).tutorialTextureID = 
	resources.addTexture("assets/textures/UI/tutorial.png")
scene.getComponent(tutorialID, CompType.Script).loadingTextureID = 
	resources.addTexture("assets/textures/UI/loading.png")


-- Camera
local cam = scene.createPrefab("scripts/prefabs/CameraPrefab.lua")
scene.setMainCamera(cam)

-- Game over logic
local gameOverID = scene.createEntity()
scene.setComponent(tutorialID, CompType.Script, "scripts/GameOver.lua")
scene.getComponent(tutorialID, CompType.Script).gameOverTextureID = 
	resources.addTexture("assets/textures/UI/gameOver.png")
scene.getComponent(tutorialID, CompType.Script).loadingTextureID = 
	resources.addTexture("assets/textures/UI/loading.png")
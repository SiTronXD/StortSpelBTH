-- Game over logic
local gameOverID = scene.createEntity()
scene.setComponent(gameOverID, CompType.Script, "scripts/GameOver.lua")
scene.getComponent(gameOverID, CompType.Script).gameOverTextureID = 
	resources.addTexture("assets/textures/UI/GameOver.png")
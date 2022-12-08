-- Game over logic
local gameOverID = scene.createEntity()
scene.setComponent(gameOverID, CompType.Script, "scripts/GameOver.lua")
scene.getComponent(gameOverID, CompType.Script).gameOverTextureID = 
	resources.addTexture("assets/textures/UI/GameOver.png")
scene.getComponent(gameOverID, CompType.Script).loadingTextureID = 
	resources.addTexture("assets/textures/UI/loading.png")
scene.getComponent(gameOverID, CompType.Script).qrCodeID =
	resources.addTexture("assets/textures/UI/Presumed Dead QR.png")
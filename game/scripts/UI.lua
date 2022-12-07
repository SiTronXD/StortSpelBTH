local script = {}

function script:init()
	scene.setComponent(self.ID, CompType.UIArea)
	self.resumeButton = scene.getComponent(self.ID, CompType.UIArea)
	self.resumeButton.position = vector(0, 100)
	self.resumeButton.dimension = vector(500, 100)
	self.exitButton = scene.getComponent(self.ID, CompType.UIArea)
	self.exitButton.position = vector(0, -100)
	self.exitButton.dimension = vector(500, 100)
	scene.removeComponent(self.ID, CompType.UIArea)
end

function script:update(dt)

	local xPos = -648
	local yPos = -465
	local xSize = 960 * 0.35
	local ySize = 40 * 0.35

	-- Clamped percentage for HP
	local staminaPercent = self.playerScript.currentStamina * 0.01
	local staminaMaxPercent = self.playerScript.maxStamina * 0.01
	local healthMaxPercent = self.playerScript.maxHealth * 0.01

    uiRenderer.setTexture(self.staminaBarBackgroundTxtID)
	uiRenderer.renderTexture(vector(xPos - (1.0 - staminaMaxPercent) * xSize * 0.5, yPos - 15), 
	vector((xSize * staminaMaxPercent) + 10, ySize + 10))

    uiRenderer.setTexture(self.staminaBarTxtID)
	uiRenderer.renderTexture(vector(xPos - (1.0 - staminaPercent) * xSize * 0.5, yPos - 15), 
	vector(xSize * staminaPercent, ySize))

	-- Health UI Border Bar
	local borderHpXPos = -600.0
	local borderHpYPos = -452.0
	local borderHpXSize = 764 * 0.56
	local borderHpYSize = 104 * 0.34

	uiRenderer.setTexture(self.uiHpMidTexID)
	uiRenderer.renderTexture(vector(borderHpXPos - (1.0 - healthMaxPercent) * borderHpXSize * 0.5, borderHpYPos), 
		vector(borderHpXSize * healthMaxPercent, borderHpYSize))

	uiRenderer.setTexture(self.uiHpSideTexID)
	uiRenderer.renderTexture(vector(borderHpXPos - (1.0 - healthMaxPercent) * borderHpXSize + 211, borderHpYPos), 
		vector(10.0, borderHpYSize))

	-- Stamina UI Border Bar
	local borderStamXPos = -648.0
	local borderStamYPos = -480.0
	local borderStamXSize = 764 * 0.45
	local borderStamYSize = 104 * 0.25

	uiRenderer.setTexture(self.uiStamMidTexID)
	uiRenderer.renderTexture(vector(borderStamXPos - (1.0 - staminaMaxPercent) * borderStamXSize * 0.5, borderStamYPos), 
		vector(borderStamXSize * staminaMaxPercent, borderStamYSize))

	uiRenderer.setTexture(self.uiStamSideTexID)
	uiRenderer.renderTexture(vector(borderStamXPos - (1.0 - staminaMaxPercent) * borderStamXSize + 169, borderStamYPos), 
		vector(10.0, borderStamYSize))

	-- Perks and Ability UI Border
	uiRenderer.setTexture(self.uiBorderTexID)
	uiRenderer.renderTexture(vector(0.0, 0.0), vector(1920.0, 1080.0))

	
	if paused then
		uiRenderer.setTexture(self.pauseBackgroundTexID)
		uiRenderer.renderTexture(vector(0.0, 0.0), vector(1920.0, 1080.0))
		uiRenderer.renderString("game paused", vector(0.0, 400), vector(80, 80));
		uiRenderer.setTexture(self.buttonTexID)
		uiRenderer.renderTexture(self.resumeButton.position, self.resumeButton.dimension, vector.fill(1), 0.85 + core.btoi(self.resumeButton:isHovering()) * 0.15)
		uiRenderer.renderTexture(self.exitButton.position, self.exitButton.dimension, vector.fill(1), 0.85 + core.btoi(self.exitButton:isHovering()) * 0.15)
		uiRenderer.renderString("resume", self.resumeButton.position, vector(50, 50))
		uiRenderer.renderString("exit", self.exitButton.position, vector(50, 50))
	end
end

return script
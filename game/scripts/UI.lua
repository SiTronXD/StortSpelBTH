local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	self.timer = 0.0
end

function script:update(dt)

	self.timer = self.timer + dt

	local xPos = -648
	local yPos = -465
	local xSize = 960 * 0.35
	local ySize = 40 * 0.35

	local resumePosX = 0.0
	local resumePosY = 100.0
	local resumeWidth = 500.0
	local resumeHeigth = 100.0

	local exitPosX = 0.0
	local exitPosY = -100.0
	local exitWidth = 500.0
	local exitHeigth = 100.0


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

	
	-- if paused then
	-- 	uiRenderer.setTexture(self.pauseBackgroundTexID)
	-- 	uiRenderer.renderTexture(vector(0.0, 0.0), vector(1920.0, 1080.0))
	-- 	uiRenderer.renderString("game paused", vector(0.0, 400), vector(80, 80));
	-- 	uiRenderer.setTexture(self.buttonTexID)
	-- 	uiRenderer.renderTexture(vector(resumePosX, resumePosY), vector(resumeWidth, resumeHeigth))
	-- 	uiRenderer.renderTexture(vector(exitPosX, exitPosY), vector(exitWidth, exitHeigth))
	-- 	uiRenderer.renderString("resume", vector(resumePosX, resumePosY), vector(50, 50));
	-- 	uiRenderer.renderString("exit", vector(exitPosX, exitPosY), vector(50, 50));
	-- end
end

return script
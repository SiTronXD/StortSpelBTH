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

	-- Clamped percentage for HP
	local staminaPercent = self.playerScript.currentStamina * 0.01
	--staminaPercent = math.min(math.max(staminaPercent, 0.0), 1.0)
	local staminaMaxPercent = self.playerScript.maxStamina * 0.01
	--staminaMaxPercent = self.playerScript.maxStamina * 0.01

    uiRenderer.setTexture(self.staminaBarBackgroundTxtID)
	uiRenderer.renderTexture(vector(xPos - (1.0 - staminaMaxPercent) * xSize * 0.5, yPos - 15), 
	vector((xSize * staminaMaxPercent) + 10, ySize + 10))

    uiRenderer.setTexture(self.staminaBarTxtID)
	uiRenderer.renderTexture(vector(xPos - (1.0 - staminaPercent) * xSize * 0.5, yPos - 15), 
	vector(xSize * staminaPercent, ySize))

	uiRenderer.setTexture(self.uiTextureID)
	uiRenderer.renderTexture(vector(0.0, 0.0), vector(1920.0, 1080.0))

	-- Perk slots
	--uiRenderer.setTexture(self.perkSlotTextureID)
	--for i = 1, 3 
	--do
	--	uiRenderer.renderTexture(-xPos - 70 + i * 80, yPos + 10, 70, 70)
	--end

	-- Crosshair
	uiRenderer.setTexture(self.crosshairTextureID)
	uiRenderer.renderTexture(vector(0, 0), vector.fill(30))
end

return script
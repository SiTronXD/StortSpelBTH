local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	self.timer = 0.0
end

function script:update(dt)

	self.timer = self.timer + dt

	local xPos = -720
	local yPos = -500
	local xSize = 1024 * 0.35
	local ySize = 64 * 0.35

	-- Clamped percentage for HP
	local hpPercent = self.playerScript.currentHP * 0.01
	hpPercent = math.min(math.max(hpPercent, 0.0), 1.0)

	-- Black HP background
    uiRenderer.setTexture(self.hpBarBackgroundTextureID)
	uiRenderer.renderTexture(xPos, yPos, xSize + 10, ySize + 10)

	-- Red HP bar
    uiRenderer.setTexture(self.hpBarTextureID)
	uiRenderer.renderTexture(xPos - (1.0 - hpPercent) * xSize * 0.5, yPos, xSize * hpPercent, ySize)

	-- Perk slots
	uiRenderer.setTexture(self.perkSlotTextureID)
	for i = 1, 3 
	do
		uiRenderer.renderTexture(-xPos - 70 + i * 80, yPos + 10, 70, 70)
	end

	-- Crosshair
	uiRenderer.setTexture(self.crosshairTextureID)
	uiRenderer.renderTexture(0, 0, 30, 30)
end

return script
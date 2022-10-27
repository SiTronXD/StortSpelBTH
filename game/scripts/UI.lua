local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	
end

function script:update(dt)

	local xPos = -720
	local yPos = -500
	local xSize = 1024 * 0.35
	local ySize = 64 * 0.35

	local hpPercent = 0.8

	-- Black HP background
    uiRenderer.setTexture(self.hpBarBackgroundTextureID)
	uiRenderer.renderTexture(xPos, yPos, xSize + 10, ySize + 10)

	-- Red HP bar
    uiRenderer.setTexture(self.hpBarTextureID)
	uiRenderer.renderTexture(xPos - (1.0 - hpPercent) * xSize * 0.5, yPos, xSize * hpPercent, ySize)
end

return script
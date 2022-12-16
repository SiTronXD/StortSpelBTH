local script = {}

function script:init()
	
end

function script:update(dt)
	-- UI
	local xSize = 1858 * 0.8
	local ySize = 296 * 0.8
	uiRenderer.setTexture(self.gameOverTextureID)
	uiRenderer.renderTexture(vector(0, 350), vector(xSize, ySize))

	uiRenderer.renderString("press space to continue", vector(0, -400), vector(50, 50))
end

return script
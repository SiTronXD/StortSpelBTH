local script = {}

function script:init()
	self.shouldLoadNextScene = false
end

function script:update(dt)
	if input.isKeyPressed(Keys.SPACE)
	then
		self.shouldLoadNextScene = true
	end

	-- UI
	if not self.shouldLoadNextScene
	then
		local xSize = 1858 * 0.8
		local ySize = 296 * 0.8
		uiRenderer.setTexture(self.gameOverTextureID)
		uiRenderer.renderTexture(vector(0, 350), vector(xSize, ySize))

		uiRenderer.renderString("press space to continue", vector(0, -400), vector(50, 50))
	else
		uiRenderer.setTexture(self.loadingTextureID)
		uiRenderer.renderTexture(vector(0, 0), vector(1920, 1080))
	end
end

return script
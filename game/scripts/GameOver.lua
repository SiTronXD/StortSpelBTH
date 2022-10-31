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
		uiRenderer.setTexture(self.gameOverTextureID)
		uiRenderer.renderTexture(0, 0, 1920, 1080)
	else
		uiRenderer.setTexture(self.loadingTextureID)
		uiRenderer.renderTexture(0, 0, 1920, 1080)
	end
end

return script
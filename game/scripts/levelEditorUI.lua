local script = {}

function script:init()
	scene.setComponent(self.ID, CompType.UIArea, { position = vector(), dimension = vector() })
	self.resumeButton = scene.getComponent(self.ID, CompType.UIArea)
	self.resumeButton.position = vector(0, 225)
	self.resumeButton.dimension = vector(500, 100)
	self.settingsButton = scene.getComponent(self.ID, CompType.UIArea)
	self.settingsButton.position = vector(0, 75)
	self.settingsButton.dimension = vector(500, 100)
	self.howToPlayButton = scene.getComponent(self.ID, CompType.UIArea)
	self.howToPlayButton.position = vector(0, -75)
	self.howToPlayButton.dimension = vector(500, 100)
	self.exitButton = scene.getComponent(self.ID, CompType.UIArea)
	self.exitButton.position = vector(0, -225)
	self.exitButton.dimension = vector(500, 100)
	scene.removeComponent(self.ID, CompType.UIArea)
	
	self.pauseBackgroundTexID = resources.addTexture("assets/textures/UI/frame.png")
	self.buttonTexID = resources.addTexture("assets/textures/UI/button.jpg")
end

function script:update(dt)
	if paused then
		if not howToPlay then
			uiRenderer.setTexture(self.pauseBackgroundTexID)
			uiRenderer.renderTexture(vector(0.0, 0.0), vector(1920.0, 1080.0))
		end
		if not settings then
			uiRenderer.renderString("paused", vector(0.0, 400), vector(80, 80))
			uiRenderer.setTexture(self.buttonTexID)
			uiRenderer.renderTexture(self.resumeButton.position, self.resumeButton.dimension, vector.fill(1), 0.85 + core.btoi(self.resumeButton:isHovering()) * 0.15)
			uiRenderer.renderTexture(self.settingsButton.position, self.settingsButton.dimension, vector.fill(1), 0.85 + core.btoi(self.settingsButton:isHovering()) * 0.15)
			uiRenderer.renderTexture(self.howToPlayButton.position, self.howToPlayButton.dimension, vector.fill(1), 0.85 + core.btoi(self.howToPlayButton:isHovering()) * 0.15)
			uiRenderer.renderTexture(self.exitButton.position, self.exitButton.dimension, vector.fill(1), 0.85 + core.btoi(self.exitButton:isHovering()) * 0.15)
			uiRenderer.renderString("resume", self.resumeButton.position, vector(50, 50))
			uiRenderer.renderString("settings", self.settingsButton.position, vector(50, 50))
			uiRenderer.renderString("how to play", self.howToPlayButton.position, vector(50, 50))
			uiRenderer.renderString("exit", self.exitButton.position, vector(50, 50))
		elseif not howToPlay then
			uiRenderer.renderString("settings", vector(0.0, 400), vector(80, 80))
		end
	end
end

return script
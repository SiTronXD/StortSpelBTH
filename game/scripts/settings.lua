local script = {}

function script:init()
	self.volume = 0.5
	self.musicVolume = 0.25
	self.sensitivity = 25.0
	self.cameraFOV = 90.0
	self.fullScreen = false

	self.volumeLimits = vector(0, 1)
	self.musicVolumeLimits = vector(0, 1)
	self.sensitivityLimits = vector(10, 50)
	self.FOVLimits = vector(60, 120)

	self.buttonTex = resources.addTexture("assets/textures/UI/button.png")
	self.barMid = resources.addTexture("assets/textures/UI/UIBarMid.png")
	self.barSide = resources.addTexture("assets/textures/UI/UIBarSide.png")
	self.barFill = resources.addTexture("assets/textures/UI/UIBarFill.png")
	self.howToPlayTex = resources.addTexture("assets/textures/UI/howToPlay.png")
	self.buttonSound = resources.addSound("assets/Sounds/buttonClick.ogg")

	scene.setComponent(self.ID, CompType.UIArea, { position = vector(), dimension = vector() })

	self.volumeArea = scene.getComponent(self.ID, CompType.UIArea)
	self.volumeArea.position = vector(0, 200)
	self.volumeArea.dimension = vector(800, 32)

	self.musicVolumeArea = scene.getComponent(self.ID, CompType.UIArea)
	self.musicVolumeArea.position = vector(0, 75)
	self.musicVolumeArea.dimension = vector(800, 32)

	self.sensitivityArea = scene.getComponent(self.ID, CompType.UIArea)
	self.sensitivityArea.position = vector(0, -50)
	self.sensitivityArea.dimension = vector(800, 32)

	self.FOVArea = scene.getComponent(self.ID, CompType.UIArea)
	self.FOVArea.position = vector(0, -175)
	self.FOVArea.dimension = vector(800, 32)

	self.backButton = scene.getComponent(self.ID, CompType.UIArea)
	self.backButton.position = vector(-745, -360)
	self.backButton.dimension = vector(190, 65)

	self.fullScreenButton = scene.getComponent(self.ID, CompType.UIArea)
	self.fullScreenButton.position = vector(0, -325)
	self.fullScreenButton.dimension = vector(75, 75)

	scene.removeComponent(self.ID, CompType.UIArea)

	self.selected = {}
end

function script:update()

	if(howToPlay) then
		uiRenderer.setTexture(self.howToPlayTex)
		uiRenderer.renderTexture(vector(0, 0), vector(1920, 1080))

		uiRenderer.setTexture(self.buttonTex)
		uiRenderer.renderTexture(self.backButton.position, self.backButton.dimension, vector.fill(1), 0.85 + core.btoi(self.backButton:isHovering()) * 0.15)
		uiRenderer.renderString("back", self.backButton.position + vector(0, 5), vector.fill(40))
		return
	end

	local barLength = 800
	local barLengthHalf = barLength / 2

	-- Select slider
	if(self.volumeArea:isClicking()) then
		scene.playSound(self.buttonSound)
		self.selected = self.volumeArea
	elseif(self.musicVolumeArea:isClicking()) then
		scene.playSound(self.buttonSound)
		self.selected = self.musicVolumeArea
	elseif(self.sensitivityArea:isClicking()) then
		scene.playSound(self.buttonSound)
		self.selected = self.sensitivityArea
	elseif(self.FOVArea:isClicking()) then
		scene.playSound(self.buttonSound)
		self.selected = self.FOVArea
	elseif (input.isMouseButtonReleased(Mouse.LEFT)) then
		self.selected = {}
	end

	-- Set value if selected
	if(self.selected == self.volumeArea) then
		self.volume = math.max(
			self.volumeLimits.x,
			math.min(
				self.volumeLimits.y,
				(self.volumeLimits.y - self.volumeLimits.x) * (input.getMousePosition().x + barLengthHalf - self.volumeArea.position.x) / barLength + self.volumeLimits.x
			)
		)
	elseif(self.selected == self.musicVolumeArea) then
		self.musicVolume = math.max(
			self.musicVolumeLimits.x,
			math.min(
				self.musicVolumeLimits.y,
				(self.musicVolumeLimits.y - self.musicVolumeLimits.x) * (input.getMousePosition().x + barLengthHalf - self.musicVolumeArea.position.x) / barLength + self.musicVolumeLimits.x
			)
		)
	elseif(self.selected == self.sensitivityArea) then
		self.sensitivity = math.max(
			self.sensitivityLimits.x,
			math.min(
				self.sensitivityLimits.y,
				(self.sensitivityLimits.y - self.sensitivityLimits.x) * (input.getMousePosition().x + barLengthHalf - self.sensitivityArea.position.x) / barLength + self.sensitivityLimits.x
			)
		)
	elseif(self.selected == self.FOVArea) then
		self.cameraFOV = math.max(
			self.FOVLimits.x,
			math.min(
				self.FOVLimits.y,
				(self.FOVLimits.y - self.FOVLimits.x) * (input.getMousePosition().x + barLengthHalf - self.FOVArea.position.x) / barLength + self.FOVLimits.x
			)
		)
	end

	-- Set fullscreen bool
	if(self.fullScreenButton:isClicking()) then
		scene.playSound(self.buttonSound)
		self.fullScreen = not self.fullScreen
	end


	----- Render UI -----

	-- Buttons
	uiRenderer.setTexture(self.buttonTex)
	uiRenderer.renderTexture(self.backButton.position, self.backButton.dimension, vector.fill(1), 0.85 + core.btoi(self.backButton:isHovering()) * 0.15)
	uiRenderer.renderTexture(self.fullScreenButton.position, self.fullScreenButton.dimension, vector.fill(1), 0.85 + core.btoi(self.fullScreenButton:isHovering()) * 0.15)
	uiRenderer.renderString("back", self.backButton.position + vector(0, 5), vector.fill(40))

	-- Bar fill
	uiRenderer.setTexture(self.barFill)
	self:renderSlider(barLength, barLengthHalf, self.volume, self.volumeLimits, self.volumeArea)
	self:renderSlider(barLength, barLengthHalf, self.musicVolume, self.musicVolumeLimits, self.musicVolumeArea)
	self:renderSlider(barLength, barLengthHalf, self.sensitivity, self.sensitivityLimits, self.sensitivityArea)
	self:renderSlider(barLength, barLengthHalf, self.cameraFOV, self.FOVLimits, self.FOVArea)

	-- Bar mid
	uiRenderer.setTexture(self.barMid)
	uiRenderer.renderTexture(self.volumeArea.position, vector(barLength, 45))
	uiRenderer.renderTexture(self.musicVolumeArea.position, vector(barLength, 45))
	uiRenderer.renderTexture(self.sensitivityArea.position, vector(barLength, 45))
	uiRenderer.renderTexture(self.FOVArea.position, vector(barLength, 45))
	uiRenderer.renderTexture(self.fullScreenButton.position, vector(55, 75))

	-- Bar side
	uiRenderer.setTexture(self.barSide)
	uiRenderer.renderTexture(self.volumeArea.position + vector(barLengthHalf, 0), vector(10, 45))
	uiRenderer.renderTexture(self.volumeArea.position - vector(barLengthHalf, 0), vector(-10, 45))
	uiRenderer.renderTexture(self.musicVolumeArea.position + vector(barLengthHalf, 0), vector(10, 45))
	uiRenderer.renderTexture(self.musicVolumeArea.position - vector(barLengthHalf, 0), vector(-10, 45))
	uiRenderer.renderTexture(self.sensitivityArea.position + vector(barLengthHalf, 0), vector(10, 45))
	uiRenderer.renderTexture(self.sensitivityArea.position - vector(barLengthHalf, 0), vector(-10, 45))
	uiRenderer.renderTexture(self.FOVArea.position + vector(barLengthHalf, 0), vector(10, 45))
	uiRenderer.renderTexture(self.FOVArea.position - vector(barLengthHalf, 0), vector(-10, 45))
	uiRenderer.renderTexture(self.fullScreenButton.position + vector(32.5, 0), vector(15, 75))
	uiRenderer.renderTexture(self.fullScreenButton.position - vector(32.5, 0), vector(-15, 75))

	-- Text
	uiRenderer.renderString("master volume", self.volumeArea.position + vector(0, 50), vector.fill(35))
	uiRenderer.renderString(string.format("%.1f", self.volume * 100), self.volumeArea.position + vector(0, 5), vector.fill(30))
	uiRenderer.renderString("music volume", self.musicVolumeArea.position + vector(0, 50), vector.fill(35))
	uiRenderer.renderString(string.format("%.1f", self.musicVolume * 100), self.musicVolumeArea.position + vector(0, 5), vector.fill(30))
	uiRenderer.renderString("sensitivity", self.sensitivityArea.position + vector(0, 50), vector.fill(35))
	uiRenderer.renderString(string.format("%.1f", self.sensitivity), self.sensitivityArea.position + vector(0, 5), vector.fill(30))
	uiRenderer.renderString("camera field of view", self.FOVArea.position + vector(0, 50), vector.fill(35))
	uiRenderer.renderString(string.format("%.1f", self.cameraFOV), self.FOVArea.position + vector(0, 5), vector.fill(30))
	uiRenderer.renderString("fullscreen", self.fullScreenButton.position + vector(0, 75), vector.fill(35))

	if(self.fullScreen) then
		uiRenderer.renderString("x", self.fullScreenButton.position + vector(0, 5), vector.fill(45))
	end

end

function script:renderSlider(barLength, barLengthHalf, val, limits, area)
	uiRenderer.renderTexture(area.position + vector(barLengthHalf * ((val - limits.x) / (limits.y - limits.x) - 1), 0), 
							 vector(barLength * (val - limits.x) / (limits.y - limits.x), area.dimension.y), 
							 vector(1.1, 1, 1) + vector.fill(0.2) * core.btoi(area:isHovering() or self.selected == area))
end

return script
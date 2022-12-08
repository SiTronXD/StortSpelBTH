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
		local qrXSize = 341;
		local qrYSize = 272;
		local qrTextSize = 30;
		uiRenderer.setTexture(self.qrCodeID);
		uiRenderer.renderTexture(vector(730, 350), vector(qrXSize, qrYSize))
		uiRenderer.renderString("scan this cute fellow", vector(720, 200), vector(qrTextSize, qrTextSize))
		uiRenderer.renderString("to give feedback", vector(720, 150), vector(qrTextSize, qrTextSize))
	else
		uiRenderer.setTexture(self.loadingTextureID)
		uiRenderer.renderTexture(vector(0, 0), vector(1920, 1080))
	end
end

return script
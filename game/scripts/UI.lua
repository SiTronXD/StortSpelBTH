local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	
end

function script:update(dt)
    uiRenderer.setTexture(0)
	uiRenderer.renderTexture(-300, 0, 100, 100)
end

return script
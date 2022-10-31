local script = {}

function script:init(dt) 
	self.speed = 7.5
	self.transform.position.y = -25
	self.remove = false
end

function script:update(dt)
	self.transform.position.y = self.transform.position.y + self.speed * dt
	if(self.transform.position.y >= 0.0) then
		scene.removeComponent(self.ID, CompType.Script)
	end
end

return script
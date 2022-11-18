local script = {}

function script:init(dt) 
	self.speed = 10.0
	self.maxY = 21.0
end

function script:update(dt)
	self.transform.position = self.transform.position + self.transform:up() * self.speed * dt
	print(self.transform.position.y)
	if (self.transform.position.y >= self.maxY) then
		scene.removeComponent(self.ID, CompType.Script)
	end
end

return script
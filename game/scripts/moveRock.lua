local script = {}

function script:init(dt) 
	self.speed = 13.0
	self.maxY = 21.0
	print("Init")
end

function script:update(dt)
	self.transform.position.x = self.transform.position.x + (math.random() * 0.2 - 0.1)
	self.transform.position.z = self.transform.position.z + (math.random() * 0.2 - 0.1)

	self.transform.position = self.transform.position + self.transform:up() * self.speed * dt

	if (self.transform.position.y >= self.maxY) then
		scene.removeComponent(self.ID, CompType.Script)
	end
end

return script
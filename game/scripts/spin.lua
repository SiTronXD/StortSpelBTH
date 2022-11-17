local script = {}
script.timer = 0

function script:update(dt)
	self.timer = self.timer + dt
	self.transform.rotation.y = self.transform.rotation.y + 60 * dt
	self.transform.position.y = self.transform.position.y + 0.06 * math.sin(self.timer)
end

return script
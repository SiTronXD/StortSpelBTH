local script = {}
script.timer = 0
script.floatValue = 7.0
script.sinMulti = 2.0

function script:update(dt)
	self.timer = self.timer + dt
	self.transform.rotation.y = self.transform.rotation.y + 60 * dt
	self.transform.position.y = self.floatValue + self.sinMulti * math.sin(self.timer)
end

return script
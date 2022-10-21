local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	self.moveDir = vector()
	self.currentSpeed = vector()
	self.maxSpeed = 50
	self.speedIncrease = 200
	self.turnSpeed = 200
	self.timer = 0
    self.slowDown = 80
    self.transform.position = vector(0, -7, 0)
    self.transform.rotation = vector(-90, 0, 0)
end

function script:update(dt)
    self:move2(dt)
    self:rotate2(dt)
end

function script:move(deltaTime)
    self.currentSpeed.x = 0
    self.moveDir.y = (core.btoi(input.isKeyDown(Keys.W))) - (core.btoi(input.isKeyDown(Keys.S)))

    if (self.currentSpeed.y > self.maxSpeed) 
    then
        self.currentSpeed.y = self.maxSpeed
    end

    if (self.moveDir.y ~= 0) 
    then
        self.currentSpeed.y = self.currentSpeed.y + self.speedIncrease * self.moveDir.y * deltaTime
    end

    if(self.moveDir.y == 0 and self.currentSpeed ~= 0)
    then
        if (self.currentSpeed.y < 0.001 and self.currentSpeed.y > -0.001)
        then
            self.currentSpeed.y = 0
        elseif (self.currentSpeed.y > 0) 
        then
            self.currentSpeed.y = self.currentSpeed.y - self.slowDown * deltaTime
        elseif (self.currentSpeed.y < 0) 
        then
            self.currentSpeed.y = self.currentSpeed.y + self.slowDown * deltaTime
        end
    end

    if (self.currentSpeed.y > self.maxSpeed) 
    then
        self.currentSpeed.y = self.maxSpeed
    elseif (self.currentSpeed.y < -self.maxSpeed)
    then
        self.currentSpeed.y = -self.maxSpeed
    end

    self.transform.position = self.transform.position + ((self.currentSpeed.y * self.transform:forward()) * deltaTime)
end

function script:rotate(deltaTime)
    self.moveDir.x = core.btoi(input.isKeyDown(Keys.A)) - core.btoi(input.isKeyDown(Keys.D))

    if (self.transform.rotation.y > 359.5 and self.transform.rotation.y ~= 0)
    then
        self.transform.rotation.y = 0
    end

    if (self.moveDir.x > 0)
    then
        self.transform.rotation.y = self.transform.rotation.y + self.turnSpeed * deltaTime
    elseif (self.moveDir.x < 0)
    then
        self.transform.rotation.y = self.transform.rotation.y - self.turnSpeed * deltaTime
    end
end

function script:move2(deltaTime)
    local camTransform = scene.getComponent(self.camID, CompType.Transform)

    self.moveDir.y = (core.btoi(input.isKeyDown(Keys.W)) - core.btoi(input.isKeyDown(Keys.S)))
    self.moveDir.x = (core.btoi(input.isKeyDown(Keys.A)) - core.btoi(input.isKeyDown(Keys.D)))

    self.currentSpeed.x = self.moveDir.x * self.maxSpeed
    self.currentSpeed.y = self.moveDir.y * self.maxSpeed

    if (self.moveDir.x and self.moveDir.y)
    then
        self.currentSpeed = vector.normalize(self.currentSpeed) * self.maxSpeed
    end

    local camFwd = camTransform:forward()
    local moveFwd = camFwd
    moveFwd.y = 0

    self.transform.position = self.transform.position + (vector.normalize(moveFwd) * (self.currentSpeed.y * deltaTime)
    + vector.normalize(self.cross(camTransform:up(), camFwd)) * (self.currentSpeed.x * deltaTime))
end

function script.cross(left, right)
	local x = (left.y * right.z) - (left.z * right.y);
	local y = (left.z * right.x) - (left.x * right.z);
	local z = (left.x * right.y) - (left.y * right.x);

	sum = vector(x, y, z);

	return sum;
end

function script:rotate2(deltaTime)
    local camTransform = scene.getComponent(self.camID, CompType.Transform)

    if (self.moveDir.y > 0)
    then
        self.transform.rotation.y = (camTransform.rotation.y + 180) + 45 * self.moveDir.x
    elseif (self.moveDir.y < 0)
    then
        self.transform.rotation.y = (camTransform.rotation.y) - 45 * self.moveDir.x
    elseif (self.moveDir.x > 0) 
    then
        self.transform.rotation.y = camTransform.rotation.y - 90
    elseif (self.moveDir.x < 0)
    then
        self.transform.rotation.y = camTransform.rotation.y + 90
    end
end

return script
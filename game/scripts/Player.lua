local script = {}

-- Active perks and their effect values
script.perkProperties = 
{
    -- Speed
    speedPerkActive = 0,
    speedPerkValue = 2.0
}

function script:init()
	print("init with ID: " .. self.ID)

	self.moveDir = vector()
	self.currentSpeed = vector()
	self.maxSpeed = 50
	self.speedIncrease = 200
	self.turnSpeed = 200
	self.timer = 0
    self.slowDown = 80
    self.transform.position = vector(0, 0, 0)
    self.transform.rotation = vector(0, 0, 0)
    --self.transform.scale = vector(0.05, 0.05, 0.05)

    self.maxHP = 100.0
    self.currentHP = self.maxHP

    self.animTimer = 0
end

function script:update(dt)
    self:move2(dt)
    self:rotate2(dt)

    -- Test for UI
    self.currentHP = self.currentHP - dt * 10.0
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

    -- Move dir
    self.moveDir.y = (core.btoi(input.isKeyDown(Keys.W)) - core.btoi(input.isKeyDown(Keys.S)))
    self.moveDir.x = (core.btoi(input.isKeyDown(Keys.A)) - core.btoi(input.isKeyDown(Keys.D)))

    -- Speed
    self.currentSpeed.x = self.moveDir.x
    self.currentSpeed.y = self.moveDir.y

    -- Normalize speed
    self.currentSpeed = vector.normalize(self.currentSpeed) * 
        self.maxSpeed * 
        math.max(
            1.0, 
            self.perkProperties.speedPerkActive * 
                self.perkProperties.speedPerkValue)

    -- Forward vector
    local camFwd = camTransform:forward()
    local forwardVec = camFwd
    forwardVec.y = 0
    forwardVec = vector.normalize(forwardVec)

    -- Right vector
    local rightVec = camTransform:up()
    rightVec = rightVec:cross(camFwd)
    rightVec = vector.normalize(rightVec)

    -- Apply position
    self.transform.position = self.transform.position + 
        (forwardVec * self.currentSpeed.y +
        rightVec * self.currentSpeed.x) * deltaTime
    
    -- Handle animation speed and timing
    if (self.animTimer > 0)
    then
        self.animTimer = self.animTimer - deltaTime
    end

    if (self.animTimer < 2)
    then
        local meshChange = scene.getComponent(self.ID, CompType.Mesh)
        meshChange = self.playerMesh
        scene.setComponent(self.ID, CompType.Mesh, meshChange)

        local anim = scene.getComponent(self.ID, CompType.Animation)
        local curSpdSqrd = self.currentSpeed * self.currentSpeed
        local curSpdSum = curSpdSqrd.x + curSpdSqrd.y + curSpdSqrd.z
        if curSpdSum > 0
        then
            anim.timeScale = 1.0
        else
            anim.timer = 0.0
            anim.timeScale = 0.0
        end
        scene.setComponent(self.ID, CompType.Animation, anim)
    end
    if (self.animTimer <= 0 and input.isMouseButtonPressed(Mouse.LEFT))
    then
        local meshChange = scene.getComponent(self.ID, CompType.Mesh)
        meshChange = self.playerAttackMesh
        scene.setComponent(self.ID, CompType.Mesh, meshChange)
        print("hej")
        local anim = scene.getComponent(self.ID, CompType.Animation)
        local curSpdSqrd = self.currentSpeed * self.currentSpeed
        local curSpdSum = curSpdSqrd.x + curSpdSqrd.y + curSpdSqrd.z
        anim.timeScale = 20.0

        scene.setComponent(self.ID, CompType.Animation, anim)
        self.animTimer = 2
    end
end

function script:rotate2(deltaTime)
    local camTransform = scene.getComponent(self.camID, CompType.Transform)

    -- Rotate towards movement direction
    if (self.moveDir.y > 0)
    then
        self.transform.rotation.y = (camTransform.rotation.y + 180) + 45 * self.moveDir.x
        
        -- Rotate because of player model
        self.transform.rotation.y = self.transform.rotation.y + 180
    elseif (self.moveDir.y < 0)
    then
        self.transform.rotation.y = (camTransform.rotation.y) - 45 * self.moveDir.x
        
        -- Rotate because of player model
        self.transform.rotation.y = self.transform.rotation.y + 180
    elseif (self.moveDir.x > 0) 
    then
        self.transform.rotation.y = camTransform.rotation.y - 90
        
        -- Rotate because of player model
        self.transform.rotation.y = self.transform.rotation.y + 180
    elseif (self.moveDir.x < 0)
    then
        self.transform.rotation.y = camTransform.rotation.y + 90
        
        -- Rotate because of player model
        self.transform.rotation.y = self.transform.rotation.y + 180
    end
end

return script
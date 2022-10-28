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
    self.transform.position = vector(0, 10, 0)
    self.transform.rotation = vector(0, 0, 0)
    self.tempRotationY = 0

    self.maxHP = 100.0
    self.currentHP = self.maxHP
    self.onGround = false
    self.active = true
end

function script:update(dt)
    -- Set and use active
    if (input.isKeyPressed(Keys.ESCAPE)) then
        self.active = not self.active
    end

    if (not self.active) then
        return
    end

    -- Check if grounded
    local payload = physics.raycast(self.transform.position, vector(0, -1, 0))
    if (payload) then
        self.onGround = (self.transform.position - payload.hitPoint):length() < 5
    end

    -- New movement using rigidbody
    local camTransform = scene.getComponent(self.camID, CompType.Transform)
    local forward = camTransform:forward()
    forward.y = 0
    local right = camTransform:right()
    right.y = 0

    -- Input vector
    self.moveDir = vector(core.btoi(input.isKeyDown(Keys.A)) - core.btoi(input.isKeyDown(Keys.D)), core.btoi(input.isKeyDown(Keys.W)) - core.btoi(input.isKeyDown(Keys.S)), 0)
    -- Local vector with speed applied
    self.currentSpeed = self.moveDir:normalize() * self.maxSpeed * math.max(1.0, self.perkProperties.speedPerkActive * self.perkProperties.speedPerkValue)
    -- Final vector in 3D using cameras directional vectors
    self.currentSpeed = forward:normalize() * self.currentSpeed.y + right:normalize() * self.currentSpeed.x

    -- Apply to rigidbody velocity
    local rb = scene.getComponent(self.ID, CompType.Rigidbody)
    local y = rb.velocity.y
    rb.velocity = self.currentSpeed
    rb.velocity.y = y + core.btoi(input.isKeyPressed(Keys.SPACE)) * core.btoi(self.onGround) * 7.5 * rb.gravityMult
    scene.setComponent(self.ID, CompType.Rigidbody, rb)

    -- Handle animation speed and timing
    local anim = scene.getComponent(self.ID, CompType.Animation)
    local curMoveSqrd = self.moveDir * self.moveDir
    local curMoveSum = curMoveSqrd.x + curMoveSqrd.y + curMoveSqrd.z
    if curMoveSum > 0.1
    then
        anim.timeScale = 1.0
    else
        anim.timer = 0.0
        anim.timeScale = 0.0
    end
    scene.setComponent(self.ID, CompType.Animation, anim)

    self:rotate2(dt)
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

function script:rotate2(deltaTime)
    local camTransform = scene.getComponent(self.camID, CompType.Transform)

    -- Rotate towards movement direction
    if (self.moveDir.y > 0.5)
    then
        self.transform.rotation.y = (camTransform.rotation.y + 180) + 45 * self.moveDir.x
        
        -- Rotate because of player model
        self.tempRotationY = self.transform.rotation.y + 180
    elseif (self.moveDir.y < -0.5)
    then
        self.transform.rotation.y = camTransform.rotation.y - 45 * self.moveDir.x
        
        -- Rotate because of player model
        self.tempRotationY = self.transform.rotation.y + 180
    elseif (self.moveDir.x > 0.5) 
    then
        self.transform.rotation.y = camTransform.rotation.y - 90
        
        -- Rotate because of player model
        self.tempRotationY = self.transform.rotation.y + 180
    elseif (self.moveDir.x < -0.5)
    then
        self.transform.rotation.y = camTransform.rotation.y + 90
        
        -- Rotate because of player model
        self.tempRotationY = self.transform.rotation.y + 180
    end

    self.transform.rotation.y = self.tempRotationY
    self.transform.rotation.x = 0.0
    self.transform.rotation.z = 0.0
    
    --print(self.moveDir.x .. " " .. self.moveDir.y)
end

return script
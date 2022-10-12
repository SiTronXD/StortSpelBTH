local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	self.maxXRot       = math.pi / 2 - ((math.pi / 2) * 0.15)
    self.minXRot       = -(math.pi / 4) + ((math.pi / 4) * 0.1)
    self.camDist       = 23
    self.camHeight     = 14
    self.sens          = 2

    self.shaking       = false
    self.shakeTimer    = 0
    self.shakeDuration = 0.3
    self.shakeScalar   = 0.3
    self.camRot        = vector()

end

function script:update(dt)

    if (input.isKeyDown(Keys.Q))
    then
        shaking = true
    end

    local XInput = core.btoi(input.isKeyDown(Keys.H)) - core.btoi(input.isKeyDown(Keys.K))
    local YInput = core.btoi(input.isKeyDown(Keys.J)) - core.btoi(input.isKeyDown(Keys.U))

    self.camRot.x = self.camRot.x + self.sens * YInput * dt
    self.camRot.y = self.camRot.y + self.sens * XInput * dt

    if (self.camRot.x >= self.maxXRot)
    then
        self.camRot.x = self.maxXRot
    elseif (self.camRot.x <= self.minXRot) 
    then
        self.camRot.x = self.minXRot
    end

    local targetPos = scene.getComponent(playerID, CompType.Transform).position
    targetPos.y = targetPos.y + self.camHeight

    if (self.shaking) 
    then
        self.shakeTimer = self.shakeTimer + dt
        if (self.shakeTimer >= self.shakeDuration)
        then
            self.shakeTimer = 0
            self.shaking = false
        else 
            targetPos = targetPos + self.randomVector(self.shakeScalar)
        end
    end

    self.transform.rotation.y = self.camRot.y * (180 / math.pi)
    self.transform.rotation.x = self.camRot.x * (180 / math.pi)
    
    local scaledFwd = self.transform:forward() * -self.camDist
    
    self.transform.position = targetPos + scaledFwd
end

function script.randomVector(scalar)
    return vector(
        ((math.random() % 201) * 0.01 - 1) * scalar,
        ((math.random() % 201) * 0.01 - 1) * scalar,
        ((math.random() % 201) * 0.01 - 1) * scalar)
end

return script
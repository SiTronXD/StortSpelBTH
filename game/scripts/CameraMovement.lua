local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	self.maxXRot       = math.pi / 2 - ((math.pi / 2) * 0.15)
    self.minXRot       = -(math.pi / 4) + ((math.pi / 4) * 0.1)
    self.camDist       = 30
    self.camHeight     = 6
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
        self.shaking = true
    end

    -- Camera input controls
    local XInput = core.btoi(input.isKeyDown(Keys.H)) - core.btoi(input.isKeyDown(Keys.K))
    local YInput = core.btoi(input.isKeyDown(Keys.J)) - core.btoi(input.isKeyDown(Keys.U))
    --local XInput = input.getMouseDelta().x
    --local YInput = -input.getMouseDelta().y

    -- Rotate camera
    self.camRot.x = self.camRot.x + self.sens * YInput * dt
    self.camRot.y = self.camRot.y + self.sens * XInput * dt

    -- Limit camera angle
    if (self.camRot.x >= self.maxXRot)
    then
        self.camRot.x = self.maxXRot
    elseif (self.camRot.x <= self.minXRot) 
    then
        self.camRot.x = self.minXRot
    end

    -- Create target pos
    local targetPos = scene.getComponent(self.playerID, CompType.Transform).position
    targetPos.y = targetPos.y + self.camHeight

    -- Camera shake
    if (self.shaking) 
    then
        self.shakeTimer = self.shakeTimer + dt

        if (self.shakeTimer >= self.shakeDuration)
        then
            -- Stop shaking
            self.shakeTimer = 0
            self.shaking = false
        else 
            -- Start shaking
            local offsetVector = vector()
            offsetVector:randomVector(self.shakeScalar)
            targetPos = targetPos + offsetVector
        end
    end

    -- Apply rotation
    self.transform.rotation.y = self.camRot.y * (180 / math.pi)
    self.transform.rotation.x = self.camRot.x * (180 / math.pi)
    
    -- Apply position
    local scaledFwd = self.transform:forward() * -self.camDist
    self.camHeight = 6
    self.camDist = 30
    self.transform.position = targetPos + scaledFwd
end

return script
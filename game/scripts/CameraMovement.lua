local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	self.maxXRot       = 75
    self.minXRot       = -40
    self.minZoom       = 10
    self.maxZoom       = 25

    self.camDist       = 30
    self.camHeight     = 6
    self.sens          = 25
    self.distAcc       = 0
    self.distMargin    = 1

    self.shaking       = false
    self.shakeTimer    = 0
    self.shakeDuration = 0.3
    self.shakeScalar   = 0.3
    self.active        = true
    
    input.setHideCursor(self.active)   
end

function script:update(dt)
    -- Set and use active
    if (input.isKeyPressed(Keys.ESCAPE)) then
        self.active = not self.active
        input.setHideCursor(self.active)
    end
    
    if (not self.active) then
        return
    end

    self.shaking = input.isKeyDown(Keys.Q)

    -- Change camera distance
    self.distAcc = (self.distAcc - input.getScrollWheelDelta() * 0.5) * 0.01 ^ dt
    self.camDist = self.camDist + self.distAcc
    
    -- Limit total zoom
    self.camDist = math.min(self.camDist, self.maxZoom)
    self.camDist = math.max(self.camDist, self.minZoom)
    --self.camDist = 400
    --self.camHeight = 150
    -- Camera input controls
    local rotInput = vector()
    rotInput.x = input.getMouseDelta().y
    rotInput.y = -input.getMouseDelta().x

    -- Rotate camera
    local camRot = self.transform.rotation + vector.fill(self.sens) * rotInput * dt

    -- Limit camera angle
    camRot.x = math.min(camRot.x, self.maxXRot)
    camRot.x = math.max(camRot.x, self.minXRot)

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
            targetPos = targetPos + vector.random(self.shakeScalar)
        end
    end

    -- Apply rotation
    self.transform.rotation = camRot
    local forward = self.transform:forward()

    -- Limit distance
    local actualDist = self.camDist + self.distMargin
    local payload = physics.raycast(targetPos, -forward, actualDist)
    if (payload) then
        if(scene.getComponent(payload.entity, CompType.Collider).isTrigger == false) then
            actualDist = vector.length(payload.hitPoint - targetPos)
        end
    end
    -- Apply position
    local scaledFwd = forward * (actualDist - self.distMargin)
    self.transform.position = targetPos - scaledFwd
end

return script
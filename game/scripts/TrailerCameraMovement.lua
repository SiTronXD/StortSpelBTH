local script = {}

function script:init()
    self.movementSpeed = 10;
    self.rotationSpeed = 20;
    self.currentSpeed = vector();
    self.moveDir = vector();
    self.upVector = vector(0, 1, 0);
end

function script:update(dt)
    local camTransform = scene.getComponent(self.camID, CompType.Transform)	

	--Movement forward,back, side to side
	self.moveDir.y = (core.btoi(input.isKeyDown(Keys.I)) - core.btoi(input.isKeyDown(Keys.K)))
    self.moveDir.x = (core.btoi(input.isKeyDown(Keys.J)) - core.btoi(input.isKeyDown(Keys.L)))
	 -- Speed
    self.currentSpeed.x = self.moveDir.x
    self.currentSpeed.y = self.moveDir.y

    -- Normalize speed
    self.currentSpeed = vector.normalize(self.currentSpeed) * self.movementSpeed;

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
        rightVec * self.currentSpeed.x) * dt

	--Movement up and down
    if(input.isKeyDown(Keys.M)) then
        self.transform.position.y = self.transform.position.y + (dt * self.movementSpeed)
    end
    if(input.isKeyDown(Keys.N)) then
        self.transform.position.y = self.transform.position.y - (dt * self.movementSpeed)
    end
    
	--rotation
    if(input.isKeyDown(Keys.U)) then
        self.transform.rotation.y = self.transform.rotation.y + (dt * self.rotationSpeed)
    end
    if(input.isKeyDown(Keys.O)) then
        self.transform.rotation.y = self.transform.rotation.y - (dt * self.rotationSpeed)
    end
    if(input.isKeyDown(Keys.Y)) then
        self.transform.rotation.x = self.transform.rotation.x + (dt * self.rotationSpeed)
    end
    if(input.isKeyDown(Keys.P)) then
        self.transform.rotation.x
        = self.transform.rotation.x - (dt * self.rotationSpeed)
    end
end

return script
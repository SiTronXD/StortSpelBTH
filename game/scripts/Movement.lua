local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	self.moveDir = vector()
	self.currentSpeed = vector()
	self.maxSpeed = 50
	self.speedIncrease = 200
	self.turnSpeed = 200
	self.timer = 0
    self.slowDown = 180

end

function script:update(dt)

    -- Reference currently only used for ImGui
    Movement& movement = scene->getComponent<Movement>(playerID)

    local moveSystem0 = true

    if (ImGui::Begin("Movement"))
    then
        ImGui::PushItemWidth(-100)
        ImGui::Text("Player")

        ImGui::Checkbox("Move system switch", &moveSystem0)
        ImGui::DragFloat("Max speed", &self.maxSpeed, 0.05, 0, 200)
        ImGui::DragFloat("Speed inc", &self.speedIncrease, 0.05)
        ImGui::DragFloat("Slow down", &self.slowDown, 0.05, 0.0001)

        ImGui::Text("Z,X Speed: (%f, %f)", self.currentSpeed.y, self.currentSpeed.x)

        ImGui::Separator()
        ImGui::PopItemWidth()
    end
    ImGui::End()


    Transform& camTra = scene->getComponent<Transform>(scene->getMainCameraID())

    if (moveSystem0) 
    then
        local playerView = {transform, mesh, "Movement.lua"}
        local moveLambda = (transform, movement, script)
            move(deltaTime);
            rotate(deltaTime)
        playerView.each(moveLambda)
    else 
    then
        local playerView = {transform, mesh, "Movement.lua"}
        local moveLambda = (transform, movement, script)
            move2(movement, transform, camTra, deltaTime)
            rotate2(movement, transform, camTra, deltaTime)
        playerView.each(moveLambda)
    end

end

function script:move(float deltaTime)
    local playerTransform = scene.getComponent(playerID, CompType.Transform)
    self.currentSpeed.x = 0.f;
    self.moveDir.y = (input::isKeyDown(Keys.W) - input.isKeyDown(Keys.S));

    if (self.currentSpeed.y > self.maxSpeed) 
    {
        self.currentSpeed.y = self.maxSpeed
    }
    if (!self.moveDir.y && self.currentSpeed.y != 0) 
    then
        if (self.currentSpeed.y < 0.001 && self.currentSpeed.y > -0.001)
        then
            self.currentSpeed.y = 0
        elseif (self.currentSpeed.y > 0) 
        then
            self.currentSpeed.y -= self.slowDown * deltaTime
        elseif (self.currentSpeed.y < 0.f) 
        then
            self.currentSpeed.y += self.slowDown * deltaTime
    else 
    then
        self.currentSpeed.y += self.speedIncrease * self.moveDir.y * deltaTime;
    end

    if (self.currentSpeed.y > self.maxSpeed) 
    then
        self.currentSpeed.y = self.maxSpeed
    elseif (self.currentSpeed.y < -self.maxSpeed)
    then
        self.currentSpeed.y = -self.maxSpeed
    end

    playerTransform.position += (self.currentSpeed.y * playerTransform.up()) * deltaTime;
end

function script:rotate(float deltaTime)
    local playerTransform = scene.getComponent(playerID, CompType.Transform)
    self.moveDir.x = (float)(input.isKeyDown(Keys.A) - input.isKeyDown(Keys.D))

    if (playerTransform.rotation.y > 359.5f && playerTransform.rotation.y != 0.f)
    then
        playerTransform.rotation.y = 0.f
    end
    if (self.moveDir.x > 0)
    then
        playerTransform.rotation.y += self.turnSpeed * deltaTime
    else if (self.moveDir.x < 0)
    then
        playerTransform.rotation.y -= self.turnSpeed * deltaTime
    end
end

return script
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
	self.maxSpeed = 30
    self.sprintSpeed = 60
    self.isSprinting = false
	self.speedIncrease = 200
	self.turnSpeed = 200
	self.timer = 0
    self.slowDown = 80
    self.transform.position = vector(0, 12, 0)
    self.transform.rotation = vector(0, 0, 0)

    self.maxHealth = 100
    self.currentHealth = 100
    self.maxStamina = 100
    self.currentStamina = 100
    self.sprintStamDrain = 10.0
    self.staminaRegen = 30.0
    self.staminaRegenCd = 2.0
    self.staminaTimer = 0.0
    self.useStamina = true

    self.dodgeSpeed = 100
    self.dodgeTimer = 0.0
    self.dodgeTime = 0.6
    self.currentMoveDir = 0
    self.isDodging = false

    self.isPushed = false;
    self.pushTimer = 0.0

    self.animTimer = -1.0
    self.wholeBody = false
    self.isAttacking = false
    self.isMoving = false
    self.canMove = true
    self.isDead = false
    self.onGround = false
    self.jumpTimer = 0

    self.activeAnimation = { idle = 1, run = 2, sprint = 3, 
        dodge = 4, attack = 5, moveAttack = 6, dead = 7 }
    self.currentAnimation = 1
    self.idleAnimTime = 1.0
    self.runAnimTime = 1.8
    self.sprintAnimTime = 2.8
    self.dodgeAnimTime = 3.0
end

function script:update(dt)
    -- Dead as animation but no longer dead
    if (self.currentAnimation == self.activeAnimation.dead and not self.isDead) then
        self.currentAnimation = 1
        scene.blendToAnimation(self.ID, "idle", "", 0.25, 1)
    end
    -- Dead change to death animation
    if self.currentHealth <= 0
    then
        if self.currentAnimation ~= self.activeAnimation.dead
        then
            scene.blendToAnimation(self.ID, "dead", "", 0.3, 1.0)
            self.currentAnimation = self.activeAnimation.dead
        end
    end
    -- Paused game or dead
    if (paused or self.currentAnimation == self.activeAnimation.dead) then
        local rb = scene.getComponent(self.ID, CompType.Rigidbody)
        rb.velocity.x = 0
        rb.velocity.z = 0
        scene.setComponent(self.ID, CompType.Rigidbody, rb)
        return
    end

    -- Check if grounded
    local payload = physics.raycast(self.transform.position + vector(0, 1, 0), vector(0, -1, 0))
    if (payload) then
        if (scene.getComponent(payload.entity, CompType.Collider).isTrigger == false and
            payload.entity ~= self.playerID) then
            self.onGround = (self.transform.position - payload.hitPoint):length() < 2
        end
    end

    -- Movement using rigidbody
    local camTransform = scene.getComponent(self.camID, CompType.Transform)
    local forward = camTransform:forward()
    forward.y = 0
    local right = camTransform:right()
    right.y = 0

    -- Push timer countdown
    if self.pushTimer > 0.0 then
        self.pushTimer = self.pushTimer - dt
        self.isPushed = true
    else
        self.isPushed = false
    end

    -- Input vector
    if not self.isDodging and self.canMove
    then
        self.moveDir = vector(core.btoi(input.isKeyDown(Keys.A)) - core.btoi(input.isKeyDown(Keys.D)), core.btoi(input.isKeyDown(Keys.W)) - core.btoi(input.isKeyDown(Keys.S)), 0)

        if self.moveDir ~= vector(0)
        then
            self.isMoving = true
        else
            self.isMoving = false
        end
    end

    -- Local vector with speed applied with or without sprint (using stamina)
    if (self.staminaTimer >= 0)
    then
        self.staminaTimer = self.staminaTimer - dt
    else
        if (self.currentStamina < self.maxStamina)
        then
            self.currentStamina = self.currentStamina + self.staminaRegen * dt
            if (self.currentStamina < 10)
            then
                self.useStamina = false
            else
                self.useStamina = true
            end
        end
    end

    -- Sprint
    if (input.isKeyDown(Keys.SHIFT) and self.canMove)
    then
        if (self.currentStamina > 0 and self.useStamina == true and self.moveDir ~= vector(0))
        then
            self.isSprinting = true
            self.currentSpeed = self.moveDir:normalize() * self.sprintSpeed
            self.currentStamina = self.currentStamina - (self.sprintStamDrain * dt)
            self.staminaTimer = self.staminaRegenCd
        else
            self.currentSpeed = self.moveDir:normalize() * self.maxSpeed
            self.isSprinting = false
        end
    else
        self.currentSpeed = self.moveDir:normalize() * self.maxSpeed
        self.isSprinting = false
    end

    if (self.dodgeTimer > -0.5)
    then
        self.dodgeTimer = self.dodgeTimer - dt
    end

    -- Dodge
    if (input.isKeyPressed(Keys.CTRL) and self.canMove)
    then
        self.currentMoveDir = self.moveDir:normalize()
        if (self.currentStamina > 20.0 and self.currentMoveDir ~= vector(0))
        then
            if not self.isDodging and not self.isAttacking
            then
                self.isDodging = true
                self.currentStamina = self.currentStamina - 20.0
                self.staminaTimer = self.staminaRegenCd
                self.currentSpeed = self.currentMoveDir * self.dodgeSpeed
                self.dodgeTimer = self.dodgeTime
            end
        end
    elseif (self.dodgeTimer > 0.0)
    then
        self.currentSpeed = self.currentMoveDir * self.dodgeSpeed
    else
        self.isDodging = false
    end

    -- Final vector in 3D using cameras directional vectors
    self.currentSpeed = forward:normalize() * self.currentSpeed.y + right:normalize() * self.currentSpeed.x

    -- Jumping
    if (input.isKeyDown(Keys.SPACE) and self.onGround) then
        self.jumpTimer = 0.25
    elseif (self.jumpTimer > 0) then
        self.jumpTimer = self.jumpTimer - dt
    end

    -- Apply to rigidbody velocity
    local rb = scene.getComponent(self.ID, CompType.Rigidbody)
    local y = rb.velocity.y
    if not self.isPushed and self.canMove then
        rb.velocity = self.currentSpeed
    elseif not self.canMove then
        rb.velocity = vector(0)
    end

    -- Apply jump via acceleration
    local jump = self.jumpTimer > 0 and input.isKeyDown(Keys.SPACE)
    if(jump) then
        rb.acceleration.y = 100
    else
        rb.acceleration.y = -300
    end
    if(input.isKeyPressed(Keys.SPACE) and self.onGround) then
        y = y + 50
    end

    rb.velocity.y = y
    scene.setComponent(self.ID, CompType.Rigidbody, rb)

    -- Handle animation speed and timing
    local curMoveSqrd = self.moveDir * self.moveDir
    local curMoveSum = curMoveSqrd.x + curMoveSqrd.y + curMoveSqrd.z

    if (self.animTimer > 0.0)
    then
        self.animTimer = self.animTimer - dt
    end

    if (input.isKeyPressed(Keys.CTRL) and not self.isDead)
    then
        if (self.animTimer < 0.0)
        then
            if self.currentAnimation ~= self.activeAnimation.dodge and self.currentSpeed ~= vector(0)
            then
                if self.isDodging and not self.isAttacking
                then
                    scene.blendToAnimation(self.ID, "dodge", "", 0.3, self.dodgeAnimTime)
                    self.currentAnimation = self.activeAnimation.dodge
                    self.animTimer = 0.6
                end
            end
        end
    end

    if (input.isKeyDown(Keys.SHIFT) and not self.isDodging and not self.isDead)
    then
        if (self.animTimer < 0.0)
        then
            if (self.isSprinting and self.moveDir ~= vector(0))
            then
                if self.currentAnimation ~= self.activeAnimation.sprint
                then
                    if self.currentAnimation == self.activeAnimation.run
                    then
                        scene.setAnimationTimeScale(self.ID, self.sprintAnimTime, "")
                        self.currentAnimation = self.activeAnimation.sprint
                    else
                        scene.blendToAnimation(self.ID, "run", "", 0.3, self.sprintAnimTime)
                        self.currentAnimation = self.activeAnimation.sprint
                    end
                end
            end
        end
    end

    if not self.isSprinting and not self.isDodging and not self.isDead
    then
        if (self.animTimer < 0.0)
        then
            if self.moveDir ~= vector(0) and self.currentAnimation ~= self.activeAnimation.run
            then
                if self.currentAnimation == self.activeAnimation.sprint
                then
                    scene.setAnimationTimeScale(self.ID, self.runAnimTime, "")
                    self.currentAnimation = self.activeAnimation.run
                else
                    scene.blendToAnimation(self.ID, "run", "", 0.3, self.runAnimTime)
                    self.currentAnimation = self.activeAnimation.run
                end
            elseif curMoveSum < 0.1 and self.currentAnimation ~= self.activeAnimation.idle
            then
                scene.blendToAnimation(self.ID, "idle", "", 0.2, self.idleAnimTime)
                self.currentAnimation = self.activeAnimation.idle
            end
        end
    end

    if self.isAttacking and self.moveDir ~= vector(0) and self.canMove
    then
        if self.currentAnimation ~= self.activeAnimation.run and not self.isSprinting
        then
            scene.blendToAnimation(self.ID, "run", "LowerBody", 0.3, self.runAnimTime)
            self.currentAnimation = self.activeAnimation.run
        elseif self.currentAnimation ~= self.activeAnimation.Sprint and self.isSprinting
        then
            scene.setAnimationTimeScale(self.ID, self.sprintAnimTime, "LowerBody")
            self.currentAnimation = self.activeAnimation.sprint
        end
    elseif self.isAttacking and curMoveSum < 0.1 and self.currentAnimation ~= self.activeAnimation.idle
    then
        scene.blendToAnimation(self.ID, "idle", "LowerBody", 0.2, self.idleAnimTime)
        self.currentAnimation = self.activeAnimation.idle
    end

    if (scene.getComponent(self.ID, CompType.Mesh).meshID == self.playerMesh)
    then
        self:rotate2(dt)
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

end

function script:rotate2(deltaTime)
    local camTransform = scene.getComponent(self.camID, CompType.Transform)

    -- Rotate towards movement direction
    if (self.moveDir.y > 0.5)
    then
        self.transform.rotation.y = (camTransform.rotation.y + 180) + 45 * self.moveDir.x
        
        self.transform.rotation.y = self.transform.rotation.y + 180
    elseif (self.moveDir.y < -0.5)
    then
        self.transform.rotation.y = camTransform.rotation.y - 45 * self.moveDir.x
        
        self.transform.rotation.y = self.transform.rotation.y + 180
    elseif (self.moveDir.x > 0.5) 
    then
        self.transform.rotation.y = camTransform.rotation.y - 90
        
        self.transform.rotation.y = self.transform.rotation.y + 180
    elseif (self.moveDir.x < -0.5)
    then
        self.transform.rotation.y = camTransform.rotation.y + 90
        
        self.transform.rotation.y = self.transform.rotation.y + 180
    end
end

return script
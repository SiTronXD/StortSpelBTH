local script = {}

function script:init()
	print("init with ID: " .. self.ID)

	local maxXRot       = M_PI / 2 - ((M_PI / 2) * 0.15f)
    local minXRot       = -(M_PI / 4) + ((M_PI / 4) * 0.1f)
    local camDist       = 23.f
    local camHeight     = 14.f
    local sens          = 2.f; //= 1.7f

    local shaking       = false
    local shakeTimer    = 0.f
    local shakeDuration = 0.3f
    local shakeScalar   = 0.3f
    local camRot {}

end

function script:update(dt)

    if (Input::isKeyDown(Keys::Q))
    then
        shaking = true
    end

    local XInput = Input::isKeyDown(Keys::LEFT) ? 1.f : Input::isKeyDown(Keys::RIGHT) ? -1.f : 0.f
    local YInput = Input::isKeyDown(Keys::DOWN) ? 1.f : Input::isKeyDown(Keys::UP)  ? -1.f : 0.f

    camRot.x += sens * YInput * deltaTime
    camRot.y += sens * XInput * deltaTime

    if (camRot.x >= maxXRot)
    then
        camRot.x = maxXRot
    else if (camRot.x <= minXRot) 
    then
        camRot.x = minXRot
    end

    local targetPos = scene->getComponent<Transform>(playerID).position
    targetPos.y += camHeight

    if (shaking) 
    then
        self.shakeTimer += deltaTime
        if (shakeTimer >= shakeDuration)
        then
            self.shakeTimer = 0.f
            self.shaking = false
        else 
            targetPos = targetPos + SMath::getRandomVector(shakeScalar)
        end
    }

    local quat = glm::quat(camRot)
    local scaledFwd = glm::normalize(SMath::rotateVectorByQuaternion(quat, glm::vec3(0.f, 0.f, 1.f))) * -camDist

    self.transform.position = targetPos + scaledFwd

    self.transform.rotation.y = self.camRot.y * (180.f / (float)M_PI)
    self.transform.rotation.x = self.camRot.x * (180.f / (float)M_PI)
end

return script
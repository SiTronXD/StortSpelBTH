#pragma once
#define _USE_MATH_DEFINES

#include "glm/glm.hpp"
#include <cmath>

struct CameraMovement {
    float maxXRot       = M_PI / 2 - ((M_PI / 2) * 0.15f);
    float minXRot       = -(M_PI / 4) + ((M_PI / 4) * 0.1f);
    float camDist       = 23.f;
    float camHeight     = 14.f;
    float sens          = 2.f; //= 1.7f;

    bool  shaking       = false;
    float shakeTimer    = 0.f;
    float shakeDuration = 0.3f;
    float shakeScalar   = 0.3f;
    glm::vec3 camRot {};
};
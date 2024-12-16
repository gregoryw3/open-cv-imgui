#pragma once

#include "transform.hpp"
#include <glm/glm.hpp>

class PointLight {
public:
    PointLight(float intensity, const glm::vec3& color);

    Transform transform;
    float intensity;
    glm::vec3 color;
};
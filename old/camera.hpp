#pragma once

#include "transform.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(float left, float right, float bottom, float top, float near, float far);

    float ratio() const;
    virtual glm::vec3 projectPoint(const glm::vec3& p) const = 0;
    virtual glm::vec3 inverseProjectPoint(const glm::vec3& p) const = 0;
    Transform transform;
    float left, right, bottom, top, near, far;
    float width, height;

protected:
    
};

class OrthoCamera : public Camera {
public:
    OrthoCamera(float left, float right, float bottom, float top, float near, float far);

    glm::vec3 projectPoint(const glm::vec3& p) const override;
    glm::vec3 inverseProjectPoint(const glm::vec3& p) const override;

private:
    glm::mat4 orthoTransform;
    glm::mat4 inverseOrthoTransform;
};

class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(float left, float right, float bottom, float top, float near, float far);

    glm::vec3 projectPoint(const glm::vec3& p) const override;
    glm::vec3 inverseProjectPoint(const glm::vec3& p) const override;

    static PerspectiveCamera fromFOV(float fov, float near, float far, float ratio);
    glm::mat4 perspectiveTransform;
    glm::mat4 inversePerspectiveTransform;

private:
    
};
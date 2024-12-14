#include "camera.hpp"

Camera::Camera(float left, float right, float bottom, float top, float near, float far)
    : left(left), right(right), bottom(bottom), top(top), near(near), far(far), transform() {
    width = right - left;
    height = top - bottom;
}

float Camera::ratio() const {
    return width / height;
}

OrthoCamera::OrthoCamera(float left, float right, float bottom, float top, float near, float far)
    : Camera(left, right, bottom, top, near, far) {
    orthoTransform = glm::ortho(left, right, bottom, top, near, far);
    inverseOrthoTransform = glm::inverse(orthoTransform);
}

glm::vec3 OrthoCamera::projectPoint(const glm::vec3& p) const {
    glm::vec3 cameraSpacePoint = transform.applyInverseToPoint(p);
    glm::vec4 homogeneousPoint = glm::vec4(cameraSpacePoint, 1.0);
    glm::vec4 orthoCamera = orthoTransform * homogeneousPoint;
    return glm::vec3(orthoCamera);
}

glm::vec3 OrthoCamera::inverseProjectPoint(const glm::vec3& p) const {
    glm::vec4 homogeneousPoint = glm::vec4(p, 1.0f);
    glm::vec4 cameraSpacePoint = inverseOrthoTransform * homogeneousPoint;
    return transform.applyToPoint(glm::vec3(cameraSpacePoint));
}

PerspectiveCamera::PerspectiveCamera(float left, float right, float bottom, float top, float near, float far)
    : Camera(left, right, bottom, top, near, far) {
    perspectiveTransform = glm::frustum(left, right, bottom, top, near, far);
    inversePerspectiveTransform = glm::inverse(perspectiveTransform);
}

glm::vec3 PerspectiveCamera::projectPoint(const glm::vec3& p) const {
    glm::vec3 cameraSpacePoint = transform.applyInverseToPoint(p);
    glm::vec4 homogeneousPoint = glm::vec4(cameraSpacePoint, 1.0f);
    glm::vec4 perspectiveCamera = perspectiveTransform * homogeneousPoint;
    perspectiveCamera /= perspectiveCamera.w;
    return glm::vec3(perspectiveCamera);
}

glm::vec3 PerspectiveCamera::inverseProjectPoint(const glm::vec3& p) const {
    glm::vec4 homogeneousPoint = glm::vec4(p, 1.0f);
    glm::vec4 cameraSpacePoint = inversePerspectiveTransform * homogeneousPoint;
    return transform.applyToPoint(glm::vec3(cameraSpacePoint));
}

PerspectiveCamera PerspectiveCamera::fromFOV(float fov, float near, float far, float ratio) {
    float fovRad = glm::radians(fov);
    float height = near * tan(fovRad / 2.0f);
    float width = height * ratio;
    float left = -width;
    float right = width;
    float top = height;
    float bottom = -top;
    return PerspectiveCamera(left, right, bottom, top, near, far);
}
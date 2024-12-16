#include "transform.hpp"

Transform::Transform() : position(0.0f), rotation(0.0f) {}

glm::mat4 Transform::rotationMatrix() const {
    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    return rotZ * rotY * rotX;
}

glm::mat4 Transform::transformationMatrix() const {
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
    return trans * rotationMatrix();
}

void Transform::setPosition(float x, float y, float z) {
    position = glm::vec3(x, y, z);
}

void Transform::setRotation(float x, float y, float z) {
    rotation = glm::vec3(x, y, z);
}

glm::mat4 Transform::inverseMatrix() const {
    return glm::inverse(transformationMatrix());
}

glm::vec3 Transform::applyToPoint(const glm::vec3& p) const {
    glm::vec4 result = transformationMatrix() * glm::vec4(p, 1.0f);
    return glm::vec3(result);
}

glm::vec3 Transform::applyInverseToPoint(const glm::vec3& p) const {
    glm::vec4 result = inverseMatrix() * glm::vec4(p, 1.0f);
    return glm::vec3(result);
}

glm::vec3 Transform::applyToNormal(const glm::vec3& n) const {
    glm::vec3 result = glm::mat3(rotationMatrix()) * n;
    return glm::normalize(result);
}

void Transform::setAxisRotation(const glm::vec3& axis, float rotation) {
    glm::quat q = glm::angleAxis(glm::radians(rotation), glm::normalize(axis));
    glm::mat4 rotMatrix = glm::mat4_cast(q) * rotationMatrix();
    glm::vec3 euler = glm::eulerAngles(glm::quat_cast(rotMatrix));
    this->rotation = glm::degrees(euler);
}
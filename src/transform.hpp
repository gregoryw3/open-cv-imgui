#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

class Transform {
public:
    Transform();

    glm::mat4 rotationMatrix() const;
    glm::mat4 transformationMatrix() const;
    void setPosition(float x, float y, float z);
    void setRotation(float x, float y, float z);
    glm::mat4 inverseMatrix() const;
    glm::vec3 applyToPoint(const glm::vec3& p) const;
    glm::vec3 applyInverseToPoint(const glm::vec3& p) const;
    glm::vec3 applyToNormal(const glm::vec3& n) const;
    void setAxisRotation(const glm::vec3& axis, float rotation);
    glm::vec3 position;
    glm::vec3 rotation;

private:
    
};
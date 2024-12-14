#include "quaternion.hpp"

// Function to rotate a vector by a quaternion
glm::vec3 rotateVectorByQuaternion(const glm::vec3& v, const glm::quat& q) {
    return q * v;
}

// Function to convert Euler angles to a quaternion
glm::quat eulerToQuaternion(const glm::vec3& eulerAngles) {
    glm::quat q = glm::quat(glm::radians(eulerAngles));
    return q;
}

// Function to convert a quaternion to Euler angles
glm::vec3 quaternionToEuler(const glm::quat& q) {
    glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(q));
    return eulerAngles;
}
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Function to rotate a vector by a quaternion
glm::vec3 rotateVectorByQuaternion(const glm::vec3& v, const glm::quat& q);

// Function to convert Euler angles to a quaternion
glm::quat eulerToQuaternion(const glm::vec3& eulerAngles);

// Function to convert a quaternion to Euler angles
glm::vec3 quaternionToEuler(const glm::quat& q);
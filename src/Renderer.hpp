#pragma once

#include "camera.hpp"
#include "mesh.hpp"
#include "PointLight.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glfw/glfw3.h>

class Renderer {
public:
    Renderer(int screenWidth, int screenHeight, const PerspectiveCamera& camera, const std::vector<Mesh>& meshes, const PointLight& light);

    void render(GLFWwindow* window, const std::string& shading, const glm::vec3& bgColor, const glm::vec3& ambientLight);

private:
    int screenWidth, screenHeight;
    PerspectiveCamera camera;
    std::vector<Mesh> meshes;
    PointLight light;
    std::vector<float> zBuffer;

    glm::vec3 computeAmbient(const glm::vec3& ambientLight, float ka, const glm::vec3& objectColor);
    glm::vec3 computeDiffuse(const glm::vec3& point, const glm::vec3& normal, const glm::vec3& diffuseColor, float kd);
    glm::vec3 computePhong(const glm::vec3& point, const glm::vec3& normal, const glm::vec3& viewDir, const Mesh& mesh, const glm::vec3& ambientLight);
    glm::vec3 computeSpecular(const glm::vec3& normal, const glm::vec3& viewDir, const glm::vec3& reflectDir, float distance, const Mesh& mesh);
    glm::vec3 reflect(const glm::vec3& L, const glm::vec3& N);
};
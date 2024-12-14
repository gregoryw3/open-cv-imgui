#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "transform.hpp"

class Mesh {
public:
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> vertex_normals;
    std::vector<GLuint> faces;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
    float ka, kd, ks, ke;
    Transform transform;

    Mesh(const glm::vec3& diffuse_color, const glm::vec3& specular_color, float ka, float kd, float ks, float ke);
    static Mesh from_stl(const std::string& stl_path, const glm::vec3& diffuse_color, const glm::vec3& specular_color, float ka, float kd, float ks, float ke);
    void setupMesh();
    void draw(GLuint shaderProgram);
    GLuint VAO, VBO, EBO;

private:
    
};
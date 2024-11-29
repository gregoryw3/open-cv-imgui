#include "Mesh.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stl.h>

using namespace openstl;

namespace std {
    template<>
    struct hash<glm::vec3> {
        std::size_t operator()(const glm::vec3& v) const noexcept {
            std::size_t h1 = std::hash<float>{}(v.x);
            std::size_t h2 = std::hash<float>{}(v.y);
            std::size_t h3 = std::hash<float>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
}

Mesh::Mesh(const glm::vec3& diffuse_color, const glm::vec3& specular_color, float ka, float kd, float ks, float ke)
    : diffuse_color(diffuse_color), specular_color(specular_color), ka(ka), kd(kd), ks(ks), ke(ke) {}

Mesh Mesh::from_stl(const std::string& stl_path, const glm::vec3& diffuse_color, const glm::vec3& specular_color, float ka, float kd, float ks, float ke) {
    Mesh new_mesh(diffuse_color, specular_color, ka, kd, ks, ke);

    // Load STL file
    std::ifstream file(stl_path, std::ios::binary);

    // Check if file is open
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file '" << stl_path << "'" << std::endl;
    }

    // Deserialize STL file
    std::vector<openstl::Triangle> triangles = openstl::deserializeStl(file);
    // Close file
    file.close();

    // Convert to vertices and faces
    const auto& [vertices, faces] = convertToVerticesAndFaces(triangles);

    // Extract unique vertices
    std::vector<glm::vec3> unique_verts;
    std::unordered_map<glm::vec3, GLuint, std::hash<glm::vec3>> vert_indices;
    for (const auto& triangle : triangles) {
        glm::vec3 vertices[3] = {
            {triangle.v0.x, triangle.v0.y, triangle.v0.z},
            {triangle.v1.x, triangle.v1.y, triangle.v1.z},
            {triangle.v2.x, triangle.v2.y, triangle.v2.z}
        };
        for (const auto& vertex : vertices) {
            if (vert_indices.find(vertex) == vert_indices.end()) {
                vert_indices[vertex] = unique_verts.size();
                unique_verts.push_back(vertex);
            }
        }
    }
    new_mesh.verts = unique_verts;


    // Extract faces
    for (const auto& triangle : triangles) {
        glm::vec3 vertices[3] = {
            {triangle.v0.x, triangle.v0.y, triangle.v0.z},
            {triangle.v1.x, triangle.v1.y, triangle.v1.z},
            {triangle.v2.x, triangle.v2.y, triangle.v2.z}
        };
        std::array<GLuint, 3> face;
        for (int i = 0; i < 3; ++i) {
            face[i] = vert_indices[vertices[i]];
        }
        new_mesh.faces.push_back(face[0]);
        new_mesh.faces.push_back(face[1]);
        new_mesh.faces.push_back(face[2]);
    }

    // Compute vertex normals with weighted average
    new_mesh.vertex_normals.resize(new_mesh.verts.size(), glm::vec3(0.0f));
    for (size_t i = 0; i < new_mesh.faces.size(); i += 3) {
        GLuint idx0 = new_mesh.faces[i];
        GLuint idx1 = new_mesh.faces[i + 1];
        GLuint idx2 = new_mesh.faces[i + 2];

        glm::vec3 v0 = new_mesh.verts[idx0];
        glm::vec3 v1 = new_mesh.verts[idx1];
        glm::vec3 v2 = new_mesh.verts[idx2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 face_normal = glm::cross(edge1, edge2);

        // Compute face area as the weight
        float face_area = 0.5f * glm::length(face_normal);

        // Normalize face normal
        face_normal = glm::normalize(face_normal);
        new_mesh.normals.push_back(face_normal);

        // Weighted normals for each vertex
        new_mesh.vertex_normals[idx0] += face_normal * face_area;
        new_mesh.vertex_normals[idx1] += face_normal * face_area;
        new_mesh.vertex_normals[idx2] += face_normal * face_area;
    }

    // Normalize vertex normals
    for (auto& normal : new_mesh.normals) {
        normal = glm::normalize(normal);
    }

    new_mesh.setupMesh();
    return new_mesh;
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), &verts[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(GLuint), &faces[0], GL_STATIC_DRAW);

    // Vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Vertex normals
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Mesh::draw(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}